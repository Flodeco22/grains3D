# ========================================================
# Imports
# ========================================================

import numpy as np
import tifffile
from math import floor
from math import ceil

import argparse
import os
import sys

sys.path.insert(0, '../../extern-ressources/code_higra/preprocessing')

from preprocess import read_tiff_data
from preprocess import slice_image
from print_progress_bar import print_bar

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

BLACK_THRESHOLD = (128 << 8)     # Greyscale value between 0 and 2^16-1. The bitshift is there to compare it to 256 more easily
DEBUG = True

# ========================================================
# Functions
# ========================================================

def remove_upper_cylinder(raw_data):
    for z in range(raw_data.shape[0]):
        print_bar(z, raw_data.shape[0], current_operation="Removing upper cylinder...")
        slice = slice_image(raw_data, 0, 0, z)[2]
        for x in range(raw_data.shape[2]):
            y = 0

            # Find cylinder
            while slice[y,x] < BLACK_THRESHOLD and y < raw_data.shape[1]-1:
                y += 1
            # Recolor cylinder pixels
            while slice[y,x] > BLACK_THRESHOLD and y < raw_data.shape[1]-1:
                raw_data[z,y,x] = 0
                y += 1

def remove_lower_cylinder(raw_data):
    for z in range(raw_data.shape[0]):
        slice = slice_image(raw_data, 0, 0, z)[2]
        for x in range(raw_data.shape[2]):
            y = raw_data.shape[1]-1

            # Find cylinder
            while slice[y,x] < BLACK_THRESHOLD and y > 0:
                y -= 1

            # Recolor cylinder pixels
            while slice[y,x] > BLACK_THRESHOLD and y > 0:
                raw_data[z,y,x] = 0
                y -= 1

def write_quadshape_in_greyscale(ydl, yul, ydr, yur, x_start, x_end, raw_data, z, greyscale=BLACK_THRESHOLD):
    if (x_start == x_end):
        return
    coef_dir_up = (yul - yur) / (x_end-x_start)
    coef_dir_down = (ydl - ydr) / (x_end-x_start)

    for x in range(x_start, min(x_end+1, raw_data.shape[2]), 1):
        for y in range(max(0,yul + ceil(coef_dir_up*x)), min(raw_data.shape[1], ydl + floor(coef_dir_down*x) + 1)):
            raw_data[z,y,x] = greyscale

def find_cylinder_bounds(raw_data, z, is_upper_cylinder):
    slice = slice_image(raw_data, 0, 0, z)[2]
    step = 1
    y_start = 0
    y_max = raw_data.shape[1]-1
    if not is_upper_cylinder:
        y_start = raw_data.shape[1]-1
        y_max = 0
        step = -1

    # Middle + cylinder height
    y = y_start
    x = floor(raw_data.shape[2]/2)
    while y != y_max and slice[y,x] < BLACK_THRESHOLD:
        y += step
    yum = y        
    while y != y_max and slice[y,x] > BLACK_THRESHOLD:
        y += step
    ydm = y
    cylinder_height = floor((yum - ydm)/2)

    # Cylinder width # faire left + right d'abord pour trouver coef dir
    x_start = 0
    while x_start < raw_data.shape[2] and slice[cylinder_height,x_start] < BLACK_THRESHOLD:
        x_start += 1
    x_end = raw_data.shape[2]-1
    while x_end > 0 and slice[cylinder_height,x_end] < BLACK_THRESHOLD:
        x_end -= 1

    # Left 
    y = y_start
    x = floor(x_start + (x_end-x_start)/3)
    while y != y_max and slice[y,x] < BLACK_THRESHOLD:
        y += step
    yul = y        
    while y != y_max and slice[y,x] > BLACK_THRESHOLD:
        y += step
    ydl = y

    # Right
    y = y_start
    x += (x-x_start)    # 2/3 (x_end-x_start) + x_start
    while y != y_max and slice[y,x] < BLACK_THRESHOLD:
        y += step
    yur = y        
    while y != y_max and slice[y,x] > BLACK_THRESHOLD:
        y += step
    ydr = y

    # Readjust to middle
    ydl -= ((ydl - ydm) >> 1)
    yul -= ((yul - yum) >> 1)
    ydr -= ((ydr - ydm) >> 1)
    yur -= ((yur - yum) >> 1)

    return (ydl, yul, ydr, yur, x_start, x_end) if is_upper_cylinder else (yul, ydl, yur, ydr, x_start, x_end)

def remove_cylinders(file_path):
    "Deprecated function. Use remove_cylinders_fast."
    
    raw_data = read_tiff_data(file_path)
    raw_data = np.transpose(raw_data, (1,0,2))
    
    remove_upper_cylinder(raw_data)
    remove_lower_cylinder(raw_data)
    return raw_data

def remove_cylinders_fast(filepath):
    raw_data = read_tiff_data(filepath)
    raw_data = np.transpose(raw_data, (1,0,2))
    
    for z in range(raw_data.shape[0]):
        ydl, yul, ydr, yur, x_start, x_end = find_cylinder_bounds(raw_data, z, is_upper_cylinder=True)
        write_quadshape_in_greyscale(ydl, yul, ydr, yur, x_start-50, x_end+50, raw_data, z, greyscale=BLACK_THRESHOLD>>2)

        _, yul, _, yur, x_start, x_end = find_cylinder_bounds(raw_data, z, is_upper_cylinder=False)
        write_quadshape_in_greyscale(raw_data.shape[1], yul, raw_data.shape[1], yur, x_start-50, x_end+50, raw_data, z, greyscale=BLACK_THRESHOLD>>2)
        if not (z+1)%50:
            print_bar(z+1, raw_data.shape[0], current_operation="Removing cylinders...")

    return raw_data

# ========================================================
# Main program
# ========================================================

parser = argparse.ArgumentParser(
                    prog = "remove_cylinder",
                    description = "Removes the white cylinders from an image.",)

parser.add_argument("file", help = "The path to the tiff file.")

args = parser.parse_args()
filepath = args.file
filename = os.path.splitext(os.path.basename(filepath))[0]

raw_data = remove_cylinders_fast(filepath)
tifffile.imwrite(filepath + "_cylinders_removed.tif", raw_data)