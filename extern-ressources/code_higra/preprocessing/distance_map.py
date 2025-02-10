#!/usr/bin/env python
"""Computing distance map on 3D image.
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
import os

import skimage.morphology as morph
from scipy.ndimage import distance_transform_edt

# ========================================================
# Main program
# ========================================================
parser = argparse.ArgumentParser(
                    prog = "distance map",
                    description = "Computing distance map on 3D image.",
                    epilog = "For any question please contact me at " + __contact__)


parser.add_argument("image", help = "The path to the tiff file to segment.")
parser.add_argument("seed", help = "The path of the tiff file to use as segmentation seed.")

args = parser.parse_args()
image_filepath = args.image
seed_filepath  = args.seed
filename = os.path.splitext(os.path.basename(image_filepath))[0]


with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(seed_filepath) as tif:
    seed = np.array([page.asarray() for page in tif.pages])


# image = (image/256).astype("uint8")

seed= morph.dilation(seed, morph.ball(1.))
image[seed > 0] = 1
image = morph.area_opening(image, 3)

tifffile.imwrite("tmp.tif", image)


dmap = distance_transform_edt(image)
out = filename + "_dmap.tif"
tifffile.imwrite(out, dmap)
print("-- Generated " + out + " successfully.")