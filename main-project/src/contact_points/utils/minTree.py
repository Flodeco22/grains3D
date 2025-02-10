__author__ = "Lysandre Macke"
# Adapted by Jonathan Palisse
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
                    prog = "minTree",
                    description = "Core extraction with minTree.")

parser.add_argument("file", help="The path to the tiff file.")
parser.add_argument("adjacency", help="Adjacency of the mask (6 or 26)")
parser.add_argument("--output", help="Name of the output file.", default=None)

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]

if args.output is not None:
    output_name = args.output
else:
    output_name = filename + "_minTree_segment_raw.tif"

adjacency = args.adjacency




timestamp_0 = timer()
gc.disable()


with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

image = (image/256).astype("uint8")



res = np.zeros_like(image)

if (adjacency == 26):
    # 26 adjacency implicit graph
    mask = [[[1, 1, 1], [1, 1, 1], [1, 1, 1]],
            [[1, 1, 1], [1, 0, 1], [1, 1, 1]],
            [[1, 1, 1], [1, 1, 1], [1, 1, 1]]]
else:
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
unwanted_nodes = np.logical_or(height < 0.14*max_height,   area > avg_area)

tree, node = hg.simplify_tree(tree, unwanted_nodes)
new_altitudes = altitudes[node]



# reconstruct tree
res = hg.reconstruct_leaf_data(tree, new_altitudes)
res = (res < np.max(res)).astype(int)*255


# write out file...
tifffile.imwrite(output_name, res)

gc.enable()
exit(0)
