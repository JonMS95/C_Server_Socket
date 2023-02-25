#!/bin/sh

if [ ! -d Object_files ]; then
    mkdir Object_files
fi

if [ ! -d Executable_files ]; then
    mkdir Executable_files
fi

if [ ! -d Dependency_files ]; then
    mkdir Dependency_files
fi

if [ ! -d Dependency_files/Header_files ]; then
    mkdir Dependency_files/Header_files
fi

if [ ! -d Dependency_files/Dynamic_libraries ]; then
    mkdir Dependency_files/Dynamic_libraries
fi