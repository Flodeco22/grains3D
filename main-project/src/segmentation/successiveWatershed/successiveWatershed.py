# ========================================================
# Imports
# ========================================================

import os
import argparse
import tifffile # tiff file manipulation
import numpy as np

from utils import *
from watershed_function import watershed



# ========================================================
# Command example
# ========================================================
# python3 successiveWatershed/successiveWatershed.py image.tif 6



# ========================================================
# Main program
# ========================================================

crop_size = 80          # ~2x max length of a grain
register_filepath = "results"


# Recuperation of arguments
parser = argparse.ArgumentParser(
                    prog = "successiveWatershed",
                    description = "Compute successive Watershed on an image")

parser.add_argument("file", help="The path to the tiff file. (image)")
parser.add_argument("adjacency", help="Adjacency of the mask (6 or 26)")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]
adjacency = args.adjacency


# Transformation image.tif to numpy
with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])


# getCentroid
os.system("python3 utils/getCentroid.py "+str(filepath)+" "+str(adjacency))

# Transformation seed.tif to numpy
seed_filepath = register_filepath + "/" + filename + "_minTree_segment_raw.tif"
with tifffile.TiffFile(seed_filepath) as tif:
    seed = np.array([page.asarray() for page in tif.pages])

image = (image/256).astype("uint8")
seed = (seed/255).astype("uint8")


# Load centroids from centroids.csv
csv_centroids_filename = register_filepath + "/" + filename + "_centroids.csv"
centroids = loadCentroidsFromCSV(csv_centroids_filename)

number_of_centroids = len(centroids)


# Creation of a 3D numpy tensor with centroid label at the centroid coordinate 
label_of_centroids = np.zeros_like(seed)
for idx, (x, y, z, Label) in enumerate(centroids):
    label_of_centroids[x, y, z] = Label

labeled_image = np.zeros_like(image)

# For each centroid, we apply a watershed on a crop centered on the centroid
for idx, (x, y, z, Label) in enumerate(centroids):
    croped_image = cropAroundPoint(x, y, z, crop_size, image)
    croped_seed = cropAroundPoint(x, y, z, crop_size, seed)
    croped_label_of_centroids = cropAroundPoint(x, y, z, crop_size, label_of_centroids)

    labeled_crop = watershed(croped_image, croped_seed, croped_label_of_centroids, adjacency)

    # We put the label of the grain on the big labeled_image
    centroid_coord = (x, y, z)
    positioning_label(labeled_image, labeled_crop, centroid_coord, crop_size, Label)

    print(str(idx) + "/" + str(number_of_centroids) + " centroids")
    


# Display image
tifffile.imwrite(register_filepath + "/successiveWatershed_labeled_image.tif", labeled_image)
os.system("python3 utils/colormap.py " + register_filepath + "/successiveWatershed_labeled_image.tif")
#os.system("gmic colored.tif a z")
os.system("python3 viewImage/viewImage.py " + register_filepath + "/successiveWatershed_labeled_image.tif")


