import numpy as np
import tifffile
import argparse

parser = argparse.ArgumentParser(
                    prog = "tiff_addition",
                    description = "Sum two tiff file")

parser.add_argument("input1", help = "The path to a tiff file")
parser.add_argument("input2", help = "The path to a tiff file")
parser.add_argument("--output", help = "The path to the output tiff file.", default = "sum.tif")

args = parser.parse_args()
input1Path = args.input1
input2Path = args.input2
outputPath = args.output

with tifffile.TiffFile(input1Path) as tif:
    input1 = np.array([page.asarray() for page in tif.pages])

with tifffile.TiffFile(input2Path) as tif:
    input2 = np.array([page.asarray() for page in tif.pages])

if (input1.shape != input2.shape):
    raise Exception("Error : the sizes do not match !")

sum = ((input1 >= 255) | (input2 >= 255)) * 255

tifffile.imwrite(outputPath, sum)
