#!/bin/sh

DEFAULT_USER_PORT=50000
DEFAULT_CONN_NUM=1

export LD_LIBRARY_PATH=~/Desktop/scripts/C/C_Socket/Dependency_files/Dynamic_libraries

echo
echo "*******************************"
echo "Testing 'main' executable file."
echo "*******************************"
./Executable_files/main ${DEFAULT_USER_PORT} ${DEFAULT_CONN_NUM}