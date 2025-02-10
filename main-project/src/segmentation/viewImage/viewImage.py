import tifffile # tiff file manipulation
import matplotlib.pyplot as plt
import argparse


# Exemple of use : python viewImage imagePath

parser = argparse.ArgumentParser(
                    prog = "viewImage",
                    description = "View 3D image with matplotlib")

parser.add_argument("file", help = "The path to the tiff file")

args = parser.parse_args()
image_filepath = args.file


image = tifffile.imread(image_filepath)


for i in range(len(image)):
    imageTmp = image[i]
    
    #nipy_spectral
    #gray
    plt.imshow(imageTmp, cmap='nipy_spectral')
    plt.title(image_filepath)
    plt.axis('off')
    plt.show()