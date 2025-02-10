# tiff2obj

A basic python script to convert a multipage tiff file (e.g. 3d scan) into a 3D obj file.

## Install

Get the source code your favorite way, for example by cloning the repo :
```sh
git clone https://github.com/Lys-san/tiff2obj.git
```

## Use

### Requirements
This program is coded using [Python3](https://www.python.org/downloads/) and uses the following libraries :
- [numpy](https://numpy.org/install/)
- [tifffile](https://pypi.org/project/tifffile/)
- [PyVista](https://docs.pyvista.org/version/stable/)

All those can be easily installed with `pip`.

### Command lines

tiff2obj has the following synopsis :
```sh
python3 tiff2obj.py [-h] [-o OUTPUT] file
```

Where file is the input tiff tile to convert.\
Please refer to the man page with the `-h` option for more details.

## Note

- Conversion time may vary depending of the size of the input.
- Output model accuracy is not 100% granted
- Any contribution would be appreciated if you think you can optimize the program ! 
- For any more question, feel free to contact me at lysandre.macke@unicaen.fr