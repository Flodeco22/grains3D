"""
Flatten areas with higher values.

[In progress/not working]
-> refer to BeamHardening program for complete code.
"""
import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings

import argparse
import numpy as np
import skimage.morphology as morph
from skimage.measure import label
from scipy.signal import find_peaks
import tifffile # tiff file manipulation
import os
import matplotlib.pyplot as plt
from scipy import stats

import cv2
import random

import igl 

parser = argparse.ArgumentParser(
                    prog = "uni",
                    description = "test program.",
                    epilog = "For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file", help = "The path to the tiff file (shall be 3D grayscale)")

args = parser.parse_args()
image_filepath = args.file

with tifffile.TiffFile(image_filepath) as tif:
    image = np.array([page.asarray() for page in tif.pages])

image = (image/256).astype("uint8")
print(image.shape)

COM = np.asarray(tuple(map(lambda x : x/2, image.shape)))
print(COM)

# divide image into n sections
n = 50
region_width  = int(image.shape[2] / n)
region_height = int(image.shape[1] / n)
region_depth  = int(image.shape[0] / n)
region_area = region_depth * region_height * region_width

# variables to store results
grains_peaks      = []
void_peaks        = []
distance_from_COM = []

for i in range(n):
    for j in range(n):
        for k in range(n):
            hist = np.array([0 for _ in range (256)])

            # compute distance from COM
            d = np.linalg.norm((i + n/2, j + n/2, k + n/2) - COM)

            # construct histogram
            voxels = image[i:i+region_width, j:j+region_width, k:k+region_width].flatten()
            for voxel in voxels:
                hist[voxel] += 1

            # save peaks with value > 50
            peaks, _ = find_peaks(hist[50:], height=5, distance=100)
            peaks += 50

            if(len(peaks) == 2):


                # plt.title("Histogram") 
                # plt.xlabel("value") 
                # plt.ylabel("n") 
                # plt.plot([i for i in range (256)],hist,"-") 
                # plt.plot(peaks, hist[peaks], "x")
                # plt.show()

                print(peaks)
                distance_from_COM.append(d)

                grains_peaks.append(max(peaks))
                void_peaks.append(min(peaks))

# linear fitting
grains = stats.linregress(grains_peaks, distance_from_COM)
print(grains)

plt.plot(distance_from_COM, grains_peaks, 'o', label='original data')
# print(type(grains.slope))
# print(type(dist))

distance_from_COM = np.array(distance_from_COM)

plt.plot(distance_from_COM, grains.intercept + grains.slope*distance_from_COM, 'r', label='fitted line')

plt.legend()

plt.show()