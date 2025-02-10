"""
overlap two 3D tif images.
"""

import argparse
import numpy as np

import tifffile # tiff file manipulation

import os

### Load files etc
parser = argparse.ArgumentParser(
                    prog = "segment",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("image_1", help = "The path to the background.")
parser.add_argument("image_2", help = "The path of the foreground.")

args = parser.parse_args()
bg_filepath = args.image_1
fg_filepath  = args.image_2

with tifffile.TiffFile(bg_filepath) as tif:
    background = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(fg_filepath) as tif:
    foreground = np.array([page.asarray() for page in tif.pages])

if(background.ndim != 4):
    im = np.copy(np.expand_dims(background, axis=(3)))
    im = np.repeat(im, 3, axis=3)
else:
    im = np.copy(background)
    
c = background.max()
im[foreground > 0] = [c, 0, 0]

# save
tifffile.imwrite("overlap.tif", im, photometric="rgb")
os.system("gmic overlap.tif a z")