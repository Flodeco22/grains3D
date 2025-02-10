"""
Test program based on the tos_segment script.
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
if(image.shape[0] == 1):
    image = image[0]
    tree, altitudes = hg.component_tree_tree_of_shapes_image2d(image)
else:
    tree, altitudes = hg.component_tree_tree_of_shapes_image3d(image)


altitudes = altitudes.astype(int) # allow us to go further than 255

# compute attributes, initialize arrays etc



parents = tree.parents()

area = hg.attribute_area(tree).astype(int)
height = hg.attribute_height(tree, altitudes).astype(int)
volume = hg.attribute_volume(tree, altitudes).astype(int)
avg_area   = np.average(area)

cores_alt = max(altitudes) + 1



cores_alt = max(altitudes) + 1

tmp = np.array([0 for _ in range(len(altitudes))])


####################################################################
### first : go down the tree to get the nodes corresponding to cores
print("Search for cores...")
for leaf in tree.leaves():
    node = tree.parent(leaf)

    if(is_new_core(node, tree)):
        subtree, subnodes = tree.sub_tree(node)

        # mark each node corresponding to core
        tmp[node] = 1
        tmp[subnodes] = 1

# dilate, smooth, overlap, reconstruct tree
cores_image = hg.reconstruct_leaf_data(tree, tmp)
cores_image = morph.dilation(cores_image, morph.ball(3.))

# os.system("python3 smooth.py " + image_filepath)
# with tifffile.TiffFile("blurred.tiff") as tif:
#     image = np.array([page.asarray() for page in tif.pages])

# image = morph.closing(image)
cleaned = image
footprint = morph.ball(3)

print("here")
# deal with black noise
cleaned = morph.dilation(cleaned, footprint)
cleaned = morph.reconstruction(cleaned, image, method="erosion", footprint=footprint)

tmp = cleaned.copy()

# deal with white noise
cleaned = morph.erosion(cleaned, footprint)
cleaned = morph.reconstruction(cleaned, tmp, method="dilation", footprint=footprint)
image = cleaned

# image[cores_image == 1] = cores_alt
tree, altitudes = hg.component_tree_tree_of_shapes_image3d(image)

tifffile.imwrite("cleaned.tiff", image)
os.system("gmic cleaned.tiff a z")
exit(0)

cores = []
core_nodes = np.array([0 for _ in range(len(altitudes))]) # nodes we want to keep

# create list of highest nodes corresponding to cores
for leaf in tree.leaves():
    node = tree.parent(leaf)

    if(altitudes[node] == cores_alt and node not in cores):
        cores.append(node)
        core_nodes[node] = 1


print("n cores : ", len(cores))


tmp = hg.reconstruct_leaf_data(tree, core_nodes)
tifffile.imwrite("cores.tif", tmp)
# os.system("gmic cores.tif a z")
# exit(0)



####################################################################
### then : go from cores to root to propagate labels
print("Propagate labels...")

labels = np.array([0 for _ in range(len(altitudes))])
count  = np.array([0 for _ in range(len(altitudes))])

label_index = 1

core_index = 18
i = 0

# for core in cores:
#     if(i == core_index):

#         _, subnodes = tree.sub_tree(core)
#         labels[subnodes] = label_index
#         # print("ancestors :", len(tree.ancestors(core)))
#         for node in tree.ancestors(core):
#             count[node] += 1
#             labels[node] = label_index
            
#             # if(label_index == 40):
#             #     break
#             label_index += 1
#         break
#     i += 1


for core in cores:
    _, subnodes = tree.sub_tree(core)
    labels[subnodes] = label_index
    for node in tree.ancestors(core):
        count[node] += 1
        labels[node] = label_index
        
    label_index += 1

print("propagated labels :", label_index - 1)

tmp_img = hg.reconstruct_leaf_data(tree, labels)
tifffile.imwrite("tmp.tif", tmp_img)
os.system("python3 colormap.py tmp.tif")

# os.system("gmic colored.tif a z")
# exit(0)

####################################################################
### finally, keep highest node corresponding to grain
print("filtering...")

grains = []

visited = np.array([0 for _ in range(len(altitudes))]) 

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

        # visited[node] = 255

        grains.append(tmp)

# visited[tree.root()] = 255
print("grains :", len(grains))
tmp = hg.reconstruct_leaf_data(tree, visited)
tifffile.imwrite("visiited.tif", tmp)
os.system("python3 colormap.py visiited.tif")
os.system("gmic colored.tif a z")
exit(0)

deleted = np.array([True for _ in range(len(altitudes))]) # which node should be deleted 
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