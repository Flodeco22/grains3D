#!/usr/bin/env python
"""Core extraction with min-tree.

This program uses min-tree filtering for core extraction.
Filtering parameters shoudl automatically adapt with most of
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
                    prog = "mintree",
                    description = "Core extraction with min-tree.",
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

# construct min tree from image
tree, altitudes = hg.component_tree_min_tree(graph, image)

# compute attributes
area = hg.attribute_area(tree)
height = hg.attribute_height(tree, altitudes)

depth = hg.attribute_depth(tree)

max_height = np.max(height)
min_area   = np.min(area)
avg_area   = np.average(area)

# remove unwanted nodes
# the nodes we want to keep have a strong value and a small area
# unwanted_nodes = np.logical_or(height < 0.16*max_height, area > 0.65*avg_area)
unwanted_nodes = np.logical_or(height < 0.14*max_height, area >avg_area)

tree, node = hg.simplify_tree(tree, unwanted_nodes)
new_altitudes = altitudes[node]

# reconstruct tree
res = hg.reconstruct_leaf_data(tree, new_altitudes)
res = (res < np.max(res)).astype(int)*255

output_name = filename + "_mintree_segment_raw.tif"

animation.finished = True
animation.show("Saving result to " + output_name,
    finish_message = "\x1b[2K-- Generated " + output_name + " successfully " + \
                        "(time : %.2f s)" % (timer() - timestamp_0),
    failed_message = "\x1b[2K-- Error occured while saving result.")

# write out file...
tifffile.imwrite(output_name, res)

animation.finished = True
#os.system("gmic " + output_name + " a z")
gc.enable()
exit(0)
