#!/usr/bin/env python
"""Label counter for labelled image.

This programs assumes that an LABELLED image is 
given as a parameter.
More precisely, it returns the number of non-black
grey values contained in the image.
"""
__author__ = "Lysandre Macke"
__contact__ = "lmacke@unistra.com"

# ========================================================
# Imports
# ========================================================

import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings

import argparse
import numpy as np
import tifffile # tiff file manipulation
from dstyle import *

# ========================================================
# Main program
# ========================================================

parser = argparse.ArgumentParser(
                    prog = "Count labels",
                    description = "Label counter for labelled image.",
                    epilog = "For any question please contact me at " + __contact__)

parser.add_argument("file", help = "The path to the tif file.")
args = parser.parse_args()
filepath = args.file

with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

values = set(image.flatten())

print(style.BOLD + "-- Image contains " + str(len(values) - 1) + " labels." + style.NO_COL) # don't take black value into account
exit(0)