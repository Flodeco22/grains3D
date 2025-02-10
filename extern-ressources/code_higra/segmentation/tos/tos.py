"""
New segmentation using tree of shapes.
"""
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

import cv2
import time

import igl 

def is_new_core(node, tree):
    # return area[node] < 100*avg_area and area[node] > 10 # for test.tiff
    return altitudes[node] > 100 and altitudes[node] < 150 and area[node] < 3*avg_area and area[node] > 10 # for cube
    # return area[node] < 10 and height[node] < 10 and altitudes[node] < 150 and altitudes[node] > 100 # for square
    # return area[node] < 1000 and altitudes[node] < 50

parser = argparse.ArgumentParser(
                    prog = "tos_segmentation",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file (shall be 3D grayscale)")

args = parser.parse_args()
image_filepath = args.file

with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

# image = image[0][:, :, 0]
# image = image[0]
print(image.shape)

# construct ToS
start = time.time()
tree, altitudes = hg.component_tree_tree_of_shapes_image3d(image)
end = time.time()
print("It took", end - start, "seconds!")
exit(0)

altitudes = altitudes.astype(int) # allow us to go further than 255


# compute attributes, initialize arrays etc
parents = tree.parents()
parents_size = len(parents)

area = hg.attribute_area(tree).astype(int)
height = hg.attribute_height(tree, altitudes).astype(int)
avg_area   = np.average(area)

cores_alt = max(altitudes) + 1

####################################################################
### first : go down the tree to get the nodes corresponding to cores
print("Search for cores...")
for leaf in tree.leaves():
    node = tree.parent(leaf)

    if(is_new_core(node, tree)):
        altitudes[node] = cores_alt
        subtree, subnodes = tree.sub_tree(node)
        altitudes[subnodes] = cores_alt

cores_max = []
cores_min = []
core_nodes = np.array([0 for _ in range(parents_size)]) # nodes we want to keep

# create list of highest nodes corresponding to cores
for leaf in tree.leaves():
    if(altitudes[leaf] == cores_alt):
        node = leaf
        tmp = node
        while(altitudes[node] == cores_alt):
            tmp = node
            node = tree.parent(node)
        
        if(tmp not in cores_max):
            cores_max.append(tmp)
            core_nodes[tmp] = len(tree.ancestors(tmp))
            subtree, subnodes = tree.sub_tree(tmp)
            core_nodes[subnodes] = len(tree.ancestors(tmp))

            _, subnodes = tree.sub_tree(tmp)
            cores_min.append(min(subnodes))

cores = cores_max

print("n cores : ", len(cores))


tmp = hg.reconstruct_leaf_data(tree, core_nodes)
tifffile.imwrite("cores.tif", tmp)
# os.system("gmic cores.tif a z")
# exit(0)

####################################################################
### then : go from cores to root to propagate labels
print("Propagate labels...")

labels = np.array([0 for _ in range(parents_size)])
count  = np.array([0 for _ in range(parents_size)])

label_index = 1

for core in cores:
    _, subnodes = tree.sub_tree(core)
    labels[subnodes] = label_index
    print("ancestors :", len(tree.ancestors(core)))
    for node in tree.ancestors(core):
        count[node] += 1
        labels[node] = label_index
    label_index += 1

print("propagated labels :", label_index - 1)

tmp_img = hg.reconstruct_leaf_data(tree, labels)
tifffile.imwrite("tmp.tif", tmp_img)
# os.system("gmic tmp.tif a z")
# exit(0)

####################################################################
### finally, keep highest node corresponding to grain
print("filtering...")

grains = []

visited = np.array([0 for _ in range(parents_size)]) 

# find highest node corresponding to grains
for core in cores:
        tmp = core
        node = tree.parent(core)
        while(count[node] == 1 and node != tree.root()):
            tmp = node
            node = tree.parent(node)

        visited[tmp] = labels[core]
        _, subnodes = tree.sub_tree(tmp)
        visited[subnodes] = labels[core]

        grains.append(tmp)

# visited[tree.root()] = 255
print("grains :", len(grains))
tmp = hg.reconstruct_leaf_data(tree, visited)
tifffile.imwrite("visiited.tif", tmp)
os.system("python3 colormap.py visiited.tif")
os.system("gmic colored.tif a z")
exit(0)

deleted = np.array([True for _ in range(parents_size)]) # which node should be deleted 
# deleted[grains] = False
# deleted[tree.root()] = False

for grain in grains:
    sub_tree, subnodes = tree.sub_tree(grain)
    deleted[subnodes] = True
    deleted[grain] = False
    

tmp = hg.reconstruct_leaf_data(tree, 1*deleted)
tifffile.imwrite("deleted.tif", tmp)

tree, nodes = hg.simplify_tree(tree, deleted)
labels = altitudes[nodes]
# labels[tree.root()] = 0


res = hg.reconstruct_leaf_data(tree, labels)

# print("number of labels in the image : ",  len(set(labels)))

tifffile.imwrite("res.tif", res)

# display...
os.system("python3 colormap.py res.tif")
os.system("gmic colored.tif a z")