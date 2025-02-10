import polyscope as ps
import numpy as np
import plotly.express as px
import plotly.graph_objects as go
from grain import *
import os
import time
import polyscope.imgui as psim
import argparse

def get_points_diff_same(grains1, grains2) :
    grains1_coords = {(grain.x, grain.y, grain.z): grain for grain in grains1.values()}
    grains2_coords = {(grain.x, grain.y, grain.z): grain for grain in grains2.values()}
    
    coords1 = set(grains1_coords.keys())
    coords2 = set(grains2_coords.keys())
    
    unique_coords1 = coords1 - coords2
    unique_coords2 = coords2 - coords1
    common_coords = coords1 & coords2
    
    grains_diff1 = [grains1_coords[coord] for coord in unique_coords1]
    grains_diff2 = [grains2_coords[coord] for coord in unique_coords2]
    grains_same = [grains1_coords[coord] for coord in common_coords]

    return grains_diff1, grains_diff2, grains_same

def get_coords_points(grains) :
    return np.array([[grain.x, grain.y, grain.z] for grain in grains])

def get_labels_points(grains) :
    return np.array([grain.label for grain in grains])

def generate_points_ps(grains1, grains2) :
    global prev_edges_co

    grains_diff1, grains_diff2, grains_same = get_points_diff_same(grains1, grains2)
    
    points_grains_diff1 = get_coords_points(grains_diff1)
    labels_grains_diff1 = get_labels_points(grains_diff1)

    points_grains_diff2 = get_coords_points(grains_diff2)
    labels_grains_diff2 = get_labels_points(grains_diff2)

    points_grains_same = get_coords_points(grains_same)
    labels_grains_same = get_labels_points(grains_same)
    
    return points_grains_diff1, labels_grains_diff1, points_grains_diff2, labels_grains_diff2, points_grains_same, labels_grains_same

def cloud_nodes_add_quantity(ps_cloud, labels_grains) :
    ps_cloud.add_scalar_quantity("Labels", labels_grains)

def prepare_display_ps(points_grains_diff1, labels_grains_diff1, points_grains_diff2, labels_grains_diff2, points_grains_same, labels_grains_same, display=False) :

    points_ignored = np.array([(-9999.9, -9999.9, 9999.9)])
    labels_ignored = np.array([-1])
    
    if len(points_grains_diff1) > 0 :
        ps_cloud_diff1 = ps.register_point_cloud("Grains from graph 1", points_grains_diff1, radius=0.0025, color=(0.0, 0.0, 1.0))
        cloud_nodes_add_quantity(ps_cloud_diff1, labels_grains_diff1)
    else :
        ps_cloud_diff1 = ps.register_point_cloud("Grains from graph 1", points_ignored, radius=0.0025, color=(0.0, 1.0, 0.0), enabled=False)
        cloud_nodes_add_quantity(ps_cloud_diff1, labels_ignored)

    if len(points_grains_diff2) > 0 :
        ps_cloud_diff2 = ps.register_point_cloud("Grains from graph 2", points_grains_diff2, radius=0.0025, color=(1.0, 0.0, 0.0))
        cloud_nodes_add_quantity(ps_cloud_diff2, labels_grains_diff2)
    else :
        ps_cloud_diff2 = ps.register_point_cloud("Grains from graph 2", points_ignored, radius=0.0025, color=(0.0, 1.0, 0.0), enabled=False)
        cloud_nodes_add_quantity(ps_cloud_diff2, labels_ignored)

    if len(points_grains_same) > 0 :
        ps_cloud_same = ps.register_point_cloud("Grains with same coords from both graphs", points_grains_same, radius=0.0025, color=(0.0, 1.0, 0.0))
        cloud_nodes_add_quantity(ps_cloud_same, labels_grains_same)
    else :
        ps_cloud_same = ps.register_point_cloud("Grains with same coords from both graphs", points_ignored, radius=0.0025, color=(0.0, 1.0, 0.0), enabled=False)
        cloud_nodes_add_quantity(ps_cloud_same, labels_ignored)

    if display :
        ps.show()

def update_display_ps(points_grains_diff1, labels_grains_diff1, points_grains_diff2, labels_grains_diff2, points_grains_same, labels_grains_same, display=False) :

    points_ignored = np.array([(-9999.9, -9999.9, 9999.9)])
    labels_ignored = np.array([-1])
    
    if len(points_grains_diff1) > 0 :
        ps_cloud_diff1 = ps.register_point_cloud("Grains from graph 1", points_grains_diff1, enabled=True)
        cloud_nodes_add_quantity(ps_cloud_diff1, labels_grains_diff1)
    else :
        ps_cloud_diff1 = ps.register_point_cloud("Grains from graph 1", points_ignored, enabled=False)
        cloud_nodes_add_quantity(ps_cloud_diff1, labels_ignored)

    if len(points_grains_diff2) > 0 :
        ps_cloud_diff2 = ps.register_point_cloud("Grains from graph 2", points_grains_diff2, enabled=True)
        cloud_nodes_add_quantity(ps_cloud_diff2, labels_grains_diff2)
    else :
        ps_cloud_diff2 = ps.register_point_cloud("Grains from graph 2", points_ignored, enabled=False)
        cloud_nodes_add_quantity(ps_cloud_diff2, labels_ignored)

    if len(points_grains_same) > 0 :
        ps_cloud_same = ps.register_point_cloud("Grains with same coords from both graphs", points_grains_same, enabled=True)
        cloud_nodes_add_quantity(ps_cloud_same, labels_grains_same)
    else :
        ps_cloud_same = ps.register_point_cloud("Grains with same coords from both graphs", points_ignored, enabled=False)
        cloud_nodes_add_quantity(ps_cloud_same, labels_ignored)
    
    if display :
        ps.show()

def get_sorted_files(directory):
    all_files = os.listdir(directory)
    
    files_only = []
    
    for f in all_files :
        if os.path.isfile(os.path.join(directory, f)) :
            if ".txt" in f :
                files_only.append(f)
    
    sorted_files = sorted([os.path.join(directory, f) for f in files_only])
    
    return sorted_files

def update_points(toNextFrame = True):
    global all_coordinates, current_frame, nb_frames

    if toNextFrame :
        current_frame = (current_frame + 1) % (nb_frames)
    
    update_display_ps(*all_coordinates[current_frame], display=False)

def user_callback():
    global last_update_time, isStopped, ui_angle_rad, duration, current_frame

    if(psim.Button("Start" if isStopped else "Stop")):
        isStopped = False if isStopped else True

    psim.SameLine()

    if(psim.Button("Reset")):
        current_frame = 0
        update_points(toNextFrame=False)
        last_update_time = time.time()
        
    psim.Separator()
    
    _, duration = psim.SliderFloat("Display Time (seconds) ", duration, 0.01, 10.0, "%.2f")

    psim.Separator()
    
    changed_frame, current_frame = psim.SliderInt("Current Frame", current_frame, 0, nb_frames-1)

    if changed_frame :
        update_points(toNextFrame=False)
        last_update_time = time.time()
    
    if isStopped == False :
        current_time = time.time()
        if current_time - last_update_time >= duration :  # Mise à jour toutes les secondes
            update_points()
            last_update_time = current_time  # Mettre à jour le dernier temps

def generate_all_graphs(grains_each_frame1, grains_each_frame2) :
    if len(grains_each_frame1) != len(grains_each_frame2) :
        raise ValueError("The videos do not have the same number of frames.")
    
    all_coordinates = []
    nb_frames = len(grains_each_frame1)
    for current_frame in range(nb_frames) :
        all_coordinates.append(generate_points_ps(grains_each_frame1[current_frame], grains_each_frame2[current_frame]))
    
    return all_coordinates


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Script de visualisation avec Polyscope")
    parser.add_argument(
        "tracking_dir",
        type=str,
        help="Chemin vers le dossier contenant les fichiers de tracking"
    )
    parser.add_argument(
        "contact_dir",
        type=str,
        help="Chemin vers le dossier contenant les fichiers de contact"
    )

    args = parser.parse_args()
    tracking_dir = args.tracking_dir
    contact_dir = args.contact_dir

    print("Loading Files...")

    tracking_files = get_sorted_files(tracking_dir)
    tracking_files_2 = get_sorted_files(contact_dir)
    grains_each_frame_file1 = []
    grains_each_frame_file2 = []
    
    for i in range(len(tracking_files)):
        file_tracking = tracking_files[i]
        file_tracking_2 = tracking_files_2[i]
        grains_each_frame_file1.append(generate_grains(None, None, file_tracking))
        grains_each_frame_file2.append(generate_grains(None, None, file_tracking_2))

    print("Files loaded !")
    
    ps.init()

    print("Generating all parameters...")
    all_coordinates = generate_all_graphs(grains_each_frame_file1, grains_each_frame_file2)
    print("Parameters generated !")

    # Créer des données initiales
    nb_frames = len(all_coordinates)
    current_frame = 0
    prepare_display_ps(*all_coordinates[current_frame], display=False)
    isStopped = True
    ui_angle_rad = 0.2
    duration = 1.0
    last_update_time = time.time()  # Temps initial

    ps.set_user_callback(user_callback)

    # Afficher la scène
    ps.show()

    ps.clear_user_callback()