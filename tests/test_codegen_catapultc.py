import heterocl as hcl
import numpy as np

def test_array_add_const():
    hcl.init()
    A = hcl.placeholder((10,), "A")
    B = hcl.compute(A.shape, lambda x: A[x] + 1, "B")

    s = hcl.create_schedule([A, B])
    # print(hcl.lower(s))

    code = hcl.build(s, target='catapultc')
    print(code)

# def test_scalar_add():
#   hcl.init()
#   a = hcl.placeholder((1, ), "a")
#   b = hcl.placeholder((1, ), "b")
#   c = hcl.placeholder((10, ), "c")

#   def simple_add(a, b, c):
#       c[0] = a[0] + b[0]

#   s = hcl.create_schedule([a, b, c], simple_add)
#   print(hcl.lower(s))

#   target = hcl.platform.aws_f1
#   s.to([a, b], target.xcel)
#   s.to(c, target.host)

#   f = hcl.build(s, target='systemc')
#   print(f)

def test_asic_target():
    hcl.init()
    A = hcl.placeholder((10, 10), "A")
    B = hcl.placeholder((10, 10), "B")
    def kernel(A, B):
        C = hcl.compute(A.shape, lambda i, j: A[i,j] + B[i,j], "C")
        return C
    config = {
        "host" : hcl.dev.asic("mentor"),
        "xcel" : [
            hcl.dev.asic("mentor")
        ]
    }
    p = hcl.platform.custom(config)
    s = hcl.create_schedule([A, B], kernel)
    p.config(compile="catapultc", mode="debug", backend="catapultc")
    code = hcl.build(s, p)
    print(code)

def test_arithmetic():
    def test_scalar_add():
        hcl.init()
        A = hcl.placeholder((1, ), "A")
        B = hcl.placeholder((1, ), "B")
        C = hcl.placeholder((1, ), "C")
        def simple_add(a, b, c):
            c[0] = a[0] + b[0]
        s = hcl.create_schedule([A, B, C], simple_add)
        print(hcl.lower(s))
        # config = {
        #   "host" : hcl.dev.cpu("intel", "e5"),
        #   "xcel" : [
        #       hcl.dev.fpga("xilinx", "xcvu19p")
        #   ]
        # }
        # p = hcl.platform.custom(config)
        # p.config(compile="vitis", mode="debug", backend="catapultc")
        # s.to([A, B], p.xcel)
        # s.to(C, p.host)

        code = hcl.build(s, target="catapultc")
        print(code)

    def test_scalar_mul():
        hcl.init()
        A = hcl.placeholder((1, ), "A")
        B = hcl.placeholder((1, ), "B")
        C = hcl.placeholder((1, ), "C")
        def simple_mul(a, b, c):
            c[0] = a[0] * b[0]
        s = hcl.create_schedule([A, B, C], simple_mul)

        code = hcl.build(s, target="catapultc")
        print(code)

    def test_scalar_mac():
        hcl.init()
        A = hcl.placeholder((1, ), "A")
        B = hcl.placeholder((1, ), "B")
        C = hcl.placeholder((1, ), "C")
        D = hcl.placeholder((1, ), "D")
        def simple_mac(a, b, c, d):
            d[0] = a[0] + (b[0] * c[0])
        s = hcl.create_schedule([A, B, C, D], simple_mac)

        code = hcl.build(s, target="catapultc")
        print(code)

    test_scalar_add()
    test_scalar_mul()
    test_scalar_mac()

def test_pragma():
    hcl.init()
    A = hcl.placeholder((10, 10), "A")
    B = hcl.placeholder((10, 10), "B")
    C = hcl.compute(A.shape, lambda i, j: A[i][j] + B[i][j])
    # unroll
    s1 = hcl.create_schedule([A, B, C])
    s1[C].unroll(C.axis[1], factor=4)
    code1 = hcl.build(s1, target = 'catapultc')
    print(code1)

    # pipeline
    s2 = hcl.create_schedule([A, B, C])
    s2[C].pipeline(C.axis[0], initiation_interval=2)
    code2 = hcl.build(s2, target='catapultc')
    print(code2)

    # partition
    # s3 = hcl.create_schedule([A, B, C])
    # s3.partition(A, hcl.Partition.Block, dim=2, factor=2)
    # code3 = hcl.build(s3, target='catapultc')
    # print(code3)

if __name__ == '__main__':
    #test_array_add_const()
    #test_arithmetic()
    #test_pragma()
    test_asic_target()


