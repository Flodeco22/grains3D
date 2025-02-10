import numpy as np
import tifffile

import argparse
import os


parser = argparse.ArgumentParser(
                    prog = "npy_to_tiff",
                    description = "Converts a npy dump into a tif image.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the dump file.")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]


image = np.load(filepath)
tifffile.imwrite(filename + ".tiff", image)