import numpy as np
import tifffile
import argparse

parser = argparse.ArgumentParser(
                    prog = "tiff_binarization",
                    description = "Binarize a tiff file with a threshold")

parser.add_argument("input", help = "The path to a tiff file")
parser.add_argument("--threshold", help = "The threshold used", default = "1")
parser.add_argument("--output", help="Name of the output file.", default=None)

args = parser.parse_args()
inputPath = args.input
threshold = int(args.threshold)

if args.output is not None:
    outputPath = args.output
else:
    outputPath = inputPath[:-4] + "_binarized.tif"

with tifffile.TiffFile(inputPath) as tif:
    input = np.array([page.asarray() for page in tif.pages])


binarized = (input >= threshold) * 255

tifffile.imwrite(outputPath, binarized)