import numpy as np
import os

def compare_edge_curves(edge_curve, prev_edge_curve):
    if prev_edge_curve is None or len(prev_edge_curve) == 0 :
        return np.array([(0, 0)]), edge_curve 

    prev_edge_set = set(map(tuple, prev_edge_curve))
    
    new_tuples = []
    old_tuples = []
    
    for edge in edge_curve:
        if tuple(edge) in prev_edge_set:
            old_tuples.append(edge)
        else:
            new_tuples.append(edge)
    
    new_tuples = np.array(new_tuples, dtype=edge_curve.dtype) if len(new_tuples) > 0 else np.array([(0, 0)])
    old_tuples = np.array(old_tuples, dtype=edge_curve.dtype) if len(old_tuples) > 0 else np.array([(0, 0)])

    return new_tuples, old_tuples

def cloud_nodes_add_quantity(ps_cloud, labels_connected, nb_neighbors_connected) :
    ps_cloud.add_scalar_quantity("Labels", labels_connected)
    ps_cloud.add_scalar_quantity("Number of neighbors", nb_neighbors_connected)


def get_sorted_files(directory):
    all_files = os.listdir(directory)
    
    files_only = []
    
    for f in all_files :
        if os.path.isfile(os.path.join(directory, f)) :
            if ".txt" in f :
                files_only.append(f)
    
    sorted_files = sorted([os.path.join(directory, f) for f in files_only])
    
    return sorted_files