PATH_CONFIG="config.xml"
PATH_DEPS_LIST="Temp/deps_list.txt"
DEPS_PREFIX="config/Dependencies/"
HEADER_PREFIX="${DEPS_PREFIX}Header_files"
SO_PREFIX="${DEPS_PREFIX}Dynamic_libraries"
HEADER_DEST=Dependency_files/Header_files
SO_DEST=Dependency_files/Dynamic_libraries

# If temporary files directory does not exist, then go ahead and create it.
if [ ! -d Temp ]; then
    mkdir Temp
fi

# Write all the api header files in a temporary file.
xmlstarlet el -a $PATH_CONFIG | grep $HEADER_PREFIX | grep "@" > $PATH_DEPS_LIST

# Create symlinks of the SO files in their target destination directory.
while read -r line
do
    source=$(xmlstarlet sel -t -v "//${line}" $PATH_CONFIG)
    full_path=$(readlink -f $source)
    echo "Copying $full_path to $HEADER_DEST ..."
    cp $full_path $HEADER_DEST
done < $PATH_DEPS_LIST

# Write all the api SO files in a temporary file.
xmlstarlet el -a $PATH_CONFIG | grep $SO_PREFIX | grep "@" > $PATH_DEPS_LIST

# Create symlinks of the SO files in their target destination directory.
while read -r line
do
    source=$(xmlstarlet sel -t -v "//${line}" $PATH_CONFIG)
    full_path=$(readlink -f $source)
    echo "Copying $full_path to $SO_DEST ..."
    cp $full_path $SO_DEST
done < $PATH_DEPS_LIST

# Delete temporary files directory if it still exists.
if [ -d Temp ]; then
    rm -rf Temp
fi
