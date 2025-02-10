# maxTree

Contains 2 files, maxTree.py and maxTree.ipynb.\
maxTree.py is an executable which apply the maxtree algorithm.\
maxTree.ipynb is a jupyterNotebook which explains the execution of maxTree.py.

 Command example
 --------------------------------------------------------
 `python3 maxTree/maxTree.py image.tif noyau.tif 6`


# minTree

Contains 2 files, minTree.py and minTree.ipynb.\
minTree.py is an executable which apply the mintree algorithm.\
minTree.ipynb is a jupyterNotebook which explains the execution of minTree.py.

 Command example
 --------------------------------------------------------
 `python3 minTree/maxTree.py image.tif 6`


# watershed

Contains 2 files, watershed.py and watershed.ipynb.\
watershed.py is an executable which apply the watershed algorithm.\
watershed.ipynb is a jupyterNotebook which explains the execution of watershed.py.

watershed.py need minTree.py and maxTree.py during its execution.

 Command example
 --------------------------------------------------------
 `python3 watershed/watershed.py image.tif seed.tif 6`


# successiveWatershed

Contains 4 files, successiveWatershed.py, watershed_function.py, example.py and utils.py.\
successiveWatershed.py is an executable which apply successive watershed algorithm.

See the Readme inside the folder for more information.

 Command example
 --------------------------------------------------------
 `python3 successiveWatershed/successiveWatershed.py image.tif 6`


# viewImage

Contains viewImage.py, an executable very useful to see the resulting tif images without application like imageJ or gmic.


# utils

Contains useful executable files for other files.


# results

Contains the results of executions and temporary files.





