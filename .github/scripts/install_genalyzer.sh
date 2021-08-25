#!/bin/bash
sudo apt-get install -y cmake tar bzip2 gzip build-essential libfftw3-dev
mkdir -p build
cd build
cmake ..
sudo make -j4
sudo make test
sudo make install
sudo ldconfig
