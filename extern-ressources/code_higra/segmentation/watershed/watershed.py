#!/usr/bin/env python
"""Seeded watershed segmentation.

This program uses seeded watershed for grain segmentation.
Takes two input images : a background image to segment and a label image.

The label image needs to have a pure black background for the program
to work.

The image to segment shall not be too big, and may be binary or not (you
are free to comment or uncomment the binarization part.)
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
from skimage.measure import label, regionprops
from skimage.segmentation import watershed
from skimage.feature import peak_local_max

from scipy.ndimage import distance_transform_edt
import tifffile # tiff file manipulation

import os

# ========================================================
# Main program
# ========================================================
parser = argparse.ArgumentParser(
                    prog = "segment",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("image", help = "The path to the tiff file to segment.")
parser.add_argument("seed", help = "The path of the tiff file to use as segmentation seed.")

args = parser.parse_args()
image_filepath = args.image
seed_filepath  = args.seed

with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(seed_filepath) as tif:
    seed = np.array([page.asarray() for page in tif.pages])

# make sure that we work with same image/label resolution
if(image.shape != seed.shape):
    print("Error : seed resolution does not match with image !", file=sys.stderr)
    exit()


###### first : binarize image with maxtree
# convert to 8 bits
image = (image/256).astype("uint8")
seed = (seed/256).astype("uint8")
print("loaded image has shape :", image.shape)

# 6 adjacency implicit graph
mask = [[[0, 0, 0], [0, 1, 0], [0, 0, 0]],
        [[0, 1, 0], [1, 0, 1], [0, 1, 0]],
        [[0, 0, 0], [0, 1, 0], [0, 0, 0]]]
neighbours = hg.mask_2_neighbours(mask)
graph = hg.get_nd_regular_implicit_graph(image.shape, neighbours)

# construct max tree from image
tree, altitudes = hg.component_tree_max_tree(graph, image)

# compute attributes
print("Starting max-tree filtering...")
height = hg.attribute_height(tree, altitudes)
area = hg.attribute_area(tree)

# remove unwanted nodes
unwanted_nodes = np.logical_or(height > 100, area < 100)

tree, node = hg.simplify_tree(tree, unwanted_nodes)
new_altitudes = altitudes[node]

# create res image from tree and computed altitudes
binary_image = hg.reconstruct_leaf_data(tree, new_altitudes)

binary_image = (binary_image - np.min(image) > 0).astype("uint8")*255  # binarize image

##### Some more preprocessing on the binary image to create the gradient
binary_image = morph.area_closing(binary_image)

tifffile.imwrite("tmp_binary.tif", binary_image) # this is our basic maxtree segmentation
print("Created binary image.")

# binary_image = image # uncomment if loaded image is already binary
gradient = binary_image
gradient = distance_transform_edt(binary_image)
gradient = 255 - gradient
tifffile.imwrite("tmp_grad.tif", gradient)


##### compute watershed
# rearrange seeds
tmp = label(seed > 0, background=0, connectivity=1)
regions = regionprops(tmp)

# create centroids image and label it
labeled_seeds = np.zeros_like(seed)
print(labeled_seeds.dtype)
i = 0
for props in regions:
    i += 1
    centroid = np.asarray(props.centroid)
    x, y, z = int(centroid[0]), int(centroid[1]), int(centroid[2])
    labeled_seeds[x, y, z] = tmp[x, y, z]

# seeded watershed
explicit_graph = graph.as_explicit_graph()

edge_weights = hg.weight_graph(explicit_graph, gradient, hg.WeightFunction.mean)
labels = hg.labelisation_seeded_watershed(explicit_graph, edge_weights, labeled_seeds)

res = np.where(binary_image, labels, 0)


tifffile.imwrite("result.tif", res) # final segmentation
os.system("python3 colormap.py result.tif")
os.system("gmic colored.tif a z")

exit(0)