#!/bin/bash

DIR_KEYWORD="Directories/"
PATH_CONFIG="config.xml"
PATH_DIR_LIST="Temp/directory_list.txt"
DIR_PREFIX="config/Directories/"

if [ ! -d Temp ]; then
    mkdir Temp
fi

xmlstarlet el $PATH_CONFIG | grep $DIR_KEYWORD >> $PATH_DIR_LIST

echo "********************"
echo "Creating directories"
echo "********************"

while read -r line
do
    new_dir=${line/#$DIR_PREFIX}
    if [ ! -d $new_dir ]
    then
        echo "Creating $new_dir directory ..."
        mkdir $new_dir
    else
        echo "$new_dir directory already exists"
    fi
done < $PATH_DIR_LIST

if [ -d Temp ]; then
    rm -rf Temp
fi
