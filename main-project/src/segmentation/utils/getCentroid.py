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

parser.add_argument("file", help="The path to the tiff file. (image)")
parser.add_argument("adjacency", help="Adjacency of the mask (6 or 26)")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]
adjacency = args.adjacency

register_filepath = "results"



os.system("python3 minTree/minTree.py "+str(filepath)+" "+str(adjacency))
filepath_mintree_image = register_filepath + "/" + filename + "_minTree_segment_raw.tif"
with tifffile.TiffFile(filepath_mintree_image) as tif:
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



csv_centroids_filename = register_filepath + "/" + filename + "_centroids.csv"
with open(csv_centroids_filename, mode="w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["X", "Y", "Z", "Label"])
    writer.writerows(centroids)


print(f"Centroids have been saved to {csv_centroids_filename}")