from math import *
import tifffile # tiff file manipulation
import numpy as np
import os


def gaussian(x, y, z, sigma):
    return (1/(sqrt(2*pi)*sigma)**3) * exp(-(x**2 + y**2 + z**2)/(2 * sigma**2))

sigma = 10
width = height = depth = 100

center_A = (width/3, height/2, depth/2)
center_B = (width/3 + width/4, height/2, depth/2 + 1)
center_C = (0, height/3, depth/2)

centers = [center_A, center_B]

N = len(centers)


image = np.zeros((width, height, depth), dtype = float)

for n in range(N):
    for i in range (width):
        for j in range (height):
            for k in range (depth):
                center = centers[n]

                x = i - center[0]
                y = j - center[1]
                z = k - center[2]
                image[i, j, k] += max(gaussian(x, y, z, sigma) - gaussian(x, y, z, sigma/10), 0)

image = (255*(image - np.min(image))/np.ptp(image)).astype(int) # normalize
tifffile.imwrite(str(N) + "gaussian.tiff", image)
os.system("gmic " + str(N) + "gaussian.tiff a z")