import tifffile # tiff file manipulation
import numpy as np

import argparse
import os


parser = argparse.ArgumentParser(
                    prog = "convery",
                    description = "Converts a tif image to 8 bit",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file.")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]


with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

# convert to 8 bits
image = (image/256).astype("uint8")

tifffile.imwrite(filename + "_8_bits.tif", image)