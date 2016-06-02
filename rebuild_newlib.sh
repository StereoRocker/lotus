#!/bin/bash

# We have to recompile everything as the Makefile doesn't seem to register changes to files

cd cross/
rm -rf build-newlib
mkdir build-newlib
cd build-newlib
../newlib-2.2.0-1/configure --prefix=/usr --target=i386-lotus
make clean
make all -j6
make DESTDIR=${HOME}/osdev/sysroot install
cd ../..
