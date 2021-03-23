import heterocl as hcl
import numpy as np

N_HASH_FUNC = 5
HASH_RANGE = 10

def func_wrapper(dtype):
    hcl.init(dtype)
    a=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    b=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    # sketch = hcl.placeholder((HASH_RANGE,), dtype=dtype)
    key=hcl.placeholder((1,),dtype=dtype)
    def hash_index(a, b, key):
        index = hcl.compute(a.shape, lambda i: (a[i]*key[0] + b[i])%HASH_RANGE)
        # with hcl.for_(0, N_HASH_FUNC) as i:
        #     sketch[index[i]] += 1
    
    s = hcl.create_schedule([a, b, key], hash_index)
    f = hcl.build(s, target="llvm")

    # hcl_sketch = hcl.asarray(np.zeros(N_HASH_FUNC*HASH_RANGE),dtype=dtype)
    hcl_a = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_b = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_key = hcl.asarray(np.random.randint(10, size=(1,)), dtype=dtype)

    f(hcl_a, hcl_b, hcl_key)

if __name__ == "__main__":
    func_wrapper(hcl.Int(64))
