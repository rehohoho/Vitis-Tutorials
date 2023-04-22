import numpy as np
np.random.seed(0)
data = np.random.randint(-128, 128, size=(100*64*64//8, 8))
np.savetxt("mmul_100samples_rand.txt", data, fmt="%d")