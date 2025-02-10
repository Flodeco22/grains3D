from common import *
import subprocess
from collections import deque
import copy

############ Parsing Arguments ############

parser = argparse.ArgumentParser(
                    prog = "contact_detection_extending_labels",
                    description = "Detect contacts in a tiff file.")

parser.add_argument("grains", help = "The path to the tiff file grains.")
parser.add_argument("x", help = "The x size of the image")
parser.add_argument("y", help = "The y size of the image")
parser.add_argument("z", help = "The z size of the image")
parser.add_argument("--threshold", help = "The threshold value used to identify grains.", default = "27000")
parser.add_argument("--keep_files", action="store_true", help = "If this is given, the temporary files will be kept.")
parser.add_argument("--pink_dir", help = "The path to the pink binary files directory.", default = "../Pink/linux/bin/")
parser.add_argument("--output", help = "The path to the output csv file.", default = "../results/contacts_extending_labels.csv")

args = parser.parse_args()
grainsPath = args.grains
x = args.x
y = args.y
z = args.z
pinkDir = args.pink_dir
threshold = args.threshold
outputPath = args.output

############ minTree, centroids and Skeletonization ############

minTreeTiffPath = "tmp/minTree.tif"
minTreeRawPath = "tmp/minTree.raw"
minTreePGMPath = "tmp/minTree.pgm"

subprocess.run("mkdir tmp", shell=True)
subprocess.run("python3 ../utils/minTree.py " + grainsPath + " 6 --output=" + minTreeTiffPath, shell=True) # minTree calculation

subprocess.run("python3 ../utils/tiff2raw.py " + minTreeTiffPath, shell=True)
subprocess.run(pinkDir + "raw2pgm " + minTreeRawPath + " " + x + " " + y + " " + z + " 0 1 0 " + minTreePGMPath, shell=True) # minTree conversion

centroidsPath = "tmp/centroids.csv"
subprocess.run("python3 ../utils/getCentroid.py " + grainsPath + " " + minTreeTiffPath + " --output=" + centroidsPath, shell=True) # centroids calculation

binarizedGrainsTiffPath = "tmp/grains_binarized.tif"
binarizedGrainsRawPath = "tmp/grains_binarized.raw"
binarizedGrainsPGMPath = "tmp/grains_binarized.pgm"

subprocess.run("python3 ../utils/tiff_binarization.py " + grainsPath + " --threshold=" + threshold + " --output=" + binarizedGrainsTiffPath, shell=True) # grains binarization
subprocess.run("python3 ../utils/tiff_binary_sum.py " + binarizedGrainsTiffPath + " " + minTreeTiffPath + " --output=" + binarizedGrainsTiffPath, shell=True) # grains + minTree
subprocess.run("python3 ../utils/tiff2raw.py " + binarizedGrainsTiffPath, shell=True) # grains conversion
subprocess.run(pinkDir + "raw2pgm " + binarizedGrainsRawPath + " " + x + " " + y + " " + z + " 0 1 0 " + binarizedGrainsPGMPath, shell=True) # grains conversion

subprocess.run(pinkDir + "skeleton " + binarizedGrainsPGMPath + " 6 6 " + minTreePGMPath + " tmp/skeleton.pgm", shell=True) # skeletonization
subprocess.run(pinkDir + "pgm2raw tmp/skeleton", shell=True) # skeleton conversion
skeletonPath = "tmp/skeleton.raw"

############ Loading data ############

with tifffile.TiffFile(binarizedGrainsTiffPath) as tif:
    grains = np.array([page.asarray() for page in tif.pages])


with tifffile.TiffFile(minTreeTiffPath) as tif:
    minTree = np.array([page.asarray() for page in tif.pages])

with open(skeletonPath, "rb") as raw_file:
    skeleton = np.fromfile(raw_file, dtype=np.uint8)

skeleton = skeleton.reshape(grains.shape)

if (grains.shape != skeleton.shape or grains.shape != minTree.shape):
    raise Exception("Error : minTree and skeleton sizes do not match !")

grains = grains.astype("int")
x, y, z = int(x), int(y), int(z)

############ Detecting contacts ############

initialVisited = defaultdict(set)
initialToVisit = defaultdict(set)

with open(centroidsPath, newline="", encoding="utf-8") as f: # read centroids and extend labels on the grains' kernels
    reader = csv.DictReader(f) 
    for line in reader:
        i = int(line["X"])
        j = int(line["Y"])
        k = int(line["Z"])
        label = int(line["Label"])
        initialVisited[label].add((i,j,k))

        kernel = deque()

        current_neighbors = neighborhood_indices((i,j,k))
        for neighbor in current_neighbors:
            if(neighbor[0] < 0 or neighbor[0] >= x or neighbor[1] < 0 or neighbor[1] >= y or neighbor[2] < 0 or neighbor[2] >= z):
                continue
            if skeleton[neighbor[0],neighbor[1],neighbor[2]] != 0:
                if minTree[neighbor[0],neighbor[1],neighbor[2]] != 0:
                    kernel.append(neighbor)
                else:
                    initialToVisit[label].add(neighbor)

        while kernel:
            voxel = kernel.pop()
            initialVisited[label].add(voxel)

            i,j,k = voxel
            current_neighbors = neighborhood_indices((i,j,k))
            for neighbor in current_neighbors:
                if(neighbor[0] < 0 or neighbor[0] >= x or neighbor[1] < 0 or neighbor[1] >= y or neighbor[2] < 0 or neighbor[2] >= z):
                    continue
                if minTree[neighbor[0],neighbor[1],neighbor[2]] != 0:
                    if neighbor not in initialVisited[label]:
                        kernel.append(neighbor)
                else:
                    if skeleton[neighbor[0],neighbor[1],neighbor[2]] != 0:
                        initialToVisit[label].add(neighbor)




contactsStrength = defaultdict(int)

def detect_contacts():
    erodedGrains = np.copy(grains)
    havingContacts = True
    contactStrength = 0
    while(havingContacts): # while contacts are detected, the image is eroded and contacts are searched again
        contacts = defaultdict(set)
        contactStrength += 1
        print(contactStrength)

        havingContacts = False

        visited = copy.deepcopy(initialVisited)
        toVisit = copy.deepcopy(initialToVisit)
        
        while any(len(toVisit[grain]) > 0 for grain in toVisit): # while a label have somewhere to extend
            for grain in toVisit:
                for voxel in set(toVisit[grain]): # Iterate on a copy
                    toVisit[grain].discard(voxel)

                    i = voxel[0]
                    j = voxel[1]
                    k = voxel[2]

                    if (erodedGrains[i,j,k] == 0):
                        continue

                    visited[grain].add(voxel)
                    
                    
                    current_neighbors = neighborhood_indices((i,j,k))

                    for neighbor in current_neighbors:
                        if(neighbor[0] < 0 or neighbor[0] >= x or neighbor[1] < 0 or neighbor[1] >= y or neighbor[2] < 0 or neighbor[2] >= z):
                            continue
                        if neighbor in visited[grain] or neighbor in toVisit[grain]:
                            continue
                        if skeleton[neighbor[0],neighbor[1],neighbor[2]] == 0 or erodedGrains[neighbor[0],neighbor[1],neighbor[2]] == 0: 
                            continue
                        
                        touchingAnotherGrain = False
                        for otherGrain in toVisit:
                            if neighbor in toVisit[otherGrain] or neighbor in visited[otherGrain]:
                                if grain < otherGrain:
                                    contacts[grain].add(otherGrain)
                                else:
                                    contacts[otherGrain].add(grain)
                                touchingAnotherGrain = True
                                break
                        if not touchingAnotherGrain:
                            toVisit[grain].add(neighbor)

        havingContacts = len(contacts) > 0
        for grain in contacts:
            for neighbor in contacts[grain]:
                contactsStrength[(grain, neighbor)] = contactStrength
        if havingContacts:
            erodedGrains = erosion(erodedGrains)

detect_contacts()

if not args.keep_files:
    subprocess.run("rm -r tmp", shell=True)

############ Saving results ############

save_results(contactsStrength, outputPath)