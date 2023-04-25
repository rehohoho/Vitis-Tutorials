import numpy as np
np.random.seed(0)


def np_mmul(matA: np.ndarray,
            matB: np.ndarray,
            dtype: type):
  res = []
  for a, b in zip(matA, matB):
    res.append(np.matmul(a,b))
  return np.array(res, dtype=dtype).transpose(0, 2, 1)


def output_txt(mat: np.ndarray,
               filename: str,
               n_per_row: int):
  np.savetxt(filename, mat.reshape(-1, n_per_row), fmt="%d")


# MmulAieapi: 64x64, random data
M = 64
N = 64
samples = 100
n_per_row = 64 // 8
mat_rand = np.random.randint(-128, 128, size=(samples*M*N)).reshape(samples, M, N)
output_txt(mat_rand, "mmul_100samples_rand.txt", n_per_row)

res = np_mmul(mat_rand, mat_rand, np.int8).transpose(0, 2, 1)
output_txt(res, "mmul_100samples_rand_res.txt", n_per_row)

# MmulIntrinsic: matA and matB column-wise
M = 64
K = 8
N = 2
samples = 100
n_per_row = 64 // 16

matA = np.tile(np.arange(K), samples * M).reshape(samples, M, K)
matA_out = matA.transpose(0, 2, 1)
output_txt(matA_out, "mmula_100samples.txt", n_per_row)

matB_out = np.tile(np.arange(K * N), samples).reshape(samples, N, K)
matB = matB_out.transpose(0, 2, 1)
output_txt(matB_out, "mmulb_100samples.txt", n_per_row)

res = np_mmul(matA, matB, np.int16)
output_txt(res, "mmulc_100samples.txt", n_per_row)

# MmulIntrinsic: matA and matB column-wise, random data
matA_rand = np.random.randint(-32768, 32768, size=(samples, M, K))
matB_rand = np.random.randint(-32768, 32768, size=(samples, K, N))
matA_rand_out = matA_rand.transpose(0, 2, 1)
matB_rand_out = matB_rand.transpose(0, 2, 1)
output_txt(matA_rand_out, "mmula_100samples_rand.txt", n_per_row)
output_txt(matB_rand_out, "mmulb_100samples_rand.txt", n_per_row)

res = np_mmul(matA_rand, matB_rand, np.int16)
output_txt(res, "mmulc_100samples_rand.txt", n_per_row)
