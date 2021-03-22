import heterocl as hcl
import numpy as np
# from hash_family import all

N_HASH_FUNC = 5
HASH_RANGE = 100
primeNumber = 39916801
RAND_SEED = 10000000
PACKET_SIZE = 100

UPDATE = 1
QUERY = 2

def func_wrapper(dtype):
    hcl.init(dtype)
    
    a=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    b=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    sketch = hcl.placeholder((N_HASH_FUNC * HASH_RANGE,), dtype=dtype)
    key=hcl.placeholder((PACKET_SIZE,),dtype=dtype)
    def hash_index(sketch, a, b, key):
        index = hcl.compute(a.shape, lambda i: a[i]*key[0] + b[i])
        out = hcl.compute(arr.shape, lambda i: sketch[i*HASH_RANGE+index[i]] + 1)
        return out
    
    hcl_sketch = hcl.asarray(np.zeros(N_HASH_FUNC*HASH_RANGE),dtype=dtype)
    hcl_a = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_b = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_key = hcl.asarray(np.random.randint(10, size=(PACKET_SIZE,)), dtype=dtype)
    s = hcl.create_schedule([hcl_sketch, hcl_a, hcl_b, hcl_key])
    target = hcl.platform.aws_f1
    target.config(compile = 'vitis', mode = 'debug')
    code = hcl.build(s, target, name='countmin')
    with open("count_min.cpp", "w") as fp:
        fp.write(code)

if __name__ == "__main__":
    func_wrapper(hcl.Int(64))
