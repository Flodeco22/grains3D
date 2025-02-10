"""
Write your test image, save it as tif.
"""
import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings

import argparse
import numpy as np
import tifffile # tiff file manipulation
import os

res = [[0, 0, 0, 0, 1, 1],
       [0, 0, 0, 1, 1, 1],
       [0, 0, 0, 1, 1, 0],
       [0, 1, 1, 0, 0, 0],
       [1, 1, 1, 0, 0, 0],
       [1, 1, 0, 0, 0, 0]]
tifffile.imwrite("res.tif", res)

# display...
os.system("gmic res.tif a z")