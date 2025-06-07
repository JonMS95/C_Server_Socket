#!/bin/bash

DEFAULT_USER_PORT=55555
DEFAULT_CONN_NUM=2

CONFIG_FILE="config.xml"

PATH_TO_THIS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PATH_TO_LIB_ROOT="$(dirname ${PATH_TO_THIS})"
PATH_TO_TEST_DEPS="$( xmlstarlet sel -t -v "config/test/deps/@Dest" ${CONFIG_FILE})"
PATH_TO_TEST_DEP_DYN_LIBS=${PATH_TO_LIB_ROOT}/${PATH_TO_TEST_DEPS}/lib

CERTIFICATE_TEST_DIR="$(realpath $(dirname $(realpath ${0}))/../certificate_test)"
CERTIFICATE_PATH="${CERTIFICATE_TEST_DIR}/certificate.crt"
PKEY_PATH="${CERTIFICATE_TEST_DIR}/private.key"
CERT_TEST_SCRIPT_PATH="$(realpath $(dirname $(realpath ${0}))/create_self_signed_cert.sh)"

export LD_LIBRARY_PATH=${PATH_TO_TEST_DEP_DYN_LIBS}

# Erase the directory and its content if it exists, then create both the certificate and the private key again.
if [ ! -d ${CERTIFICATE_TEST_DIR} ] || [ ! -f ${CERTIFICATE_PATH} ] || [ ! -f ${PKEY_PATH} ]
then
    rm -rf ${CERTIFICATE_TEST_DIR}
    $(${CERT_TEST_SCRIPT_PATH})
fi

echo
echo "*******************************"
echo "Testing 'main' executable file."
echo "*******************************"
./test/exe/main -r ${DEFAULT_USER_PORT} -m ${DEFAULT_CONN_NUM} -t 1000 -u 0 -y 1000 -i 0 -pnabs -c ${CERTIFICATE_PATH} -k ${PKEY_PATH}
