#!/bin/bash

# execute in distance folder and copies all files recursively into local directory.

GREEN='\033[0;32m'
NC='\033[0m' # No Color
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

cp -r * /home/macke241/Documents/code/
printf "${GREEN}${BOLD}\055- Copied files to local directory.\n${NORMAL}${NC}"