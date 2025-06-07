#!/bin/bash

CERT_DIR_PATH="$(dirname $(realpath ${0}))/../certificate_test"
PKEY_PATH="${CERT_DIR_PATH}/private.key"
CERT_PATH="${CERT_DIR_PATH}/certificate.crt"

# Create directory if it doesn't exist
if [ ! -d ${CERT_DIR_PATH} ]
then
    mkdir ${CERT_DIR_PATH}
fi

# Generate private key first
openssl genpkey -algorithm RSA -out ${PKEY_PATH}

# Update permissions just in case
chmod 600 ${PKEY_PATH}

# Generate self-signed certificate
openssl req -new -key ${PKEY_PATH} -x509 -days 365 -out ${CERT_PATH}

# View certificate
openssl x509 -in ${CERT_PATH} -text