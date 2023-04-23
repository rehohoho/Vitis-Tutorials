import numpy as np

M = 64
N = 16
samples = 100
bitwidth = 64
datasize = 16

# column based
col = np.tile(np.arange(N), samples)
data = np.repeat(col[:, None], M, axis=1)
data = data.reshape(-1, bitwidth//datasize)
np.savetxt("vmula_100samples.txt", data, fmt="%d")

# row based
data = np.arange(M)
data = np.repeat(data[None, :], 100, axis=0)
data = data.reshape(-1, bitwidth//datasize)
np.savetxt("vmulb_100samples.txt", data, fmt="%d")
