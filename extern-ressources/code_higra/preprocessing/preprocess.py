#!/usr/bin/env python
"""General image preprocessing program.

This program gives a few preprocessing tools for 3D tif images.
"""
__author__ = "Lysandre Macke"
__contact__ = "lmacke@unistra.com"

# ========================================================
# Imports
# ========================================================

import numpy as np
import argparse
import tifffile

import matplotlib.pyplot as plt 
import matplotlib.animation as animation 
import matplotlib.image as img
import sys
import os
from timeit import default_timer as timer
import gc
import threading

from PIL import Image
from scipy import ndimage

from dstyle import *

# ========================================================
# Some global vars
# ========================================================
class style:
    RED     = '\033[0;31m'
    PURPLE  = '\033[0;35m'
    GREEN   = '\033[0;32m'
    NO_COL  = '\033[0m'
    BOLD    = '\033[1m'
    NORMAL  = '\033[0m'

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

def slice_image(image_array, x, y, z):
    """
    Slices the array in three planes according to the given x y z indexes.
    """
    shape = image_array.shape
    if not (0 <= z < shape[0]):
        print(style.RED + style.BOLD + "Error : Bad z index for slicing (must be < " + str(shape[0]) + "). Please check again." + style.NO_COL, file=sys.stderr)
        exit()

    slice_a = [] 
    slice_b = [] 
    slice_c = image_array[z]

    for image in image_array:
        # construct  vertical slices
        slice_a.append(image[x])   # get x row
        slice_b.append(image[:,y]) # get y column

    return (slice_a, slice_b, slice_c)

def load_file_and_save_slice(file, x, y, z, out_dir):
    """
    Whole process of loading/slicing/saving.
    """
    timestamp_0 = timer()
    gc.disable()
    filename = os.fsdecode(file)
    cleaned_filename = os.path.splitext(os.path.basename(filename))[0]
    print(filename)
    filepath = args.file + filename if recursive else file

    # read + slice
    tif = read_tiff_data(filepath)
    shape = tif.shape

    if(not ((0 <= z <= shape[0]) and \
            (0 <= x <= shape[2]) and \
            (0 <= y <= shape[1]))):
        print(style.RED + style.BOLD + "Error : Bad crop coordinates (image has shape" + str(shape) + "). Please check again." + style.NO_COL, file=sys.stderr)
        exit()

    slices = slice_image(tif, x, y, z)

    # save
    for i in range (3):
        tifffile.imwrite(out_dir + cleaned_filename + "_slice_" + chr(ord ('x') + i) + ".tif",
            slices[i])

    print(style.BOLD + style.GREEN + "-- Sliced " + cleaned_filename + " successfully (time : " + str(timer() - timestamp_0) + " s)."+ style.NORMAL)
    gc.enable()

def crop_image(image_array, x1, y1, x2, y2, z1, z2):
    """
    crop a single tif file
    """
    sequence = []
    for i, image in enumerate(image_array):
        if(z1 <= i < z2):
            croped = image[x1:x2, y1:y2]
            sequence.append(croped)

    return np.array(sequence)

def load_file_and_save_croped(file, x1, y1, x2, y2, z1, z2, out_dir):
    """
    Whole process of loading/cropping/saving
    """
    timestamp_0 = timer()
    gc.disable()
    filename = os.fsdecode(file)
    cleaned_filename = os.path.splitext(os.path.basename(filename))[0]
    print(filename)
    filepath = args.file + filename if recursive else file

    # read + crop
    tif = read_tiff_data(filepath)
    shape = tif.shape

    if(not ((0 <= z1 < z2 <= shape[0]) and \
        (0 <= x1 < x2 <= shape[2]) and \
        (0 <= y1 < y2 <= shape[1]))):
        print(style.RED + style.BOLD + "Error : Bad crop coordinates (image has shape" + str(shape) + "). Please check again." + style.NO_COL, file=sys.stderr)
        exit()
    
    croped_image = crop_image(tif, x1, y1, x2, y2, z1, z2)

    # save
    tifffile.imwrite(out_dir + cleaned_filename + "_croped.tif", croped_image, bigtiff=True)
    
    print(style.BOLD + style.GREEN + "-- Generated " + cleaned_filename + "_croped.tif successfully (time : " + str(timer() - timestamp_0) + " s)."+ style.NORMAL)
    gc.enable()

def gaussian_blur(image_array,sigma = 1):
    """
    Apply Gaussian blur to the image.
    """
    return ndimage.gaussian_filter(image_array, sigma)

def sample_naive(image_array, n):
    extracted = []
    for i, page in enumerate(image_array):
        if(i%n == 0):
            compressed = page[::n, ::n]
            extracted.append(compressed)

    return np.array(extracted)

def load_file_and_save_naive_filter(file, n, out_dir):
    """
    Whole process of loading/compressing/saving
    """
    timestamp_0 = timer()
    gc.disable()
    filename = os.fsdecode(file)
    cleaned_filename = os.path.splitext(os.path.basename(filename))[0]
    print(filename)
    filepath = args.file + filename if recursive else file

    # read + sample
    animation.show("Processing " + filename,
        finish_message = "-- Downsampled data successfully.",
        failed_message = "-- Error occured while processing data.")

    tif = read_tiff_data(filepath)
    compressed = gaussian_blur(tif, n)
    compressed = sample_naive(tif, n)

    animation.finished = True

    # save
    out_file = cleaned_filename + "_naive_" + args.n + ".tif"
    animation.show("Saving result to " + out_dir + out_file,
        finish_message = "-- Generated " + out_file + " successfully " + \
                         "(time : " + str(timer() - timestamp_0) + " s).",
        failed_message = "-- Error occured while saving result.")

    tifffile.imwrite(out_dir + out_file, compressed)
    animation.finished = True
    gc.enable()

def sample_min(image_array, n):
    res = ndimage.minimum_filter(image_array, size = n)
    res = sample_naive(res, n)
    return res

def load_file_and_save_min_filter(file, n, out_dir):
    """
    Whole process of loading/compressing/saving
    """
    timestamp_0 = timer()
    gc.disable()

    filename = os.fsdecode(file)
    cleaned_filename = os.path.splitext(os.path.basename(filename))[0]
    filepath = args.file + filename if recursive else file

    # read + sample
    animation.show("Processing " + filename,
        finish_message = "-- Downsampled data successfully.",
        failed_message = "-- Error occured while processing data.")

    tif = read_tiff_data(filepath)
    compressed = sample_min(tif, n)

    animation.finished= True
    
    # save
    out_file = cleaned_filename + "_min_" + args.n + ".tif"
    animation.show("Saving result to " + out_dir + out_file,
        finish_message = "-- Generated " + out_file + " successfully " + \
                         "(time : " + str(timer() - timestamp_0) + " s).",
        failed_message = "-- Error occured while saving result.")

    tifffile.imwrite(out_dir + out_file, compressed)
    animation.finished = True
    gc.enable()

# ========================================================
# Main program
# ========================================================

animation: TermLoading = TermLoading()

# globals
recursive = True

# parse arguments
parser = argparse.ArgumentParser(
                    prog = "preprocess",
                    description = "Apply some preprocess functions to an image.",
                    epilog = "For any question please contact me at " + __contact__)

parser.add_argument("file", help = "The path to the tiff file to preprocess")
subparsers = parser.add_subparsers(dest='command')

parser.add_argument("-r",
                    dest = "recursive",
                    action='store_const',
                    default = not(recursive),
                    const = recursive,
                    help    = "Apply to directory recursively.")

crop = subparsers.add_parser("crop")
crop.add_argument("x1",  help = "x coordinate of the first point.")
crop.add_argument("y1",  help = "y coordinate of the first point.")
crop.add_argument("x2",  help = "x coordinate of the second point.")
crop.add_argument("y2",  help = "y coordinate of the second point.")
crop.add_argument("z1",  help = "min z coordinate.")
crop.add_argument("z2",  help = "max z coordinate.")

downsample = subparsers.add_parser("downsample")
downsample.add_argument("type", choices = ['naive', "min"])
downsample.add_argument("n",    help = "divide resolution by n.")

slice = subparsers.add_parser("slice")
slice.add_argument("x", help = "x index to do the slicing")
slice.add_argument("y", help = "y index to do the slicing")
slice.add_argument("z", help = "z index to do the slicing")


args = parser.parse_args()
recursive = args.recursive

if(recursive):
    filepath = os.fsencode(args.file)
    out_dir = args.file.removesuffix("/") + "_" + args.command + "/"
    if not os.path.exists(out_dir):
        os.mkdir(out_dir)
else:
    filepath = args.file
    out_dir = "./"

# Apply given command
if args.command == 'crop':
    # store into local variables for easier read
    z1 = int(args.z1)
    z2 = int(args.z2)
    x1 = int(args.x1)
    x2 = int(args.x2)
    y1 = int(args.y1)
    y2 = int(args.y2)

    if(recursive):
        for file in os.listdir(filepath):
            load_file_and_save_croped(file, x1, y1, x2, y2, z1, z2, out_dir)
    else:
        load_file_and_save_croped(filepath, x1, y1, x2, y2, z1, z2, out_dir)

if(args.command == 'downsample'):
    n = int(args.n)

    if(recursive):
        if(args.type == 'naive'):
            for file in os.listdir(filepath):
                load_file_and_save_naive_filter(file, n, out_dir)
        if(args.type == 'min'):
            for file in os.listdir(filepath):
                load_file_and_save_min_filter(file, n, out_dir)

    else:
        if(args.type == 'naive'):
            load_file_and_save_naive_filter(filepath, n, out_dir)
        if(args.type == 'min'):
            load_file_and_save_min_filter(filepath, n, out_dir)

if(args.command == 'slice'):
    x = int(args.x)
    y = int(args.y)
    z = int(args.z)

    if(recursive):
        for file in os.listdir(filepath):
            load_file_and_save_slice(file, x, y, z, out_dir)
    else:
        load_file_and_save_slice(filepath, x, y, z, out_dir)