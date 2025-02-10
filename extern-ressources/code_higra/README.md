Hi there !

Here are all of the work I did during my M2 internship with Yukiko.
I tried to make everything as clean as possible, but my organisation skills are still limited ahah.

I divided everything into four main folders :
- display
- preprocessing
- segmentation
- tools

**Display** will contain a few scripts for visualization. I've also added a small program for tiff <-> 3D view, which is useless because GMIC is better but is a funny toy to play with (can also be usefull if you want to generate clean 3D illustrations).

**Preprocessing** contains quite a lot of stuff related to.. well.. image preprocessing. Whenever you need to crop, subsample, slice, blur or anything, there's a chance I already coded it, and it would be stored here.

**Segmentation** will store our main algorithms, described in my *Rapport de Stage*. 

Finally, the **tools** folder contains the last scripts I didn't really know where to put. The names of the scripts are explicit.

All of the scripts I wrote are Python code. For all of them, their execution command follow the same global scheme :

```sh
python3 my_program.py [-h] file_1 ...
```

Without digging much in the code, you can use the ``-h`` option to know how to use a specific script, each one has a detailed man page.\
However if you actually *want* to dig in the matrix, you can refer to headers and comments, which sould help you understand what's going on.

Also, please note that I moved some files that once were together at the execution time. You may need to copy some files from the tools folder for some script to run properly (for most of them however, there will be no problem).

In any case, don't hesitate to contact me at lmacke@unistra.fr, I'll be happy to (try to) help.

Have fun !

Lysandre
