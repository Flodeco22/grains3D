This folder executes the successive watershed algorithm.

- successiveWatershed.py is an executable.
- watershed_function.py is the same algorithm as watershed.py in the watershed folder but it's a python function. We can import it and use it. For more, see watershed_function.py and example.py.
- example.py is an example to use watershed_function.
- utils.py is a file to import, it contains 3 funtions for successiveWatershed, "cropAroundPoint", "loadCentroidsFromCSV" and "positioning_label"

successiveWatershed also need colormap.py and getCentroid.py in the folder "utils".


To use successiveWatershed.py, you have to be in the "src" folder.
All the files generated are put in the "results" folder.

# ========================================================
# Command example
# ========================================================
# python3 successiveWatershed/successiveWatershed.py image.tif 6