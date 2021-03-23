import heterocl as hcl
import numpy as np
# from hash_family import all

N_HASH_FUNC = 5
HASH_RANGE = 100
PRIME_NUM = 39916801
RAND_SEED = 1000
PACKET_SIZE = 10

UPDATE = 1
QUERY = 2

def func_wrapper(dtype):
    hcl.init(dtype)
    
    a=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    b=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    sketch = hcl.placeholder((N_HASH_FUNC * HASH_RANGE,), dtype=dtype)
    key=hcl.placeholder((PACKET_SIZE,),dtype=dtype)
    def hash_index(sketch, a, b, key):
        index = hcl.compute(a.shape, lambda i: (a[i]*key[0] + b[i])%HASH_RANGE)
        with hcl.for_(0, N_HASH_FUNC) as i:
            sketch[i*HASH_RANGE+index[i]] += 1
        # hcl.update(sketch, lambda i: sketch[i*HASH_RANGE+index[i]] + 1, name="update")
        # return out
    
    s = hcl.create_schedule([sketch, a, b, key], hash_index)
    # target = hcl.platform.aws_f1
    # target.config(compile = 'vitis', mode = 'debug')
    target = "llvm"
    f = hcl.build(s, target, name='countmin')
    print(f)

    hcl_sketch = hcl.asarray(np.zeros(N_HASH_FUNC*HASH_RANGE),dtype=dtype)
    hcl_a = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_b = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_key = hcl.asarray(np.random.randint(10, size=(PACKET_SIZE,)), dtype=dtype)
    np_sketch = hcl_sketch.asnumpy()
    np_a = hcl_a.asnumpy()
    np_b = hcl_b.asnumpy()
    np_key = hcl_key.asnumpy()
    print(np_sketch)
    print(np_a)
    print(np_b)
    print(np_key)

    f(hcl_sketch, hcl_a, hcl_b, hcl_key)
    np_sketch = hcl_sketch.asnumpy()
    np_a = hcl_a.asnumpy()
    np_b = hcl_b.asnumpy()
    np_key = hcl_key.asnumpy()
    print(np_sketch)
    # f = hcl.build(s, target, name='countmin')

    # with open("count_min.cpp", "w") as fp:
    #     fp.write(code)

if __name__ == "__main__":
    func_wrapper(hcl.Int(64))
