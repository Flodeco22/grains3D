#!/bin/bash
# Make sure that you have the G'MIC libary installed !
# See https://gmic.eu/index.html


RED='\033[0;31m'
PURPLE='\033[0;35m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

shopt -s globstar dotglob

# checks
[ $# -ne 1 ] &&
printf "${RED}${BOLD}Error${NORMAL}${NC} : wrong number of argument.\n\n${BOLD}Usage${NORMAL} : ./crop_dataset.sh dataset_folder\n" &&
exit 1

[[ $1 != */ ]] && 
printf "${RED}${BOLD}Error${NORMAL}${NC} : '$1' is not a directory. (did you mean '$1/' ?)\n" &&
exit 1

[ ! -d $1 ] &&
printf "${RED}${BOLD}Error${NORMAL}${NC} : directory '$1' does not exist.\n" &&
exit 1

# [ ! -f $1**/*.tif ] &&
# printf "${PURPLE}${BOLD}Warning${NORMAL}${NC} : directory '$1' does not contain any tif file.\n"

# create output directory
ORIG=$1
DIR_NAME=${ORIG//"../"}
DIR_NAME=${DIR_NAME%/}
DIR_NAME=${DIR_NAME##*/}
OUT="${DIR_NAME}_cropped"
rm -rf ${OUT} # clean an already existing output directory
mkdir ${OUT}  # create a new one

# process files
for FILENAME in $1**/*; do
	FILE=${FILENAME##*/} # get only the 'real' file name
	
	[[ ${FILE} == *.tif ]] &&
	printf "${BOLD}\055-Cropping ${FILE}... \n${NORMAL}" &&
	gmic ${FILENAME} crop 350,350,1050,1050 keep[500-1100] a z o ${OUT}/${FILE}
done

# Display generated output with size.
OUTSIZE=$([ "$(ls -A ${OUT})" ] &&
du -hs ${OUT} | cut -d $'\t' -f1 ||
echo "empty")

printf "${GREEN}${BOLD}\055- Created directory ${OUT} (${OUTSIZE}) with cropped files successfully.\n${NORMAL}${NC}"