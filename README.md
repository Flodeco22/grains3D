# Projet industriel : Analyse topologique de mouvements de grains dans une séquence d’images 3D

Élèves :
- LEFEBVRE Léo
- LEMOISSON Hugo
- OLIVÉ Elliot
- PALISSE Jonathan

Tuteurs :
- KENMOCHI Yukiko
- MENDES-FORTE Julien

# Contexte 

L’objectif de ce travail est d’analyser les changements topologiques intervenant lors du mouvement des grains dans une séquence d’images 3D. En quantifiant ces variations et en proposant de nouvelles méthodes d’identification, nous visons à améliorer la compréhension des dynamiques granulaires et à fournir des outils plus performants pour l’étude des matériaux déformables. Cette séquence d’images consiste, dans nos futurs exemples, de vidéos d’une cinquantaine d’images 3D.


Cela peut se découper en 4 grandes tâches qui ont été effectuées :
- Application des méthodes de segmentation
- Suppression des parties indésirables
- Recherche et quantification des points de contacts
- Affichage dans un graphe 3D intéractif

# Arborescence projet

Le projet se découpe de cette manière :
```
├── extern-ressources
│   ├── code_higra #Code de Lysandre MACKE (demander ressource à Mme KENMOCHI)
│   └── PhdUpload #Ressources du géomécanicien Gustavo PINZON FORERO (https://zenodo.org/records/8014905)
├── main-project # Projet principal
│   ├── ressources #Ressources liées directement au projet
│   └── src # Codes et programmes construits par le groupe de projet
│       ├── graphs #Programmes et outils pour l'affichage sur Polyscope
│       └── segmentation # Programmes et outils pour la segmentation
│       └── contact_points # Programmes et outils pour la détection des contacts
│       └── removal_utils # Programmes et outils pour la suppression des parties indésirables
```

Pour plus de détails dans le dossier **src**, vous trouverez un README pour chacun des sous-dossiers.

# Tests

Pour exécuter les différents sous-programmes, vous trouverez ci-dessous un lien google drive avec des données de tests :
- https://drive.google.com/drive/folders/1Js6EvltIOVVMZXwInvqEsXcCLXHrGpQo?usp=sharing


