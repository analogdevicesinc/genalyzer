#!/bin/bash
mkdir -p build
cd build
cmake ..
make -j4
sudo make install
sudo ldconfig
cd ..
