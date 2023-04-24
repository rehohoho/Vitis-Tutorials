import numpy as np
np.random.seed(0)
data = np.random.randint(-128, 128, size=(100*64*64//8, 8))
np.savetxt("mmul_100samples_rand.txt", data, fmt="%d")

M = 64
K = 8
N = 2
samples = 100
bitwidth = 64
datasize = 16

col = np.tile(np.arange(K), samples)
data = np.repeat(col[:, None], M, axis=1)
np.savetxt("mmula_100samples.txt", 
           data.reshape(-1, bitwidth//datasize), fmt="%d")

instance = np.arange(K*N).reshape(2, -1)
data = np.repeat(instance[None, :], samples, axis=0).reshape(-1, K)
np.savetxt("mmulb_100samples.txt",
           data.reshape(-1, bitwidth//datasize), fmt="%d")