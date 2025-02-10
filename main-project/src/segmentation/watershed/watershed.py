__author__ = "Lysandre Macke"
# Adapted by Jonathan Palisse
__contact__ = "lmacke@unistra.com"


# ========================================================
# Imports
# ========================================================

import sys

import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings

import argparse
import numpy as np
import skimage.morphology as morph
from skimage.measure import label, regionprops

from scipy.ndimage import distance_transform_edt
import tifffile # tiff file manipulation

import os


# ========================================================
# Main program
# ========================================================

register_filepath = "results"


parser = argparse.ArgumentParser(
                    prog = "segment",
                    description = "test program.")

parser.add_argument("image", help="The path to the tiff file to segment.")
parser.add_argument("seed", help="The path of the tiff file to use as segmentation seed.")
parser.add_argument("adjacency", help="Adjacency of the mask (6 or 26)")

args = parser.parse_args()
image_filepath = args.image
seed_filepath  = args.seed
adjacency = int(args.adjacency)



with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(seed_filepath) as tif:
    seed = np.array([page.asarray() for page in tif.pages])

if(image.shape != seed.shape):
    print("Error : seed resolution does not match with image !", file=sys.stderr)
    exit()



image = (image/256).astype("uint8")
seed = (seed/255).astype("uint8")
print("loaded image has shape :", image.shape)



with tifffile.TiffFile(image_filepath) as tif:
    print(tif)  # Affiche des informations sur le fichier
    print("Number of pages:", len(tif.pages))
    #for i, page in enumerate(tif.pages):
    #    print(f"Page {i}: shape={page.shape}, dtype={page.dtype}, axes={page.axes}")



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



tree, altitudes = hg.component_tree_max_tree(graph, image)


print("Starting max-tree filtering...")
height = hg.attribute_height(tree, altitudes)
area = hg.attribute_area(tree)

unwanted_nodes = np.logical_or(height > 100, area < 100)

tree, node = hg.simplify_tree(tree, unwanted_nodes)
new_altitudes = altitudes[node]



binary_image = hg.reconstruct_leaf_data(tree, new_altitudes)

binary_image = (binary_image - np.min(image) > 0).astype("uint8")


binary_image = morph.area_closing(binary_image)

tifffile.imwrite(register_filepath + "/tmp_binary.tif", binary_image)
print("Created binary image.")

gradient = binary_image
gradient = distance_transform_edt(binary_image)
gradient = 255 - gradient
tifffile.imwrite(register_filepath + "/tmp_grad.tif", gradient)



tmp = label(seed > 0, background=0, connectivity=1)
regions = regionprops(tmp)



labeled_seeds = np.zeros_like(seed)

for props in regions:
    centroid = np.asarray(props.centroid)
    x, y, z = int(centroid[0]), int(centroid[1]), int(centroid[2])
    labeled_seeds[x, y, z] = tmp[x, y, z]



explicit_graph = graph.as_explicit_graph()

edge_weights = hg.weight_graph(explicit_graph, gradient, hg.WeightFunction.mean)
labels = hg.labelisation_seeded_watershed(explicit_graph, edge_weights, labeled_seeds)

res = np.where(binary_image, labels, 0)



tifffile.imwrite(register_filepath + "/watershed_labeled_image.tif", res)
os.system("python3 utils/colormap.py " + register_filepath + "/watershed_labeled_image.tif")
#os.system("gmic colored.tif a z")
os.system("python3 viewImage/viewImage.py " + register_filepath + "/watershed_labeled_image.tif")

exit(0)



