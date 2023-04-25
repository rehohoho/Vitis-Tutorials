import numpy as np

M = 64
N = 16
samples = 100
bitwidth = 64
datasize = 16
n_per_row = bitwidth // datasize

# column major
col = np.tile(np.arange(N), samples)
matA_out = np.repeat(col[:, None], M, axis=1)
matA = matA_out.reshape(samples, N, M).transpose(0, 2, 1)
np.savetxt("vmula_100samples.txt", 
           matA_out.reshape(-1, n_per_row), fmt="%d")
np.savetxt("vmula_100samples_phase1.txt", 
           matA_out[::2,:].reshape(-1, n_per_row), fmt="%d")
np.savetxt("vmula_100samples_phase2.txt", 
           matA_out[1::2,:].reshape(-1, n_per_row), fmt="%d")

# column major
row = np.arange(N)
matB = np.repeat(row[None, :], samples, axis=0)
np.savetxt("vmulb_100samples.txt", matB.reshape(-1, n_per_row), fmt="%d")

# golden
res = []
for a, b in zip(matA, matB):
  res.append(a.dot(b))
res = np.array(res, dtype=np.int16)
np.savetxt("vmulc_100samples.txt", res.reshape(-1, n_per_row), fmt="%d")
