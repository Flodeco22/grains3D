# ========================================================
# Imports
# ========================================================

import numpy as np
import tifffile

import argparse
import os
import sys

sys.path.insert(0, '../../extern-ressources/code_higra/preprocessing')

from preprocess import read_tiff_data

# ========================================================
# Main program
# ========================================================

def rotateXaxis(filepath):
    raw_data = read_tiff_data(filepath)
    raw_data = np.transpose(raw_data, (1,0,2))
    return raw_data

parser = argparse.ArgumentParser(
                    prog = "rotateXaxis",
                    description = "Rotates the image 90 degrees on its X axis. The top view will become a side view.",)

parser.add_argument("file", help = "The path to the tiff file.")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]

raw_data = rotateXaxis(filepath)
tifffile.imwrite(filepath + "_rotated.tif", raw_data)