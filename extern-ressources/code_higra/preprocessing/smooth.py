import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings

import argparse
import numpy as np
import skimage.morphology as morph
from skimage.measure import label
import tifffile # tiff file manipulation
import os
from scipy.ndimage import gaussian_filter


import cv2
import random

import igl 

parser = argparse.ArgumentParser(
                    prog = "smooth",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file (shall be 3D grayscale)")

args = parser.parse_args()
image_filepath = args.file

with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

print(image.shape)

image = gaussian_filter(image, 1)
tifffile.imwrite("blurred.tiff", image)
os.system("gmic blurred.tiff a z")
