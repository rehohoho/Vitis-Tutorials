import numpy as np

file_name = "mmul_100samples_rand.txt"
with open(file_name) as f:
  raw = f.read()

raw = raw.replace("\n", " ").split()
data = np.array(raw, dtype=np.int8)[:4096].reshape((64, 64))
res = np.matmul(data, data).reshape(-1, 8)
np.savetxt(file_name.replace(".txt", "_res.txt"), res, fmt="%d")