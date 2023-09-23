#!/bin/bash

MSG_USAGE="Usage: $0 [-a] [-b arg] [-h]"
MSG_p="Location of the node in the config.xml file where the paths to source header files are stored."
MSG_P="Destination directory within the project where symbolic links to header files mentioned in the config.xml files are meant to be created."
MSG_s="Location of the node in the config.xml file where the paths to source SO files are stored."
MSG_S="Destination directory within the project where symbolic links to SO mentioned in the config.xml files are meant to be created."

PATH_CONFIG="config.xml"
PATH_DEPS_LIST="Temp/temp.txt"

# HEADER_PREFIX=
# SO_PREFIX=
# HEADER_DEST=
# SO_DEST=

OPTS_SHORT="p:P:s:S:h"
OPTS_LONG="header_prefix:,header_dest:,so_prefix:,so_dest:,help"

OPTS=$(getopt --options $OPTS_SHORT --longoptions $OPTS_LONG -- "$@")

eval set -- "$OPTS"

while :
do
    case "$1" in
        -p | --header_prefix)
            HEADER_PREFIX="$2"
            shift 2
            ;;

        -P | --header_dest)
            HEADER_DEST="$2"
            shift 2
            ;;

        -s | --so_prefix)
            SO_PREFIX="$2"
            shift 2
            ;;

        -S | --so_dest)
            SO_DEST="$2"
            shift 2
            ;;

        -h | --help)
            echo "Usage: $0 [-p arg] [-P arg] [-s arg] [-S arg]"
            echo -e "-p --header_prefix\t${MSG_p}"
            echo -e "-P --header_dest\t${MSG_P}"
            echo -e "-s --so_prefix\t\t${MSG_s}"
            echo -e "-S --so_dest\t\t${MSG_S}"
            exit 0
            ;;

        --) shift; 
            break 
            ;;
        
        *)
            echo "Unexpected option: $1"
            exit 1
            ;;
    esac
done

list=("HEADER_PREFIX" "HEADER_DEST" "SO_PREFIX" "SO_DEST")

for var in "${list[@]}"
do
    if [ -z "${!var}" ]
    then
        echo "$var is NULL"
        exit 1
    else
        echo "$var = ${!var}"
    fi
done

# $1: $PATH_CONFIG
# $2: $FILE_PREFIX
# $3: $ORG_LOCATIONS_LIST
# $4: $SYM_LINK_DEST
CreateSymLinks()
{
    # Write all the api header files in a temporary file.
    xmlstarlet el -a $1 | grep $2 | grep "@" > $3

    # Create symlinks of the header files in their target destination directory.
    while read -r line
    do
        local source=$(xmlstarlet sel -t -v "//${line}" $1)
        local full_path=$(readlink -f $source)
        echo "Making symbolic link from $full_path to $4."
        ln -sf $full_path $4
    done < $3
}

# If temporary files directory does not exist, then go ahead and create it.
if [ ! -d Temp ]; then
    mkdir Temp
fi

echo "***********************"
echo "Creating symbolic links"
echo "***********************"
CreateSymLinks $PATH_CONFIG $HEADER_PREFIX $PATH_DEPS_LIST $HEADER_DEST
CreateSymLinks $PATH_CONFIG $SO_PREFIX $PATH_DEPS_LIST $SO_DEST

# Delete temporary files directory if it still exists.
if [ -d Temp ]; then
    rm -rf Temp
fi

# HEADER_PREFIX="config/Dependencies/Header_files"
# SO_PREFIX="config/Dependencies/Dynamic_libraries"
# HEADER_DEST=Dependency_files/Header_files
# SO_DEST=Dependency_files/Dynamic_libraries
