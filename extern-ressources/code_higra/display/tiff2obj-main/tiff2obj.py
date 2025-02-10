#!/usr/bin/env python

"""Tiff to obj converter.

A basic python script to convert a multipage tiff file (e.g. 3d scan) into a
3D obj file.

Open source rocks ! This program is free to use, redistribute, modify.
Merge requests are gladly appreciated and encouraged.

"""

__author__  = "Lysandre Macke"
__contact__ = "lysandre.macke@unicaen.fr"
__version__ = "0.0.1" 

#######################################################################

import argparse
import tifffile
import numpy as np
import pyvista as pv

from dstyle import *

def main():
    """
    Main program.
    """
    
    animation: TermLoading = TermLoading()
    
    # parse arguments
    parser = argparse.ArgumentParser(
                        prog = "tiff2obj",
                        description = "Tiff to obj converter.",
                        epilog = "For any question please contact me at "\
                        + __contact__)

    parser.add_argument("file",
                        help    = "the path to the tiff file")
    parser.add_argument("-o",
                        dest    = "output",
                        help    = "specify output file name (default : output.obj)",
                        default = "output.obj")

    args     = parser.parse_args()
    filepath = args.file
    outname  = args.output

    # outname checks...
    if outname.split(".")[-1] != "obj":
        print_err("Bad output file extension. Valid extensions are : .obj")
        exit(1)
    


    # convert
    animation.show('Converting file',
                finish_message = "-- Converted tif file into 3D grid successfully.",
                failed_message = "-- Error while converting to 3D grid.")

    with tifffile.TiffFile(filepath) as tif:
        raw_data = np.array([page.asarray() for page in tif.pages])

    grid = pv.ImageData()
    grid.dimensions = np.array(raw_data.shape) + 1
    
    grid.cell_data["values"] = raw_data.flatten(order = "F")

    threshed = grid.threshold_percent(0.6)
    surface = threshed.extract_surface()
    animation.finished = True

    threshed.plot(cmap='viridis', show_scalar_bar=False, show_edges=True)

    # save obj file
    animation.show('Writing output',
                finish_message = "-- Generated file " + outname + " successfully.",
                failed_message = "-- Error while writing output file.")

    pv.save_meshio(outname, surface)
    animation.finished = True

    mesh = pv.read("output.obj")
    mesh.plot()

if __name__ == "__main__":
    main()