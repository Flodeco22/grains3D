# ========================================================
# Imports
# ========================================================

from skimage.measure import regionprops, label
import os
import argparse
import tifffile # tiff file manipulation
import numpy as np
import csv


# ========================================================
# Main program
# ========================================================

parser = argparse.ArgumentParser(
                    prog = "getCentroid",
                    description = "Extract the coord of the center of the black part of each grain")

parser.add_argument("grains", help="The path to the tiff file of the grains. (image)")
parser.add_argument("minTree", help="The path to the tiff file of the minTree. (image)")
parser.add_argument("--output", help="Name of the output file.", default="centroids.csv")


args = parser.parse_args()
grainPath = args.grains
minTreePath = args.minTree
outputPath = args.output


with tifffile.TiffFile(minTreePath) as tif:
    mintree_image = np.array([page.asarray() for page in tif.pages])



tmp = label(mintree_image > 0, background=0, connectivity=1)
regions = regionprops(tmp)

labeled_centroids = np.zeros_like(mintree_image)
centroids = []
for props in regions:
    centroid = np.asarray(props.centroid)
    x, y, z = int(centroid[0]), int(centroid[1]), int(centroid[2])
    Label = tmp[x, y, z]
    centroids.append((x, y, z, Label))



with open(outputPath, mode="w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["X", "Y", "Z", "Label"])
    writer.writerows(centroids)