import polyscope as ps
import numpy as np
import plotly.express as px
import plotly.graph_objects as go
from grain import *
from polyscope_tools import *
import os
import time
import polyscope.imgui as psim
import argparse

prev_edges_co = None

def generate_points_ps(grains) :
    global prev_edges_co

    grains_connected, grains_unconnected = get_points_connected_and_not_connected(grains)

    points_connected = np.array([[grain.x, grain.y, grain.z] for grain in grains_connected])
    points_unconnected = np.array([[grain.x, grain.y, grain.z] for grain in grains_unconnected])
    
    labels_connected = np.array([grain.label for grain in grains_connected])
    nb_neighbors_connected = np.array([len(grain.neighbors) for grain in grains_connected])
    labels_unconnected = np.array([grain.label for grain in grains_unconnected])
    nb_neighbors_unconnected = np.zeros(len(grains_unconnected))
        
    
    ### Register a point cloud
    grain_index = {grain: idx for idx, grain in enumerate(grains_connected)}  # Associe chaque grain à son index
    
    edges_co = []
    
    for grain in grains_connected:
        for neighbor in grain.neighbors:
            edges_co.append((grain_index[grain], grain_index[neighbor]))  # Ajoute une arête entre les indices des grains
    
    edges_co = np.array(edges_co)
    new_cos, old_cos = compare_edge_curves(edges_co, prev_edges_co)
    prev_edges_co = edges_co

    return points_connected, points_unconnected, labels_connected, nb_neighbors_connected, labels_unconnected, nb_neighbors_unconnected, new_cos, old_cos

def prepare_display_ps(points_connected, points_unconnected, labels_connected, nb_neighbors_connected, labels_unconnected, nb_neighbors_unconnected, new_cos, old_cos, display=False) :
    ps_cloud_co = ps.register_point_cloud("Grains connected", points_connected, radius=0.0025, color=(0.0, 0.0, 1.0))
    ps_cloud_unco = ps.register_point_cloud("Grains unconnected", points_unconnected, radius=0.0025, color=(0.0, 1.0, 0.0))

    cloud_nodes_add_quantity(ps_cloud_co, labels_connected, nb_neighbors_connected)
    cloud_nodes_add_quantity(ps_cloud_unco, labels_unconnected, nb_neighbors_unconnected)

    ps.register_curve_network("Links between neighbors", points_connected, old_cos, color=(0.75, 0.0, 0.75), radius=0.00025)
    ps.register_curve_network("New links between neighbors", points_connected, new_cos, color=(0.75, 0.75, 0.75), radius=0.00025)

    if display :
        ps.show()

def update_display_ps(points_connected, points_unconnected, labels_connected, nb_neighbors_connected, labels_unconnected, nb_neighbors_unconnected, new_cos, old_cos, display=False) :
    ps_cloud_co = ps.register_point_cloud("Grains connected", points_connected)
    ps_cloud_unco = ps.register_point_cloud("Grains unconnected", points_unconnected)

    cloud_nodes_add_quantity(ps_cloud_co, labels_connected, nb_neighbors_connected)
    cloud_nodes_add_quantity(ps_cloud_unco, labels_unconnected, nb_neighbors_unconnected)

    ps.register_curve_network("Links between neighbors", points_connected, old_cos)
    ps.register_curve_network("New links between neighbors", points_connected, new_cos)
    
    if display :
        ps.show()

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

def generate_all_graphs(grains_each_frame) :
    all_coordinates = []
    for grains in grains_each_frame :
        all_coordinates.append(generate_points_ps(grains))
    
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
    contact_files = get_sorted_files(contact_dir)
    grains_each_frame = []
    
    for i in range(len(tracking_files)):
        file_tracking = tracking_files[i]
        file_contact = contact_files[i]
        file_contact2 = contact_files[i + 1]
        grains_each_frame.append(generate_grains(file_contact, file_contact2, file_tracking))

    print("Files loaded !")
    
    ps.init()

    print("Generating all parameters...")
    all_coordinates = generate_all_graphs(grains_each_frame)
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