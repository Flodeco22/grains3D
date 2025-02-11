# Context

This folder contains utility functions meant to help dealing with non-grain parts of the 3D images used for this project. They are not meant to create a final product by themselves.

# remove_cylinder

### in:
 * **"file"**: tif image to remove cylinders from. Should be a full 3D image.
 *Note: in Lysandre's repository, the full images we used are named "EFRGPXX_YY.tif", where XX is the number of the video and YY is the number of the frame (so, a 3D image) in video XX.*

### out:
 * **"file_cylinders_removed"**: original tif image with black bars instead of the cylinders. May give an undetermined result if there are no cylinders in the original image.


Execute from /src/ folder. Command examples:
---
`python3 ./removal_utils/remove_cylinder.py "image.tif"`

`python3 ./removal_utils/remove_cylinder.py "EFRGP01_01.tif"`

# apply_cylinder_centroid_label

### in:
 * **"file"**: original tif image with no modifications. Should be a 3D image.
 * **"centroids"**: csv file containing centroid data of **file**. Use `src/segmentation/utils/getCentroids.py` to generate this CSV from **file**.
 * **"seed"**: tif image of a minTree applied on **file**. Use `src/segmentation/minTree/minTree.py` to generate this TIF from **file**.

### out:
 * **"file_with_cylinder_labels"**: csv file containing extra labels for locating cylinders. Upper cylinder has label **-1** and lower cylinder has label **-2**. 

Execute from /src/ folder. Command example:
---
`python3 ./removal_utils/apply_cylinder_centroid_label.py "image.tif" "centroids.csv" "image_minTree_segment_raw.tif"`

# rotateXaxis

Example program to view images from the same angle as the output from **remove_cylinder**. No need to apply this before **remove_cylinder**.

### in:
 * **"file"**: tif image to rotate. Should be a 3D image.

### out:
 * **"file_rotated"**: tif image rotated 90° around its X axis.

Execute from any folder. Command example:
---
`python3 ./[current_directory_to_/removal_utils]/rotateXaxis.py "image.tif"`

# print_progress_bar

Utility function used in **remove_cylinder.py**. Pointless to execute alone.