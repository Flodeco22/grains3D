"""
Applies a random colormap to the set, while keeping 
black pixels black.
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
lut = {x : [random.randint(0, 255) for _ in range (3)] for x in labels}
lut[0] = [0, 0, 0] # black shall remain black

convert = lambda x : lut[x]

colored = np.copy(np.expand_dims(image, axis=(3)))
colored = np.repeat(colored, 3, axis=3)
colored = colored.astype('uint8')

shape = image.shape
for x in range(shape[0]):
    for y in range(shape[1]):
        for z in range(shape[2]):
            colored[x, y, z] = convert(image[x, y, z])




tifffile.imwrite("colored.tif", colored)