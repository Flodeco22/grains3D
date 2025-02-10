# Détection des contacts entre grains

Développeur : Hugo Lemoisson (hugo.lemoisson@ecole.ensicaen.fr)

## Contexte

Dans le contexte du projet industrielle "Analyse topologique de mouvements de grains dans une séquence d’images 3D" tutoré par Mme KENMOCHI. J'ai conçu plusieurs programmes permettant de détecter les contacts entre grains et la force de ces contacts, à partir d'une labellisation complète ou partielle des grains.

Les résultats sont enregistrés dans des fichiers csv.

## Structure

Dans cette partie du projet, vous trouverez différents dossiers clés :
- utils : contient des scripts python utiles aux programmes principaux
- results : dossier où les résultats sont enregistrés par défaut
- contact_detection : contient les programmes principaux de détection des contacts
- Pink : bibliothèque de traitement d'images utilisée par certains des programmes principaux (https://perso.esiee.fr/~coupriem/Pink/doc/html/)

**Structure des fichiers csv créés**
```
Label1,	Label2,	ContactStrength
13,      24,      3
13,      27,      1
```

## Lancement

**Information**

Avant de pouvoir utiliser *contact_detection_from_label_and_skeleton.py* et *contact_detection_by_extending_labels.py*, il faut compiler la bibliothèque Pink.
Si vous avez déjà une instance compilée de la bibliothèque sur votre appareil,
vous pourrez spécifier le chemin jusqu'à cette instance plutôt que de compiler celle présente ici.

Pour compiler la bibliothèque Pink, se placer dans le dossier *Pink/* puis exécuter la commande :
```
./makelin
```



**Lancer contact_detection_from_label_naive.py**

```
cd contact_detection
python3 contact_detection_from_label_naive.py labels.tif --output=contacts.csv
```

Le programme prend en argument :
- labels.tif : l'image tif des grains labellisés.
- --output(optionnel) : désigne le chemin vers le fichier de sortie. Par défaut,
*../results/contacts_naive.csv*

**Lancer contact_detection_from_label_and_skeleton.py**

```
cd contact_detection
python3 contact_detection_from_label_and_skeleton.py grains.tif labels.tif x y z --keep_files --pink_dir=~/Pink/linux/bin/ --output=contacts.csv
```

Le programme prend en argument :
- grains.tif : l'image tif originale des grains.
- labels.tif : l'image tif des grains labellisés.
- x y z : Tailles des images selon les axes x,y,z
- --keep_files(optionnel) : si cet argument est donné, le dossier contact_detection/tmp/ contenant tous les fichiers intermédiaires créés n'est pas supprimé.
- --pink_dir(optionnel) : désigne le chemin vers le dossier des fichiers binaires de la librairie Pink, par défaut *../Pink/linux/bin/*
- --output(optionnel) : désigne le chemin vers le fichier de sortie. Par défaut,
*../results/contacts_using_skeleton.csv*

**Lancer contact_detection_by_extending_labels.py**

```
cd contact_detection
python3 contact_detection_by_extending_labels.py grains.tif x y z --keep_files --pink_dir=~/Pink/linux/bin/ --output=contacts.csv
```

Le programme prend en argument :
- grains.tif : l'image tif originale des grains.
- x y z : Tailles des images selon les axes x,y,z
- --threshold(optionnel) : valeur du seuil utilisé pour binariser l'image des grains. Par défaut, 27000
- --keep_files(optionnel) : si cet argument est donné, le dossier contact_detection/tmp/ contenant tous les fichiers intermédiaires créés n'est pas supprimé.
- --pink_dir(optionnel) : désigne le chemin vers le dossier des fichiers binaires de la librairie Pink, par défaut *../Pink/linux/bin/*
- --output(optionnel) : désigne le chemin vers le fichier de sortie. Par défaut,
*../results/contacts_extending_labels.csv*

## Amélioration possible

Amélioration possible :
- Utiliser la bibliothèque Pink pour l'érosion, plutôt qu'une fonction python. Le temps d'éxécution serait meilleur.
- Parallélisation de la diffusion des labels dans *contact_detection_by_extending_labels.py*

