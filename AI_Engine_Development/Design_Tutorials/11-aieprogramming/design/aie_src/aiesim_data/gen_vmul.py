import numpy as np

M = 64
N = 16
samples = 100
bitwidth = 64
datasize = 16

# column based
col = np.tile(np.arange(N), samples)
data = np.repeat(col[:, None], M, axis=1)
np.savetxt("vmula_100samples.txt", 
           data.reshape(-1, bitwidth//datasize), fmt="%d")
np.savetxt("vmula_100samples_phase1.txt", 
           data[::2,:].reshape(-1, bitwidth//datasize), fmt="%d")
np.savetxt("vmula_100samples_phase2.txt", 
           data[1::2,:].reshape(-1, bitwidth//datasize), fmt="%d")

# row based
data = np.arange(M)
data = np.repeat(data[None, :], 100, axis=0)
data = data.reshape(-1, bitwidth//datasize)
np.savetxt("vmulb_100samples.txt", data, fmt="%d")
