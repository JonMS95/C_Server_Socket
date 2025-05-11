#!/bin/bash

# Generate private key first
openssl genpkey -algorithm RSA -out private.key

# Update permissions just in case
chmod 600 private.key

# Generate self-signed certificate
openssl req -new -key private.key -x509 -days 365 -out certificate.crt

# View certificate
openssl x509 -in certificate.crt -text