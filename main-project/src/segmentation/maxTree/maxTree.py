"""
This file is a copy of the maxtree_new.py program.


New maxtree segmentation : considering a core segmentation, 
add it to the original image putting its value as a maximum value.
The cores will correspond to leaves and the grains will be direct parents.
"""


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


# ========================================================
# Main program
# ========================================================

register_filepath = "results"


parser = argparse.ArgumentParser(
                    prog = "maxTree",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file (shall be 3D grayscale)")
parser.add_argument("markers", help = "Path to core segmentation (background shall be black)")
parser.add_argument("adjacency", help="Adjacency of the mask (6 or 26)")

args = parser.parse_args()
image_filepath = args.file
seed_filepath = args.markers
adjacency = args.adjacency



# image = cv2.imread(filepath, cv2.IMREAD_GRAYSCALE)
with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(seed_filepath) as tif:
    cores = np.array([page.asarray() for page in tif.pages])

# convert to 8 bits
image = (image/256).astype("uint")
print("loaded image has shape :", image.shape)



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



# overlap two images
cores = morph.dilation(cores, morph.ball(2.2))

components = label(cores, return_num=True)
total = components[1]
print("number of cores in the image :", total)

cores_val = image.max() + 1
image[cores > 0] = cores_val

tifffile.imwrite(register_filepath + "/tmp.tiff", image)



# construct max tree from image
tree, altitudes = hg.component_tree_max_tree(graph, image)



parents = tree.parents()
parents_size = len(parents)
labels = [0 for _ in range(parents_size)]
count  = [0 for _ in range(parents_size)]

depth = hg.attribute_depth(tree)
min_depth = 0.05*depth.max()

label_index = 0

print("start computing attributes")
# place attribute
for leaf in tree.leaves():
    # if we are in a new core region
    if(altitudes[leaf] == cores_val and count[tree.parent(leaf)] == 0):
        node = leaf
        while(node != tree.root()):
            count[node] += 1
            labels[node] = label_index
            node = tree.parent(node)
            
        label_index += 1



print("filtering...")
for node in tree.leaves_to_root_iterator(include_leaves = False, include_root = True):
    if(count[node] > 1):
        labels[node] = 0



labels = np.asarray(labels)
res = hg.reconstruct_leaf_data(tree, labels)
tifffile.imwrite(register_filepath + "/maxTree_result.tif", res)



# print some info
components = label(res, return_num=True)
total = components[1]
print("number of components in the image :", total)
print("number of labels :", max(labels))