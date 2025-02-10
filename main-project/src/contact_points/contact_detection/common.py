import numpy as np
import tifffile
import argparse
import csv
from collections import defaultdict

def neighborhood_indices(indices):
    i,j,k = indices
    return [(i-1,j,k),(i+1,j,k),(i,j-1,k),(i,j+1,k),(i,j,k-1),(i,j,k+1)]


def erosion(grains):
    x, y, z = grains.shape
    eroded = np.copy(grains)

    for i in range(x):
        for j in range(y):
            for k in range(z):
                neighbors = neighborhood_indices((i,j,k))

                if grains[i,j,k] == 0:
                    continue
                for neighbor in neighbors:
                    if(neighbor[0] < 0 or neighbor[0] >= x or neighbor[1] < 0 or neighbor[1] >= y or neighbor[2] < 0 or neighbor[2] >= z):
                        continue
                    if grains[neighbor[0],neighbor[1],neighbor[2]] == 0:
                        eroded[i,j,k] = 0
                        break
    return eroded

def save_results(contactsStrength, outputPath):

    contacts = sorted(contactsStrength.keys())

    with open(outputPath, mode='w', newline='', encoding='utf-8') as fichier:
        writer = csv.writer(fichier)
        writer.writerow(["Label1", "Label2", "ContactStrength"])
        for contact in contacts:
            contactStrength = contactsStrength[contact]
            writer.writerow([contact[0], contact[1], contactStrength])