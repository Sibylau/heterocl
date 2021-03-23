import heterocl as hcl
import numpy as np
# from hash_family import all

N_HASH_FUNC = 5
HASH_RANGE = 100
PRIME_NUM = 39916801
RAND_SEED = 1000
PACKET_SIZE = 20
QUERY_NUM = 5

UPDATE = 1
QUERY = 2

def func_wrapper(dtype):
    hcl.init(dtype)
    
    a=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    b=hcl.placeholder((N_HASH_FUNC,), dtype=dtype)
    sketch = hcl.placeholder((N_HASH_FUNC * HASH_RANGE,), dtype=dtype)
    data_key=hcl.placeholder((PACKET_SIZE,),dtype=dtype)
    query_key=hcl.placeholder((QUERY_NUM,),dtype=dtype)
    result=hcl.placeholder((QUERY_NUM,),dtype=dtype)
    mode = hcl.placeholder((1, ))

    def hash_index(sketch, a, b, data_key, query_key, result, mode):
        with hcl.if_(mode == UPDATE):
            with hcl.for_(0, PACKET_SIZE) as p:
                index = hcl.compute(a.shape, lambda i: (a[i]*key[p] + b[i]))
                with hcl.for_(0, N_HASH_FUNC) as i:
                    sketch[i*HASH_RANGE+index[i]] += 1
                # hcl.update(sketch, lambda i: sketch[i*HASH_RANGE+index[i]] + 1, name="update")
        with hcl.if_(mode == QUERY):
            result = hcl.compute(key.shape, lambda i: 0)
            with hcl.for_(0, PACKET_SIZE) as p:
                index = hcl.compute(a.shape, lambda i: (a[i]*key[p] + b[i]))
                result[p] = sketch[index[0]]
                with hcl.for_(1, N_HASH_FUNC) as n:
                    with hcl.if_(result[p] > sketch[n*HASH_RANGE+index[n]]):
                        result[p] = sketch[n*HASH_RANGE+index[n]]
    
    s = hcl.create_schedule([sketch, a, b, key, result, mode], hash_index)
    # target = hcl.platform.aws_f1
    # target.config(compile = 'vitis', mode = 'debug')
    target = "llvm"
    f = hcl.build(s, target, name='countmin')
    print(f)

    hcl_sketch = hcl.asarray(np.zeros(N_HASH_FUNC*HASH_RANGE),dtype=dtype)
    hcl_a = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_b = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    hcl_data_key = hcl.asarray(np.random.randint(10, size=(PACKET_SIZE,)), dtype=dtype)
    hcl_query_key = hcl.asarray(np.random.randint(10, size=(QUERY_NUM,)), dtype=dtype)
    hcl_result = hcl.asarray(np.zeros(QUERY_NUM), dtype=dtype)
    hcl_mode = hcl.asarray(np.array([1]), dtype=dtype)
    np_sketch = hcl_sketch.asnumpy()
    np_a = hcl_a.asnumpy()
    np_b = hcl_b.asnumpy()
    np_key = hcl_key.asnumpy()
    np_result = hcl_result.asnumpy()
    np_mode = hcl_mode.asnumpy()
    print(np_sketch)
    print(np_a)
    print(np_b)
    print(np_key)
    print(np_result)
    print(np_mode)

    f(hcl_sketch, hcl_a, hcl_b, hcl_key, hcl_result, hcl_mode)
    np_sketch = hcl_sketch.asnumpy()
    np_a = hcl_a.asnumpy()
    np_b = hcl_b.asnumpy()
    np_key = hcl_key.asnumpy()
    np_result = hcl_result.asnumpy()
    np_mode = hcl_mode.asnumpy()
    print(np_sketch)
    # f = hcl.build(s, target, name='countmin')

    # with open("count_min.cpp", "w") as fp:
    #     fp.write(code)

if __name__ == "__main__":
    func_wrapper(hcl.Int(64))
