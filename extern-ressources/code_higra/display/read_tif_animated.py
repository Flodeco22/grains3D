# setup
import numpy as np
import argparse
import tifffile

import matplotlib.pyplot as plt 
import matplotlib.animation as animation 


# some global vars
class style:
    RED     = '\033[0;31m'
    PURPLE  = '\033[0;35m'
    GREEN   = '\033[0;32m'
    NO_COL  = '\033[0m'
    BOLD    = '\033[1m'
    NORMAL  = '\033[0m'

# program use checks
parser = argparse.ArgumentParser(
                    prog = "Read_Tif_Animated",
                    description="Reads a tif image sequence and displays it as an animation.",
                    epilog="For any question please contact me at lysandre.macke@unicaen.fr.")

parser.add_argument("file",
                    help = "The path to the tif image to use.")

args = parser.parse_args()

fig, ax = plt.subplots()

sequence = []
with tifffile.TiffFile(args.file) as tif:
    for page in tif.pages:
        image = page.asarray()
        image = ax.imshow(image, animated=True)
        sequence.append([image])

ani = animation.ArtistAnimation(fig, sequence, interval=50, blit=True,
                                repeat_delay=1000)
plt.show()