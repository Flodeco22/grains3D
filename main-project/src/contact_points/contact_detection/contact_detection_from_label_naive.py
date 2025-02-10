from common import *

############ Parsing Arguments ############

parser = argparse.ArgumentParser(
                    prog = "contact_detection_naive",
                    description = "Detect contacts in a tiff file labeled.")

parser.add_argument("label" , help = "The path to the tiff file labeled.")
parser.add_argument("--output", help = "The path to the output csv file.", default = "../results/contacts_naive.csv")

args = parser.parse_args()
filepath = args.label
outputPath = args.output

############ Loading data ############

with tifffile.TiffFile(filepath) as tif:
    input = np.array([page.asarray() for page in tif.pages])

input = input.astype("int")
x, y, z = input.shape

############ Detecting contacts ############

contactsStrength = defaultdict(int)

def detect_contacts():
    labels = np.copy(input)
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
                    if labels[i,j,k] != 0:
                        detect_contact_on_pixel(i,j,k, contacts)
        
        havingContacts = len(contacts) > 0
        for grain in contacts:
            for neighbor in contacts[grain]:
                contactsStrength[(grain, neighbor)] = contactStrength
        if havingContacts:
            labels = erosion(labels)


def detect_contact_on_pixel(i,j,k, contactDict):
    x, y, z = input.shape

    current_pixel = input[i,j,k]

    # Neightbors in 6-convexity
    current_neighbors = neighborhood_indices((i,j,k))

    for neighbor in current_neighbors:
        if(neighbor[0] < 0 or neighbor[0] >= x or neighbor[1] < 0 or neighbor[1] >= y or neighbor[2] < 0 or neighbor[2] >= z):
            continue

        neighbor_pixel = input[neighbor[0],neighbor[1],neighbor[2]]

        if neighbor_pixel == 0:
            continue

        if current_pixel == neighbor_pixel:
            continue
        
        if not neighbor_pixel in contactDict[current_pixel] and current_pixel < neighbor_pixel:
            contactDict[current_pixel].append(neighbor_pixel)

detect_contacts()

############ Saving results ############

save_results(contactsStrength, outputPath)