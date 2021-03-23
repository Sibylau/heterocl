import heterocl as hcl
import numpy as np
# from hash_family import all

N_HASH_FUNC = 5
HASH_RANGE = 100
PRIME_NUM = 39916801
RAND_SEED = 10000000
PACKET_SIZE = 100

UPDATE = 1
QUERY = 2

def func_wrapper(dtype):
    hcl.init(dtype)
    sketch = hcl.placeholder((N_HASH_FUNC * HASH_RANGE, ), name="sketch")
    aParams = hcl.placeholder((N_HASH_FUNC, ))
    bParams = hcl.placeholder((N_HASH_FUNC, ))
    flow_key = hcl.placeholder((PACKET_SIZE, ))
    instr = hcl.placeholder((1, ))

    def count_min(sketch, flow_key, aParams, bParams, instr):
        result = hcl.compute((1, ), lambda x: 0)
        with hcl.if_(instr == UPDATE):
            with hcl.for_(0, PACKET_SIZE) as p:
                index = hcl.compute(aParams.shape, lambda i: aParams[i]*flow_key[p]+bParams[i])
                hcl.update(result, lambda x: sketch[index[0]] + 1)
                with hcl.for_(0, N_HASH_FUNC) as n:
                    # index = ((aParams[depth]*flow_key[stream_idx] + \
                    #     bParams[depth]) % primeNumber) % depth
                    sketch[n*HASH_RANGE+index] += 1
                    with hcl.if_(sketch[n*HASH_RANGE+index] < result):
                        hcl.update(result, lambda x: sketch[n*HASH_RANGE+index])
                
            freq = hcl.compute((1, 0), lambda x: sketch[0], name="freq")
            with hcl.for_(0, N_HASH_FUNC) as depth:
                with hcl.if_(result[depth] < freq):
                    freq = result[depth]
            return freq
        with hcl.if_(instr == QUERY):
            result = hcl.placeholder((N_HASH_FUNC, 0), name="result")
            with hcl.for_(0, N_HASH_FUNC) as depth:
                index = ((aParams[i]*flow_key + bParams[i]) % primeNumber) % i
                result[depth] = sketch[depth][index]
            # sort - find min
            freq = hcl.compute((1, 0), lambda x: result[0], name="freq")
            with hcl.for_(0, N_HASH_FUNC) as depth:
                with hcl.if_(result[depth] < freq):
                    freq = result[depth]
            return freq
    
    sched_countmin = hcl.create_schedule([sketch, flow_key, aParams, \
        bParams, instr], count_min)
    # s_query = hcl.create_schedule([flow_key], query)
    target = hcl.platform.aws_f1
    target.config(compile = 'vitis', mode = 'debug')
    code = hcl.build(sched_countmin, target, name='countmin')
    # code_query = hcl.build(s_query, p, name='query')
    hcl_sketch = hcl.asarray(np.zeros((N_HASH_FUNC, HASH_RANGE)), dtype=dtype)
    hcl_aParams = hcl.asarray( \
        np.random.randint(RAND_SEED, size=(N_HASH_FUNC, )) % primeNumber,\
        dtype=dtype)
    hcl_bParams = hcl.asarray( \
        np.random.randint(RAND_SEED, size=(N_HASH_FUNC, )) % primeNumber,\
        dtype=dtype)
    hcl_flow_key = hcl.asarray( \
        np.random.randint(20, size=(PACKET_SIZE, )), dtype=dtype)
    hcl_instr = hcl.asarray(np.array([1]))
    with open("count_min.cpp", "w") as fp:
        fp.write(code)
        # fp.write(code_query)

    # def update(flow_key, aParams, bParams, instr):
    #     with hcl.for_(0, PACKET_SIZE) as stream_idx:
    #         with hcl.for_(0, N_HASH_FUNC) as depth:
    #             index = ((aParams[depth]*flow_key[stream_idx] + \
    #                 bParams[depth]) % primeNumber) % depth
    #             sketch[depth][index] += 1
        
    # def query(flow_key, aParams, bParams):
    #     result = hcl.placeholder((N_HASH_FUNC, 0), name="result")
    #     with hcl.for_(0, N_HASH_FUNC) as depth:
    #         index = ((aParams[i]*flow_key + bParams[i]) % primeNumber) % i
    #         result[depth] = sketch[depth][index]
    #     # sort - find min
    #     freq = hcl.compute((1, 0), lambda x: result[0], name="freq")
    #     with hcl.for_(0, N_HASH_FUNC) as depth:
    #         with hcl.if_(result[depth] < freq):
    #             freq = result[depth]
    #     return freq 

if __name__ == "__main__":
    func_wrapper(hcl.Int(64))
