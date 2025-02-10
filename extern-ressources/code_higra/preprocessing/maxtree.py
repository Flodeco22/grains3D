#!/usr/bin/env python
"""Grain binarization with max-tree.

This program uses max-tree filtering for grain binarization.
Filtering parameters are hard coded and adapted for most of
the EFR greyscale subresolutions, but feel free to experiment
with other values if needed.
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
import os
from dstyle import *
from timeit import default_timer as timer
import gc

# ========================================================
# Main program
# ========================================================
parser = argparse.ArgumentParser(
                    prog = "maxtree",
                    description = "Grain binarization with max-tree",
                    epilog = "For any question please contact me at " + __contact__)


parser.add_argument("file", help = "The path to the tiff file.")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]

timestamp_0 = timer()
gc.disable()
animation: TermLoading = TermLoading()
animation.show("Processing " + filename,
    finish_message = "\x1b[2K-- Binarization done.",
    failed_message = "\x1b[2K-- Error occured while processing data.")

# load file and convert to 8 bits
with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

image = (image/256).astype("uint8")

# create res image
res = np.zeros_like(image)

# 6 adjacency implicit graph
mask = [[[0, 0, 0], [0, 1, 0], [0, 0, 0]],
        [[0, 1, 0], [1, 0, 1], [0, 1, 0]],
        [[0, 0, 0], [0, 1, 0], [0, 0, 0]]]
neighbours = hg.mask_2_neighbours(mask)
graph = hg.get_nd_regular_implicit_graph(image.shape, neighbours)

# construct max tree from image
tree, altitudes = hg.component_tree_max_tree(graph, image)

# compute attributes
area = hg.attribute_area(tree)
volume = hg.attribute_volume(tree, altitudes)
height = hg.attribute_height(tree, altitudes)

# remove unwanted nodes
# the nodes we want to keep have a strong value and a small area

# unwanted_nodes = np.logical_or(height < 25, area > 12)
# unwanted_nodes = np.logical_or(height > 100, height < 50)
unwanted_nodes = np.logical_or(height > 100, area < 100)

tree, node = hg.simplify_tree(tree, unwanted_nodes)
new_altitudes = altitudes[node]

# reconstruct and binarize image
res = hg.reconstruct_leaf_data(tree, new_altitudes)
res[res > 0] = 1

animation.finished = True
animation.show("Saving result to res.tif",
    finish_message = "\x1b[2K-- Generated res.tif successfully " + \
                        "(time : %.2f s)" % (timer() - timestamp_0),
    failed_message = "\x1b[2K-- Error occured while saving result.")

# write out file...
tifffile.imwrite("res.tif", res)

animation.finished = True
os.system("gmic res.tif a z")
gc.enable()
exit(0)