"""
Applies a colormap to the set. 
The colormap values are proportionnal with the image 
greyscale values.
"""
import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings

import argparse
import numpy as np
import tifffile # tiff file manipulation
import os

import random
import matplotlib.cm as cm

parser = argparse.ArgumentParser(
                    prog = "colormap",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file (shall be 3D grayscale)")

args = parser.parse_args()
image_filepath = args.file

with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

print(image.shape)

labels = np.array(list(set(image.flatten())))
maxlab = labels.max()

cmap = cm.hsv
norm = Normalize(vmin=labels.min(), vmax=labels.max())


colored = np.copy(np.expand_dims(image, axis=(3)))
colored = np.repeat(colored, 3, axis=3)
colored = colored.astype('uint8')

shape = image.shape
for x in range(shape[0]):
    for y in range(shape[1]):
        for z in range(shape[2]):
            color = cmap(image[x, y, z])
            colored[x, y, z] = [255*color[i] for i in range (3)]


tifffile.imwrite("colored_uni.tif", colored)