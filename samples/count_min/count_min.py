import heterocl as hcl
import numpy as np
# from hash_family import all

N_HASH_FUNC = 5
HASH_RANGE = 10
PRIME_NUM = 39916801
RAND_SEED = 1000
PACKET_SIZE = 10
QUERY_NUM = 5

UPDATE = 1
QUERY = 2
UPDATE_QUERY = 3

def func_wrapper(dtype):
    hcl.init(dtype)
    
    a=hcl.placeholder((N_HASH_FUNC,), dtype=dtype, name="a")
    b=hcl.placeholder((N_HASH_FUNC,), dtype=dtype, name="b")
    sketch = hcl.placeholder((N_HASH_FUNC, HASH_RANGE), dtype=dtype, name="sketch")
    data_key=hcl.placeholder((PACKET_SIZE,),dtype=dtype, name="data_key")
    query_key=hcl.placeholder((QUERY_NUM,),dtype=dtype, name="query_key")
    result=hcl.placeholder((QUERY_NUM,),dtype=dtype, name="result")
    mode = hcl.placeholder((1, ), name="mode")
    # mode = hcl.scalar(name="mode")

    def hash_index(sketch, a, b, data_key, query_key, result, mode):
        with hcl.if_(mode[0] == UPDATE):
            with hcl.for_(0, PACKET_SIZE) as p:
                index = hcl.compute(a.shape, lambda i: (a[i]*data_key[p] + b[i]))
                with hcl.for_(0, N_HASH_FUNC) as i:
                    sketch[i][index[i]] += 1
                # hcl.update(sketch, lambda i: sketch[i*HASH_RANGE+index[i]] + 1, name="update")
        with hcl.if_(mode[0] == QUERY):
            with hcl.for_(0, QUERY_NUM) as p:
                index = hcl.compute(a.shape, lambda i: (a[i]*query_key[p] + b[i]))
                result[p] = sketch[0][index[0]]
                with hcl.for_(1, N_HASH_FUNC) as n:
                    with hcl.if_(result[p] > sketch[n][index[n]]):
                        result[p] = sketch[n][index[n]]
        with hcl.if_(mode[0] == UPDATE_QUERY):
            with hcl.for_(0, PACKET_SIZE) as p:
                index = hcl.compute(a.shape, lambda i: (a[i]*data_key[p] + b[i])%HASH_RANGE)
                with hcl.for_(0, N_HASH_FUNC) as i:
                    sketch[i][index[i]] = sketch[i][index[i]] + 1
            with hcl.for_(0, QUERY_NUM) as q:
                index = hcl.compute(a.shape, lambda i: (a[i]*query_key[q] + b[i])%HASH_RANGE)
                result[q] = sketch[0][index[0]]
                with hcl.for_(1, N_HASH_FUNC) as n:
                    with hcl.if_(result[q] > sketch[n][index[n]]):
                        result[q] = sketch[n][index[n]]
    
    s = hcl.create_schedule([sketch, a, b, data_key, query_key, result, mode], hash_index)
    print(hcl.lower(s))
    # target = "llvm"
    # target = hcl.platform.aws_f1
    # target.config(compile='vitis', mode='sw_sim')
    config = {"host": hcl.dev.asic("mentor"), "xcel": [hcl.dev.asic("mentor")]}
    target = hcl.platform.custom(config)
    target.config(compile="catapultc", mode="sw_sim", backend="catapultc")
    f = hcl.build(s, target, name='countmin')
    print(f)

    hcl_sketch = hcl.asarray(np.zeros((N_HASH_FUNC,HASH_RANGE)),dtype=dtype)
    hcl_a = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    # hcl_a = hcl.asarray(np.array([7,3,3,1,7]), dtype=dtype)
    hcl_b = hcl.asarray(np.random.randint(10, size=(N_HASH_FUNC,)), dtype=dtype)
    # hcl_b = hcl.asarray(np.array([9,9,4,1,2]), dtype=dtype)
    # hcl_data_key = hcl.asarray(np.array([5,3,7,1,0,0,7,4,3,3]), dtype=dtype)
    hcl_data_key = hcl.asarray(np.random.randint(10, size=(PACKET_SIZE,)), dtype=dtype)
    # hcl_query_key = hcl.asarray(np.array([1,2,3,4,5]), dtype=dtype)
    hcl_query_key = hcl.asarray(np.random.randint(10, size=(QUERY_NUM,)), dtype=dtype)
    hcl_result = hcl.asarray(np.zeros(QUERY_NUM), dtype=dtype)
    hcl_mode = hcl.asarray(np.array([3]), dtype=dtype)
    np_sketch = hcl_sketch.asnumpy()
    np_a = hcl_a.asnumpy()
    np_b = hcl_b.asnumpy()
    np_data_key = hcl_data_key.asnumpy()
    np_query_key = hcl_query_key.asnumpy()
    np_result = hcl_result.asnumpy()
    np_mode = hcl_mode.asnumpy()
    print(np_sketch)
    print("a:", np_a)
    print("b:", np_b)
    print("data:", np_data_key)
    print("query:", np_query_key)
    print("result:", np_result)
    print("mode:", np_mode)

    f(hcl_sketch, hcl_a, hcl_b, hcl_data_key, hcl_query_key, hcl_result, hcl_mode)
    np_sketch = hcl_sketch.asnumpy()
    np_a = hcl_a.asnumpy()
    np_b = hcl_b.asnumpy()
    np_data_key = hcl_data_key.asnumpy()
    np_query_key = hcl_query_key.asnumpy()
    np_result = hcl_result.asnumpy()
    np_mode = hcl_mode.asnumpy()
    print(np_sketch)
    print("a:", np_a)
    print("b:", np_b)
    print("data:", np_data_key)
    print("query:", np_query_key)
    print("result:", np_result)
    print("mode:", np_mode)
    # f = hcl.build(s, target, name='countmin')

    # with open("count_min.cpp", "w") as fp:
    #     fp.write(code)

if __name__ == "__main__":
    func_wrapper(hcl.Int(32))
