import heterocl as hcl
import numpy as np

INC = 1
DEC = 2

def func_wrapper(dtype):
    hcl.init(dtype)
    
    a=hcl.placeholder((2,), dtype=dtype, name="a")
    b=hcl.placeholder((2,), dtype=dtype, name="a")
    mode = hcl.placeholder((1, ), name="mode")

    def test(a, b, mode):
        with hcl.if_(mode[0] == INC):
            with hcl.for_(0, 2) as i:
                b[i] = a[i] + 1
        with hcl.if_(mode[0] == DEC):
            with hcl.for_(0, 2) as i:
                b[i] = a[i] - 1
    
    s = hcl.create_schedule([a, b, mode], test)
    # target="vhls"
    # target = hcl.platform.aws_f1
    # target.config(compile='vhls', mode='debug')
    # f = hcl.build(s, target="vhls")
    config = {"host": hcl.dev.asic("mentor"), "xcel": [hcl.dev.asic("mentor")]}
    target = hcl.platform.custom(config)
    target.config(compile="catapultc", mode="debug", backend="catapultc")
    f = hcl.build(s, target, name='test')
    print(f)
    # hcl_a = hcl.asarray(np.random.randint(10, size=(2,)), dtype=dtype)
    # hcl_b = hcl.asarray(np.zeros(2), dtype=dtype)
    # hcl_mode = hcl.asarray(np.array([1]), dtype=dtype)

    # f(hcl_a, hcl_b, hcl_mode)
    # np_a = hcl_a.asnumpy()
    # np_b = hcl_b.asnumpy()
    # np_mode = hcl_mode.asnumpy()
    # print("a:", np_a)
    # print("b:", np_b)
    # print("mode:", np_mode)

def test_pack():

    def pack(A):
        return hcl.pack(A, factor=5)

    A = hcl.placeholder((40,), "A", dtype = hcl.UInt(3))
    s = hcl.create_schedule([A], pack)
    code = hcl.build(s, target="vhls")
    slice_range = "(((i * 3) + 2), (i * 3))"
    assert slice_range in code

if __name__ == "__main__":
    func_wrapper(hcl.Int(32))
    # test_pack()
