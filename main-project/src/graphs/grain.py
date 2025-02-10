class Grain:
    def __init__(self, label: int, x: float, y: float, z: float):
        self.label = label
        self.x = x
        self.y = y
        self.z = z
        self.neighbors = []

    def __eq__(self, other):
        if not isinstance(other, Grain):
            return False

        return self.label == other.label

    def __hash__(self):
         return hash(self.label)
    
    def add_neighbor(self, neighbor: 'Grain'):
        if neighbor not in self.neighbors:
            self.neighbors.append(neighbor)

    def __repr__(self):
        return f"Grain(label={self.label}, x={self.x}, y={self.y}, z={self.z})"
    
def get_grains_from_tracking(file):
    grains_dict = {}
    with open(file, 'r') as f:
        lines = f.readlines()[1:]  # Ignorer la première ligne d'en-tête
        for line in lines:
            values = line.split()  # Séparer les valeurs
            label = int(float(values[0]))  # Label est un entier
            z = float(values[1])  # Zpos
            y = float(values[2])  # Ypos
            x = float(values[3])  # Xpos
            grain = Grain(label, x, y, z)
            grains_dict[label] = grain
    return grains_dict

def get_neighbors_from_contact(grain_dict, file):
    if not file :
        return
    
    with open(file, 'r') as f:
        lines = f.readlines()[1:]  # Ignorer la première ligne d'en-tête
        for line in lines:
            values = line.split()
            label_1 = int(float(values[0]))
            label_2 = int(float(values[1]))
            grain_dict[label_1].add_neighbor(grain_dict[label_2])
            grain_dict[label_2].add_neighbor(grain_dict[label_1])

def get_points_connected_and_not_connected(grains) :
    grains_connected = []
    grains_unconnected = []

    for grain in grains.values() :
        if len(grain.neighbors) > 0 :
            grains_connected.append(grain)
        else :
            grains_unconnected.append(grain)

    return grains_connected, grains_unconnected

def generate_grains(file_contact, file_contact2, file_tracking, display=True) :
    grains = get_grains_from_tracking(file_tracking)

    if file_contact is not None :
        get_neighbors_from_contact(grains, file_contact)

    if file_contact2 is not None :
        get_neighbors_from_contact(grains, file_contact2)

    return grains