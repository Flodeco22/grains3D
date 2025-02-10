import csv

def cropAroundPoint(x, y, z, side, image):
    """
    Crops a cube of size `side` around the point (x, y, z) in the given 3D image.

    Parameters:
        x (int): X-coordinate of the center point.
        y (int): Y-coordinate of the center point.
        z (int): Z-coordinate of the center point.
        side (int): Length of the side of the cube to crop.
        image (numpy.ndarray): 3D numpy array representing the image.

    Returns:
        numpy.ndarray: Cropped 3D cube.
    """

    half_side = side // 2

    # Define the crop boundaries
    x_min = max(0, x - half_side)
    x_max = min(image.shape[0], x + half_side + 1)

    y_min = max(0, y - half_side)
    y_max = min(image.shape[1], y + half_side + 1)

    z_min = max(0, z - half_side)
    z_max = min(image.shape[2], z + half_side + 1)

    # Perform the crop
    cropped_image = image[x_min:x_max, y_min:y_max, z_min:z_max]

    return cropped_image


def loadCentroidsFromCSV(csv_file):
    """
    Load centroids from a CSV file.

    Parameters:
        csv_file (str): Path to the CSV file containing centroids.

    Returns:
        list of tuples: A list of (X, Y, Z, Label) coordinates of centroids and the label.
    """
    centroids = []
    with open(csv_file, mode="r") as file:
        reader = csv.reader(file)
        next(reader)  # Skip the header
        for row in reader:
            x, y, z, Label = map(int, row)
            centroids.append((x, y, z, Label))
    return centroids


def positioning_label(labeled_image, labeled_crop_image, centroid_coord, crop_size, Label):
    """
    Position a label within an image based on centroid coordinates.

    This function adjusts the position of a cropped image within a larger image
    using the provided centroid coordinates and applies a label to the specified
    region.

    Parameters:
        labeled_image (ndarray): The original 3D image array.
        labeled_crop_image (ndarray): The cropped 3D image array to be positioned.
        centroid_coord (tuple): A tuple (x, y, z) representing the centroid coordinates.
        crop_size (int) : The size of the croped image
        label (int): The label value to be applied to the specified region.

    Returns:
        None
    """
    (x, y, z) = centroid_coord
    half_crop_size = crop_size//2

    start_x = max(0, x -  half_crop_size)
    end_x = min(labeled_image.shape[0], x + half_crop_size+1)

    start_y = max(0, y - half_crop_size)
    end_y = min(labeled_image.shape[1], y + half_crop_size+1)

    start_z = max(0, z - half_crop_size)
    end_z = min(labeled_image.shape[2], z + half_crop_size+1)

    # Extract the mask and region
    mask = (labeled_crop_image == Label)
    region = labeled_image[start_x:end_x, start_y:end_y, start_z:end_z]
    region[mask] = Label