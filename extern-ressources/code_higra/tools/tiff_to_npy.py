import numpy as np
import tifffile

import argparse
import os


parser = argparse.ArgumentParser(
                    prog = "tiff_to_npy",
                    description = "Converts a tif image into a npy file.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file.")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]

with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

np.save(filename + ".npy", image)