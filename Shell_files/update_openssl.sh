#!/bin/bash

# Update package lists
sudo apt update

# Install prerequisites
sudo apt install -y build-essential checkinstall zlib1g-dev

# Download OpenSSL 3.0.2 source code
wget https://www.openssl.org/source/openssl-3.0.2.tar.gz

# Extract the tarball
tar -xzvf openssl-3.0.2.tar.gz
cd openssl-3.0.2

# Configure and compile OpenSSL
./config --prefix=/usr --openssldir=/usr/lib/ssl
make

# Install OpenSSL
sudo make install

# Move libraries to /usr/lib/x86_64-linux-gnu
sudo cp *.so* /usr/lib/x86_64-linux-gnu/

# Move header files to /usr/include/openssl
sudo rm -rf /usr/include/openssl
sudo mkdir -p /usr/include/openssl
sudo cp -r include/openssl/* /usr/include/openssl/

# Update shared library cache
sudo ldconfig

# Update the system path to prioritize the new OpenSSL version
echo 'export PATH=/usr/bin:$PATH' >> ~/.bashrc
source ~/.bashrc

# Verify the installation
openssl version
