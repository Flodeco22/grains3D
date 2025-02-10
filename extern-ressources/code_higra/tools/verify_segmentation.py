#!/usr/bin/env python
"""Segmentation validation helper.

Compares a segmentation with a given ground truth (reference labels).
The labels do not necesseraly need to match for the segmentation to be correct,
as long as the two images contains exactly the same number of labels and each
object position match from one image to another.

However, if the given segmentation has missing labels, the corresponding grains
will be highlighted in the grains_missed.tif output file.

To manually check if there are false positives (i.e. segmented objects that are not
grains according to the given ground truth), a tmp file is created by overlapping
the two given images.

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
import skimage.morphology as morph
from skimage.measure import label
import tifffile # tiff file manipulation

from collections import Counter
from dstyle import *
import cv2

import igl 

# ========================================================
# Main program
# ========================================================

parser = argparse.ArgumentParser(
                    prog = "Segmentation validation helper.",
                    description = "Compares a segmentation with a given ground truth (reference labels)." + \
                    "Will display labels informations and will create temporary files. See file header for more details.",
                    epilog = "For any question please contact me at " + __contact__)

parser.add_argument("segmentation", help = "The path to the segmentation file")
parser.add_argument("ground_truth", help = "The reference labels.")


args = parser.parse_args()

with tifffile.TiffFile(args.segmentation) as tif:
    seg = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(args.ground_truth) as tif:
    lab = np.array([page.asarray() for page in tif.pages])

# discard color channel if exists
if(len(seg.shape) == 4):
    seg = seg[:, :, :, 0]
    
# make sure that we work with same image resolution
if(seg.shape != lab.shape):
    print(style.RED + style.BOLD + "Error : two images resolution does not match !" + \
    style.NO_COL, file=sys.stderr)
    exit()

# expected number of labels
labels_groundtruth = set(lab.flatten())
print("Expecting " + str(len(labels_groundtruth) - 1) + " labels.")


# use segmentation as mask :
# apply dilation to make sure to catch the label for each core
seg = morph.binary_dilation(seg, morph.ball(radius=1))

# change ground truth background to make difference with segmentation
c = np.max(lab)
lab[lab == 0] = c + 1

mask = seg > 0
image = lab[mask]

image = np.where(mask, lab, 0)

# compare number of labels
labels_segment = set(image.flatten())
n_seg = len(labels_segment) - 1
print("Got " + str(len(labels_segment) - 1) + ".")

n_components = label(seg, return_num=True)[1]
print("Number of components :", n_components)


not_detected = labels_groundtruth.difference(labels_segment)
n_not_detected = len(not_detected)
print(str(n_not_detected) + " grains are not detected.")
if(n_not_detected > 0):
    print(not_detected)

# contruct res images
im = np.zeros_like(lab)

for z in range (seg.shape[0]):
    for y in range (seg.shape[1]):
        for x in range (seg.shape[2]):
            if(lab[z, y, x] in not_detected):
                im[z, y, x] = lab[z, y, x]

tifffile.imwrite("grains_missed.tif", im)

im = np.copy(np.expand_dims(lab, axis=(3)))
im = np.repeat(im, 3, axis=3)
im[seg > 0] = [0, c, 0]
im[image == c + 1] = [c, 0, 0] # false positives will be displayed in red

# save
tifffile.imwrite("tmp.tif", im, photometric="rgb")
print("Please check tmp.tif for false positives (displayed in red).")

exit(0)