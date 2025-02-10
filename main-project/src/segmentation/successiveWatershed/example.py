import os
import argparse
import tifffile # tiff file manipulation
import numpy as np


from utils import *
from watershed_function import watershed


register_filepath = "results"

parser = argparse.ArgumentParser(
                    prog = "example",
                    description = "Compute Watershed on an image")

parser.add_argument("file", help="The path to the tiff file. (image)")
parser.add_argument("adjacency", help="Adjacency of the mask (6 or 26)")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]
adjacency = args.adjacency

with tifffile.TiffFile(filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

os.system("python3 utils/getCentroid.py "+str(filepath)+" "+str(adjacency))


seed_filepath = register_filepath + "/" + filename + "_minTree_segment_raw.tif"
with tifffile.TiffFile(seed_filepath) as tif:
    seed = np.array([page.asarray() for page in tif.pages])

image = (image/256).astype("uint8")
seed = (seed/255).astype("uint8")


csv_centroids_filename = register_filepath + "/" + filename + "_centroids.csv"
centroids = loadCentroidsFromCSV(csv_centroids_filename)

label_of_centroids = np.zeros_like(seed)
for idx, (x, y, z, Label) in enumerate(centroids):
    label_of_centroids[x, y, z] = Label


segmented_image = watershed(image, seed, label_of_centroids, adjacency)

tifffile.imwrite(register_filepath + "/result.tif", segmented_image)
os.system("python3 utils/colormap.py " + register_filepath + "/result.tif")
#os.system("gmic colored.tif a z")
os.system("python3 viewImage/viewImage.py " + register_filepath + "/result.tif")