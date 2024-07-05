# C_Server_Socket
### A server socket written in C language
The aim of this project is to build and learn about internet sockets, so it has been created for both educational and practical purposes.

## Table of contents
* [Introduction](#introduction)
* [Features](#features)
* [Prerequisites](#prerequisites)
* [Installation instructions](#installation-instructions)
* [Usage](#usage)
* [To do](#to-do)
* [Contributing](#contributing)
* [License](#license)

## Introduction
This library was started as a way for me to have a more practical approach to how TCP sockets work. I felt a bit scared in the beginning since I was overwhelmed by the many concepts I had to get a deeper understanding of.
However, as I found out the vast amount of practical applications it may have in real life, I started regarding it as a way more amusing and enriching activity for my professional career.

Building this project has involved a deep understanding of:
* Parallel-running processes
* TCP/IP protocol stack
* Dependencies
* Tests
* Linux OS operation
  
and the list goes on and on.
Despite existing some aspects that need to be improved, it's a project I feel quite proud of. More of those (such as UDP support) will be coming soon.
By now, it has been designed for it to be run on Linux distros (such as Ubuntu). The resulting library is a *.so* file alongside a C language header (*.h*) file.

## Features
The resulting library comes with many options that can be applied to it, such as setting timeouts, or adding a custom interaction function,
it's to say, an arbitrary function to describe how the socket should interact once the client has connected to it.

When it comes to security, TLS can be set, as well as the paths to the private key and server certificate, which should be created by the user itself
(these files are not provided with the current project for privacy concerns).

In order to get some knowledge about how to use the library alongside its options, go to [Usage](#usage).

## Prerequisites
By now, the application has only been tested in POSIX compliant Linux distros. In these, many of the dependencies dependencies below may already come installed in the SO.
In the following list, the minimum versions required (if any) by the library are listed.

* [gcc](https://gcc.gnu.org/) 11.4 or above
* [Bash](https://www.gnu.org/software/bash/) 4.4 or above
* [Make](https://www.gnu.org/software/make/) 4.1 or above
* [Git](https://git-scm.com/)
* [Xmlstarlet](https://xmlstar.sourceforge.net/)
* [OpenSSL](https://www.openssl.org/) 3.0.2 or above

Except for Make, Bash and OpenSSL, the latest version of each of the remaining dependencies will be installed automatically if they have not been found beforehand. 

On top of the ones listed above, there are some dependencies that are required for both the library and the test executable to be 

## Installation instructions
In order to install it, just clone it from GitHub to your choice path by using the [link](https://github.com/JonMS95/C_Server_Socket) to the project.

```bash
cd /path/to/repos
git clone https://github.com/JonMS95/C_Server_Socket
```

Then navigate to the directory in which the repo has been downloaded, and set execution permissions to every file just in case they have not been sent beforehand.

```bash
cd /path/to/repos/C_Server_Socket

find . -type f -exec chmod u+x {} +
```

For the library to be built (i.e., clean, download dependencies and compile), just type the following on your command:

```bash
make
```

The result of the line above will be a new API directory (which will match the used version). Within it, a *.h* and a *.so* file will be found.
- **/path/to/repos/C_Server_Socket/API**
  - **vM_m**
    - **Dynamic_libraries**
      - **libServerSocket.so.M.m**
    - **Header_files**
      - **ServerSocket_api.h**

Where _M_ and _m_ stand for the major and minor version numbers.

For the test executable file to be compiled and executed, use:

```bash
make test
```

Again, the one below is the path to the generated executable file:
- **/path/to/repos/C_Server_Socket/Tests**
  - Source_files
  - Dependency_files
  - **Executable_files**
      - **main**

## Usage

## To do

## Contributing

## License
