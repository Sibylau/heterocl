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

def test_scalar_add():
  hcl.init()
  a = hcl.placeholder((), "a")
  b = hcl.placeholder((), "b")
  c = hcl.placeholder((10, ), "c")

  def simple_add(a, b, c):
      c[0] = a + b

  s = hcl.create_schedule([a, b, c], simple_add)
  print(hcl.lower(s))

  target = hcl.platform.aws_f1
  s.to([a, b], target.xcel)
  s.to(c, target.host)

  f = hcl.build(s, target='systemc')
  print(f)


if __name__ == '__main__':
  test_scalar_add()
  # test_array_add()

