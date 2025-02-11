# Affichage des grains dans un espace 3D interactif par l'utilisateur.

Développeur : Léo LEFEBVRE (leo.lefebvre@ecole.ensicaen.fr)

## Contexte

Dans le contexte du projet industrielle "Analyse topologique de mouvements de grains dans une séquence d’images 3D" tutoré par Mme KENMOCHI. J'ai conçu un programme python pour visualer et intérragir avec graphes représentant un ensemble de grains avec différentes informations clés :
- Les labels
- Les coordonnées en XYZ
- Les points de contacts

En utilisant différents fichiers txt avec une certaine structure, les programmes lisent ces fichiers pour afficher avec la bibliothèque Polyscope les grains.

Bibliothèque Polyscope : https://polyscope.run/py/

## Structure

Dans cette partie du projet, vous trouverez différents fichiers clés :
- grain.py : la structure de nos grains et des fonctions qui y sont associés
- polyscope_tools.py : différentes fonctions "outils" utilisés par les différents programmes principaux pour l'affichage Polyscope.
- polyscope_follow_grains.py : le 1er programme principal, il récupère un fichier des labels + coordonées avec les points de contacts pour ainsi afficher tous les grains avec leurs informations.
- polyscope_follow_grains_with_labels : le 2nd programme principal, il récupère un fichier des labels + coordonées avec les points de contacts pour ainsi afficher des grains filtrés par l'utilisateur en amont.
- polyscope_compare_grains : le 3e programme principal, il récupère deux fichiers correspondant à deux graphes. Si un grain du graphe 1 possède les mêmes coordonnées qu’un grain du graphe 2, alors on le catégorise d’une certaines couleurs, si le grain est unique au graphe 1, il possède alors une autre couleur, de même dans le cas où le grain est unique au graphe 2. Cela permet par exemple, de comparer l’évolution des grains entre deux simulations différentes.

**Structure d'un fichier txt des coordonnées**
```
Label	Zpos	Ypos	Xpos
0.0000000	0.0000000	0.0000000	0.0000000
1.0000000	106.1302872	677.3010254	430.3721619
2.0000000	106.4360046	683.3963623	522.8596191
3.0000000	112.3213120	413.4475708	534.6060791
4.0000000	112.1344604	457.2076721	473.1884155
```

**Structure d'un fichier txt des points de contacts**
```
# Grain A - Grain B
1 2
3 4
2 3
```

Pour des tests, vous pouvez télécharger des données réelles de ces fichiers txt via ce site : https://zenodo.org/records/8014905

Vous pouvez aussi aller dans la section "Polyscope" de ce google drive :
https://drive.google.com/drive/folders/1Js6EvltIOVVMZXwInvqEsXcCLXHrGpQo?usp=sharing

## Lancement

**Information**

Le dossier *TrackingFolder* et *ContactPointsFolder* contient les fichiers txt des coordonnées et des labels. Il y a plusieurs fichiers car notre programme peut afficher plusieurs frames des graphes pour montrer une évolution dans le temps.


**Lancer polyscope_follow_grains.py**

```
python polyscope_follow_grains_global.py TrackingFolder ContactPointsFolder
```

**Lancer polyscope_follow_grains_with_labels.py**
```
python polyscope_follow_grains_with_labels.py .TrackingFolder ContactPoints num_label1 num_label2 ... num_labeln
```

num_label correspond à l'entier que vous voulez faire apparaitre sur polyscope. Vous pouvez en faire apparaitre plusieurs en notant les labels à la suite en les séparant d'un espace.

***Attention***: Lorsque Polyscope est lancé sur programme, il est normal que vous ne voyiez rien. Il suffit juste de dézoomer pour voir les différents grains filtrés. Cela est en lien avec un problème de la gestion de la caméra non corrigée.

**Lancer polyscope_compare_grains.py**
```
python polyscope_compare_grains.py TrackingFolder TrackingFolder2
```

## Bug / Améliorations possibles

Améliorations possibles :
- Dans tous les programmes, pouvoir ajouter la possibilité de lire des fichiers CSV.
- Améliorer la stabilité de lancement de polyscope_compare_grains et de mesurer les performances avec deux graphes complets.

Bug : 
- Dans *polyscope_follow_grains_with_labels.py*, les arrêtes entre deux grains existants ne s'affichent pas.
- La bibliothèque Python de Polyscope, conçue pour être accessible, ne permet pas une liberté de conception suffisante pour gérer certains aspects clés de notre cas d’usage. Par exemple, la suppression de grains entre deux frames : il arrive qu’aucun grain connecté ou non connecté n’existe, et il serait alors pertinent de pouvoir les supprimer. Cependant, cette opération n'est pas possible avec la bibliothèque Python. La solution la plus viable trouvée consiste à définir l’attribut Enabled=False pour le graphe. Toutefois, cette approche permet toujours à l'utilisateur d'afficher le graphe via les options de Polyscope. J'ai donc choisi de placer le grain à des coordonnées aberrantes afin qu’il ne soit plus visible. Malheureusement, cette solution entraîne plusieurs instabilités lors de l'affichage, telles qu’un éloignement excessif de la caméra, l’impossibilité de zoomer ou encore des bugs d'affichage du fond.

C'est pourquoi, pour la suite de ce projet, je vous recommande fortement de passer à la bibliothèque C++, qui offre une bien plus grande liberté dans l'utilisation de Polyscope.

Polyscope en C++ : https://polyscope.run/
