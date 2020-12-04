import heterocl as hcl
import numpy as np

def test_array_add():
  A = hcl.placeholder((10,), "A")

  def simple_add(A):
      return hcl.compute(A.shape, lambda x: A[x] + 1, "B")
      #return hcl.compute(A.shape, lambda x: A[x] + 1, "B")

  s = hcl.create_schedule([A], simple_add)
  print(hcl.lower(s))

  f = hcl.build(s, target='systemc')
  print(f)

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

def test_scalar_add():
  hcl.init()
  A = hcl.placeholder((1, ), "A")
  B = hcl.placeholder((1, ), "B")
  C = hcl.placeholder((10, ), "C")
  def simple_add(a, b, c):
      c[0] = a[0] + b[0]
  s = hcl.create_schedule([A, B, C], simple_add)
  print(hcl.lower(s))
  config = {
    "host" : hcl.dev.cpu("intel", "e5"),
    "xcel" : [
        hcl.dev.fpga("xilinx", "xcvu19p")
    ]
  }
  p = hcl.platform.custom(config)
  p.config(compile="vitis", mode="debug", backend="systemc")
  s.to([A, B], p.xcel)
  s.to(C, p.host)
  f = hcl.build(s, target=p)
  print(f)

if __name__ == '__main__':
  test_scalar_add()
  # test_array_add()

