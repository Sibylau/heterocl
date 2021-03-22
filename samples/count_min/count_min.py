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
# def hash_family(flow_key, i):
#     aParams = hcl.asarray( \
#         np.random.randint(RAND_SEED, size=(N_HASH_FUNC, )) % primeNumber,\
#         dtype = hcl.UInt(64))
#     bParams = hcl.asarray( \
#         np.random.randint(RAND_SEED, size=(N_HASH_FUNC, )) % primeNumber,\
#         dtype = hcl.UInt(64))

#     # initiation
#     # with hcl.for_(0, N_HASH_FUNC) as depth:
#     #     aParams[depth] = np.random.randint % primeNumber
#     #     bParams[depth] = np.random.randint % primeNumber

#     def universal(flow_key, i):
#         index = ((aParams[i]*flow_key + bParams[i]) % primeNumber) % i
#         return index

def func_wrapper(dtype):
    hcl.init(dtype)
    sketch = hcl.placeholder((N_HASH_FUNC, HASH_RANGE), name="sketch")
    # aParams = hcl.asarray( \
    #     np.random.randint(RAND_SEED, size=(N_HASH_FUNC, )) % primeNumber,\
    #     dtype = hcl.UInt(64))
    # bParams = hcl.asarray( \
    #     np.random.randint(RAND_SEED, size=(N_HASH_FUNC, )) % primeNumber,\
    #     dtype = hcl.UInt(64))
    aParams = hcl.placeholder((N_HASH_FUNC, ))
    bParams = hcl.placeholder((N_HASH_FUNC, ))
    flow_key = hcl.placeholder((PACKET_SIZE, ))
    instr = hcl.placeholder((1, ))
    # def universal(flow_key, i):
    #     index = ((aParams[i]*flow_key + bParams[i]) % primeNumber) % i
    #     return index

    def count_min(sketch, flow_key, aParams, bParams, instr):
        with hcl.if_(instr == UPDATE):
            with hcl.for_(0, PACKET_SIZE) as stream_idx:
                with hcl.for_(0, N_HASH_FUNC) as depth:
                    index = ((aParams[depth]*flow_key[stream_idx] + \
                        bParams[depth]) % primeNumber) % depth
                    sketch[depth][index] += 1
            freq = hcl.compute((1, 0), lambda x: result[0], name="freq")
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
    
    sche_countmin = hcl.create_schedule([hcl_sketch, hcl_flow_key, hcl_aParams, \
        hcl_bParams, hcl_instr], count_min)
    # s_query = hcl.create_schedule([flow_key], query)
    target = hcl.platform.aws_f1
    target.config(compile = 'vitis', mode = 'debug')
    code = hcl.build(sche_countmin, target, name='countmin')
    # code_query = hcl.build(s_query, p, name='query')
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
