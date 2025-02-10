# ========================================================
# Imports
# ========================================================

import os
import argparse
import tifffile # tiff file manipulation
import numpy as np
import csv
import sys 

sys.path.insert(0, './segmentation/successiveWatershed')

from utils import loadCentroidsFromCSV

# ========================================================
# Functions
# ========================================================

def read_tiff_data(filepath):
    """
    Reads tiff and returns its values into a 3D np.array.
    """
    with tifffile.TiffFile(filepath) as tif:
        raw_data = np.array([page.asarray() for page in tif.pages])
    return raw_data

def find_cylinder_bounds(image, z, is_upper_cylinder):
    step = 1
    y_start = 0
    y_max = image.shape[1]-1
    if not is_upper_cylinder:
        y_start = image.shape[1]-1
        y_max = 0
        step = -1

    # Middle + cylinder height
    y = y_start
    x = image.shape[2]//2
    while y != y_max and image[x,y,z] < BLACK_THRESHOLD:
        y += step
    yum = y        
    while y != y_max and image[x,y,z] > BLACK_THRESHOLD:
        y += step
    ydm = y
    cylinder_height = (yum - ydm)//2

    # Cylinder width
    x_start = 0
    while x_start < image.shape[2] and image[x_start,cylinder_height,z] < BLACK_THRESHOLD:
        x_start += 1
    x_end = image.shape[2]-1
    while x_end > 0 and image[x_end,cylinder_height,z] < BLACK_THRESHOLD:
        x_end -= 1

    # Left 
    y = y_start
    x = x_start + (x_end-x_start)//3
    while y != y_max and image[x,y,z] < BLACK_THRESHOLD:
        y += step
    yul = y        
    while y != y_max and image[x,y,z] > BLACK_THRESHOLD:
        y += step
    ydl = y

    # Right
    y = y_start
    x += (x-x_start)    # 2/3 (x_end-x_start) + x_start
    while y != y_max and image[x,y,z] < BLACK_THRESHOLD:
        y += step
    yur = y        
    while y != y_max and image[x,y,z] > BLACK_THRESHOLD:
        y += step
    ydr = y

    # Readjust to middle
    ydl -= ((ydl - ydm) >> 1)
    yul -= ((yul - yum) >> 1)
    ydr -= ((ydr - ydm) >> 1)
    yur -= ((yur - yum) >> 1)

    return (ydl, yul, ydr, yur, x_start, x_end) if is_upper_cylinder else (yul, ydl, yur, ydr, x_start, x_end)

def compute_average(ydl, yul, ydr, yur, x_start, x_end, z):
    x_average = x_end - x_start
    y_average = min(ydl,ydr) - max(yul,yur)
    z_average = z
    return (x_average, y_average, z_average)

def add_cylinder_centroids(image, centroids, label_of_centroids):

    # Add centroid to upper cylinder
    z = image.shape[2]//2
    ydl, yul, ydr, yur, x_start, x_end = find_cylinder_bounds(image, z, is_upper_cylinder=True)
    upper_cylinder_center = compute_average(ydl, yul, ydr, yur, x_start, x_end, z)
    #label_of_centroids[upper_cylinder_center] = CYLINDER_LABEL_HIGH    # not needed in the current state of this program. May be useful for future needs.
    upper_cylinder_center += (CYLINDER_LABEL_HIGH,)

    # Add centroid to lower cylinder
    ydl, yul, ydr, yur, x_start, x_end = find_cylinder_bounds(image, z, is_upper_cylinder=False)
    lower_cylinder_center = compute_average(ydl, yul, ydr, yur, x_start, x_end, z)
    #label_of_centroids[lower_cylinder_center] = CYLINDER_LABEL_LOW     # not needed in the current state of this program. May be useful for future needs.
    lower_cylinder_center += (CYLINDER_LABEL_LOW,)

    centroids.append(upper_cylinder_center)
    centroids.append(lower_cylinder_center)
            
    return centroids, label_of_centroids

# ========================================================
# Main program
# ========================================================

BLACK_THRESHOLD = (128 << 8)     # Greyscale value between 0 and 2^16-1. The bitshift is there to compare it to 256 more easily
CYLINDER_LABEL_HIGH = -1
CYLINDER_LABEL_LOW = -2

parser = argparse.ArgumentParser(
                    prog = "apply_cylinder_centroid_label",
                    description = "Adds extra centroids in cylinders with a special label.")

parser.add_argument("file", help="The path to the watershed tiff file. (image)")
parser.add_argument("centroids", help="The path to the centroids file. (centroids)")
parser.add_argument("seed", help="The path to the mintree file. (seed)")

args = parser.parse_args()
filepath = args.file
centroids_path = args.centroids
seed_path = args.seed
centroids_name = os.path.splitext(os.path.basename(centroids_path))[0]

raw_data = read_tiff_data(filepath)
seed_data = read_tiff_data(seed_path)

seed_data = (seed_data/255).astype("uint8")
centroids = loadCentroidsFromCSV(centroids_path)

label_of_centroids = np.zeros_like(seed_data)
for idx, (x, y, z, Label) in enumerate(centroids):
    label_of_centroids[x, y, z] = Label

centroids, label_of_centroids = add_cylinder_centroids(raw_data, centroids, label_of_centroids)

with open(centroids_name + '_with_cylinder_labels.csv','w') as out:
    csv_out=csv.writer(out)
    csv_out.writerow(['x','y','z','Label'])
    for row in centroids:
        csv_out.writerow(row)

    print("Cylinder labels added in new CSV file: ", centroids_name + '_with_cylinder_labels.csv')