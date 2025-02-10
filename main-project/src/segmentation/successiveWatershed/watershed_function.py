import numpy as np
import warnings
with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import higra as hg # ignore the generated warnings
import skimage.morphology as morph
from scipy.ndimage import distance_transform_edt
import sys



register_filepath = "results"


def watershed(image, seed, labeled_seeds, adjacency):
    '''
    Crops a cube of size `side` around the point (x, y, z) in the given 3D image.

    Parameters:
        image (numpy.ndarray): image to apply watershed on.
        seed (numpy.ndarray): image with mintree apply on.
        labeled_seeds (numpy.ndarray): 3D tab the size of the image. Values: centroid label on centroid coordinates; 0 otherwise.
        adjacency (int): Adjacency of the mask (6 or 26).

    Returns:
        numpy.ndarray: Cropped 3D cube.
    '''


    if(image.shape != seed.shape):
        print("Error : seed resolution does not match with image !", file=sys.stderr)
        exit()



    res = np.zeros_like(image)

    if (adjacency == 26):
        # 26 adjacency implicit graph
        mask = [[[1, 1, 1], [1, 1, 1], [1, 1, 1]],
                [[1, 1, 1], [1, 0, 1], [1, 1, 1]],
                [[1, 1, 1], [1, 1, 1], [1, 1, 1]]]
    else:
        # 6 adjacency implicit graph
        mask = [[[0, 0, 0], [0, 1, 0], [0, 0, 0]],
                [[0, 1, 0], [1, 0, 1], [0, 1, 0]],
                [[0, 0, 0], [0, 1, 0], [0, 0, 0]]]

    neighbours = hg.mask_2_neighbours(mask)
    graph = hg.get_nd_regular_implicit_graph(image.shape, neighbours)



    tree, altitudes = hg.component_tree_max_tree(graph, image)


    print("Starting max-tree filtering...")
    height = hg.attribute_height(tree, altitudes)
    area = hg.attribute_area(tree)

    unwanted_nodes = np.logical_or(height > 100, area < 100)

    tree, node = hg.simplify_tree(tree, unwanted_nodes)
    new_altitudes = altitudes[node]



    binary_image = hg.reconstruct_leaf_data(tree, new_altitudes)

    binary_image = (binary_image - np.min(image) > 0).astype("uint8")


    binary_image = morph.area_closing(binary_image)


    gradient = binary_image
    gradient = distance_transform_edt(binary_image)
    gradient = 255 - gradient


    explicit_graph = graph.as_explicit_graph()

    edge_weights = hg.weight_graph(explicit_graph, gradient, hg.WeightFunction.mean)
    labels = hg.labelisation_seeded_watershed(explicit_graph, edge_weights, labeled_seeds)

    res = np.where(binary_image, labels, 0)

    return res