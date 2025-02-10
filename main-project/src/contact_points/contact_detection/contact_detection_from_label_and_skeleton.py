from common import *
import subprocess

############ Parsing Arguments ############

parser = argparse.ArgumentParser(
                    prog = "contact_detection_using_skeleton",
                    description = "Detect contacts in a tiff file labeled.")

parser.add_argument("grains", help = "The path to the tiff file grains.")
parser.add_argument("label", help = "The path to the tiff file labeled.")
parser.add_argument("x", help = "The x size of the image")
parser.add_argument("y", help = "The y size of the image")
parser.add_argument("z", help = "The z size of the image")
parser.add_argument("--keep_files", action="store_true", help = "If this is given, the temporary files will be kept.")
parser.add_argument("--pink_dir", help = "The path to the pink binary files directory.", default = "../Pink/linux/bin/")
parser.add_argument("--output", help = "The path to the output csv file.", default = "../results/contacts_using_skeleton.csv")

args = parser.parse_args()
grainsPath = args.grains
labelPath = args.label
x = args.x
y = args.y
z = args.z
pinkDir = args.pink_dir
outputPath = args.output

############ minTree and Skeletonization ############

minTreeTiffPath = "tmp/minTree.tif"
minTreeRawPath = "tmp/minTree.raw"
minTreePGMPath = "tmp/minTree.pgm"

subprocess.run("mkdir tmp", shell=True)
subprocess.run("python3 ../utils/minTree.py " + grainsPath + " 6 --output=" + minTreeTiffPath, shell=True) # minTree calculation

subprocess.run("python3 ../utils/tiff2raw.py " + minTreeTiffPath, shell=True) # minTree conversion
subprocess.run(pinkDir + "raw2pgm " + minTreeRawPath + " " + x + " " + y + " " + z + " 0 1 0 " + minTreePGMPath, shell=True) # minTree conversion

binarizedLabelTiffPath = "tmp/label_binarized.tif"
binarizedLabelRawPath = "tmp/label_binarized.raw"
binarizedLabelPGMPath = "tmp/label_binarized.pgm"

subprocess.run("python3 ../utils/tiff_binarization.py " + labelPath + " --output=" + binarizedLabelTiffPath, shell=True) # labels binarization

subprocess.run("python3 ../utils/tiff2raw.py " + binarizedLabelTiffPath, shell=True) # labels conversion
subprocess.run(pinkDir + "raw2pgm " + binarizedLabelRawPath + " " + x + " " + y + " " + z + " 0 1 0 " + binarizedLabelPGMPath, shell=True) # labels conversion

subprocess.run(pinkDir + "skeleton " + binarizedLabelPGMPath + " 6 6 " + minTreePGMPath + " tmp/skeleton.pgm", shell=True) # skeletonization
subprocess.run(pinkDir + "pgm2raw tmp/skeleton", shell=True) # skeleton conversion
skeletonPath = "tmp/skeleton.raw"

############ Loading data ############

with tifffile.TiffFile(labelPath) as tif:
    label = np.array([page.asarray() for page in tif.pages])

with open(skeletonPath, "rb") as raw_file:
    skeleton = np.fromfile(raw_file, dtype=np.uint8)

skeleton = skeleton.reshape(label.shape)

if (label.shape != skeleton.shape):
    raise Exception("Error : label and skeleton sizes do not match !")

label = label.astype("int")
x, y, z = int(x), int(y), int(z)

############ Detecting contacts ############

contactsStrength = defaultdict(int)

def detect_contacts():
    labels = np.copy(label)
    havingContacts = True
    contactStrength = 0
    while(havingContacts): # while contacts are detected, the image is eroded and contacts are searched again
        contacts = defaultdict(list)
        contactStrength += 1
        print(contactStrength)
        havingContacts = False

        for i in range(x):
            for j in range(y):
                for k in range(z):
                    if skeleton[i,j,k] != 0:
                        if labels[i,j,k] != 0:
                            detect_contact_on_pixel(i,j,k, contacts)

        havingContacts = len(contacts) > 0
        for grain in contacts:
            for neighbor in contacts[grain]:
                contactsStrength[(grain, neighbor)] = contactStrength
        if havingContacts:
            labels = erosion(labels)


def detect_contact_on_pixel(i,j,k, contactDict):
    x, y, z = label.shape

    current_pixel = label[i,j,k]

    # Neightbors in 6-convexity
    current_neighbors = neighborhood_indices((i,j,k))

    for neighbor in current_neighbors:
        if(neighbor[0] < 0 or neighbor[0] >= x or neighbor[1] < 0 or neighbor[1] >= y or neighbor[2] < 0 or neighbor[2] >= z):
            continue

        neighbor_pixel = label[neighbor[0],neighbor[1],neighbor[2]]

        if neighbor_pixel == 0:
            continue

        if current_pixel == neighbor_pixel:
            continue
        
        if not neighbor_pixel in contactDict[current_pixel] and current_pixel < neighbor_pixel:
            contactDict[current_pixel].append(neighbor_pixel)

detect_contacts()

if not args.keep_files:
    subprocess.run("rm -r tmp", shell=True)

############ Saving results ############

save_results(contactsStrength, outputPath)