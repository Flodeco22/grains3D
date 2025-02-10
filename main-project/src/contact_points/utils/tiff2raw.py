import argparse
from tifffile import imread

parser = argparse.ArgumentParser(
                    prog = "tiff_to_raw",
                    description = "Converts a tiff file to a raw file")

parser.add_argument("input", help = "The path to a tiff file")

args = parser.parse_args()
outputPath = args.input[:-4] + ".raw"


data = imread(args.input)
data = data.astype("int16")
shape = data.shape
datatype = data.dtype
endianness = "LittleEndian" if data.dtype.byteorder in ('<', '=') else "BigEndian"

header_size = 0

data.tofile(outputPath)