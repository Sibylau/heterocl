import heterocl as hcl
import hlib
import numpy as np
import sys
from resnet_const import *

target = hcl.platform.vlab
target.config(compile="aocl", mode="hw_sim")
if not args.opt:
    resnet20 = build_resnet20_inf(target)
else:
    print("Use streaming")
    resnet20 = build_resnet20_stream_inf(target)
print("Finish building function.")

images, labels = next(iter(test_loader))
np_image = np.array(images)
labels = np.array(labels)
hcl_image = hcl.asarray(np_image, dtype=qtype_float)
resnet20(hcl_image, hcl_out)
print("Done synthesis.")
