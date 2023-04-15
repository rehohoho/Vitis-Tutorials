from typing import List, Callable, Any, Tuple
import os, sys
import re
import argparse

import numpy as np
import multiprocessing as mp
import struct
import logging
import matplotlib.pyplot as plt

from xilinx_xtlm import ipc_axis_master_util, ipc_axis_slave_util, xtlm_ipc


# See https://docs.python.org/3/library/struct.html#format-characters
def cint16_tobytes(real: List[int], im: List[int]):
  rVec = np.real(real).astype(np.int16)
  iVec = np.imag(im).astype(np.int16)
  return rVec.tolist(), iVec.tolist()

def cint16_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//2)+"h" 

def int8_tobytes(data: List[int]):
  return np.real(data).astype(np.int8)

def int8_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//1)+"b" 

def int32_tobytes(data: List[int]):
  return np.real(data).astype(np.int32).tolist()

def int32_fstr(payload_len_in_bytes: int):
  return "<"+str(payload_len_in_bytes//4)+"i"

def get_format_to_bytes_callable(dtype: str):
  if dtype == "int32":
    return int32_tobytes
  elif dtype == "int8":
    return int8_tobytes
  elif dtype == "cint16":
    return cint16_tobytes
  else:
    raise NotImplementedError(f"{dtype} formatting not supported.")

def get_format_string_callable(dtype: str):
  if dtype == "int32":
    return int32_fstr
  elif dtype == "int8":
    return int8_fstr
  elif dtype == "cint16":
    return cint16_fstr
  else:
    raise NotImplementedError(f"{dtype} format string not supported.")


class ExternalTraffic:
  
  def __init__(self,
               master_list: List[Tuple[str, str, int, str]],
               slave_list: List[Tuple[str, str, str]]):
    
    self.ipc_masters = []
    for ipc_name, file_path, bitwidth, dtype in master_list:
      self.ipc_masters.append(
        (ipc_axis_master_util(ipc_name), ipc_name, file_path, bitwidth, dtype))
      logging.info(f"Creating ipc_axis_master_util for {ipc_name}...")

    self.ipc_slaves = []
    for ipc_name, file_path, bitwidth, dtype in slave_list:
      parent, child = mp.Pipe()
      self.ipc_slaves.append(
        (ipc_axis_slave_util(ipc_name), parent, child, ipc_name, file_path, bitwidth, dtype))
      logging.info(f"Creating ipc_axis_slave_util for {ipc_name}...")

  def send_to_aie(self,
                  ipc_name: str,
                  file_path: str,
                  bitwidth: int, 
                  dtype: str,
                  transport: Callable[[Any], None] #xtlm_ipc_axis_pb2.axi_stream_packet
                  ):
    """Sending data to AIE from memory"""
    format_to_bytes = get_format_to_bytes_callable(dtype)
    get_format_string = get_format_string_callable(dtype)

    with open(file_path) as f:
      L = f.readlines()
      logging.info(f"[{ipc_name}]: Sending {len(L)} int32 data...")
      
      for i, line in enumerate(L):
        values = re.findall(r'-?\d+', line)
        values = [int(v) for v in values]
        packet = format_to_bytes(values)

        payload = xtlm_ipc.axi_stream_packet()
        payload.data_length = bitwidth // 8 # in bytes
        payload.tlast = True

        format_string = get_format_string(payload.data_length)
        payload.data = bytes(bytearray(struct.pack(format_string, *tuple(packet))))
        transport(payload)

  def recv_fr_aie(self,
                  ipc_name: str,
                  dtype: str,
                  ipc_slave: ipc_axis_slave_util,
                  child: Any): # mp.connection.Connection
    """Receiving data from AIE to memory"""
    get_format_string = get_format_string_callable(dtype)

    payload = ipc_slave.sample_transaction()
    
    # See https://docs.python.org/3/library/struct.html#format-characters
    formatString = get_format_string(len(payload.data))
    rxData = struct.unpack(formatString, payload.data)
    child.send(rxData)

  def run(self):
    logging.info("Begin run...")
    master_tasks = []
    
    for ipc_master, ipc_name, file_path, bitwidth, dtype in self.ipc_masters:
      t = mp.Process(target=self.send_to_aie, 
                     args=(ipc_name, file_path, bitwidth, dtype, ipc_master.b_transport))
      t.start()
      master_tasks.append((t, ipc_name))
      logging.info(f"Running master {ipc_name}")

    slave_tasks = []
    for ipc_slave, parent, child, ipc_name, file_path, bitwidth, dtype in self.ipc_slaves:
      t = mp.Process(target=self.recv_fr_aie, 
                     args=(ipc_name, dtype, ipc_slave, child))
      t.start()
      slave_tasks.append((t, parent, ipc_name, file_path, bitwidth, dtype))
      logging.info(f"Running slave {ipc_name}")

    for slave_task, parent, ipc_name, file_path, bitwidth, dtype in slave_tasks:
      data = parent.recv()
      slave_task.join()

      repeat_count = bitwidth // int(re.findall(r'\d+', dtype)[0])
      
      tmp = ""
      with open(file_path, 'w') as f:
        for i, d in enumerate(data):
          tmp += f"{d} "
          if i % repeat_count == repeat_count - 1:
            f.write(f"{tmp}\n")
            tmp = ""

      logging.info(f"Slave {ipc_name} finished. Written to {file_path}")
    
    for master_task, ipc_name in master_tasks:
      master_task.join()
      logging.info(f"Master {ipc_name} finished.")


if __name__ == "__main__":
  
  parser = argparse.ArgumentParser(description='xtg_aie.py')
  parser.add_argument('--input_dir', required=True)
  parser.add_argument('--output_dir', required=True)
  args = parser.parse_args()
  
  logging.basicConfig(format="%(asctime)s: %(message)s", level=logging.INFO, datefmt="%H:%M:%S")

  master_list = [
    ("plin1", f"{args.input_dir}/data/0_1/matA_in_64plio.txt", 64, "int32"),
    ("plin2", f"{args.input_dir}/data/0_2/matA_in_64plio.txt", 64, "int32"),
    ("plin3", f"{args.input_dir}/data/0_3/matA_in_64plio.txt", 64, "int32"),
    ("plin5", f"{args.input_dir}/data/0_5/matA_in_64plio.txt", 64, "int32"),
  ]

  slave_list = [
    ("plout1", f"{args.output_dir}/output01.txt", 64, "int32"), 
    ("plout2", f"{args.output_dir}/output02.txt", 64, "int32"), 
    ("plout4", f"{args.output_dir}/output04.txt", 64, "int32"),
  ]
  
  design = ExternalTraffic(master_list, slave_list)
  design.run()
