PATH_CONFIG="config.xml"
PATH_API_LIST="Temp/API_list.txt"
API_PREFIX="config/API/"
HEADER_PREFIX="${API_PREFIX}Header_files"
SO_PREFIX="${API_PREFIX}Dynamic_libraries"
HEADER_DEST=API/Header_files
SO_DEST=API/Dynamic_libraries

# If temporary files directory does not exist, then go ahead and create it.
if [ ! -d Temp ]; then
    mkdir Temp
fi

# Write all the api header files in a temporary file.
xmlstarlet el -a $PATH_CONFIG | grep $HEADER_PREFIX | grep "@" > $PATH_API_LIST

# Create symlinks of the SO files in their target destination directory.
while read -r line
do
    source=$(xmlstarlet sel -t -v "//${line}" $PATH_CONFIG)
    full_path=$(readlink -f $source)
    echo "Making symbolic link to $full_path."
    ln -sf $full_path $HEADER_DEST
done < $PATH_API_LIST

# Write all the api SO files in a temporary file.
xmlstarlet el -a $PATH_CONFIG | grep $SO_PREFIX | grep "@" > $PATH_API_LIST

# Create symlinks of the SO files in their target destination directory.
while read -r line
do
    source=$(xmlstarlet sel -t -v "//${line}" $PATH_CONFIG)
    full_path=$(readlink -f $source)
    echo "Making symbolic link to $full_path."
    ln -sf $full_path $SO_DEST
done < $PATH_API_LIST

# Delete temporary files directory if it still exists.
if [ -d Temp ]; then
    rm -rf Temp
fi
