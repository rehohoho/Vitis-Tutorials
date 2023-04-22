import numpy as np
np.random.seed(0)
data = np.random.randint(-32768, 32768, size=(6400, 4))
np.savetxt("fir_100samples_rand.txt", data, fmt="%d")