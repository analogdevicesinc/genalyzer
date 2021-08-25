#!/bin/bash
sudo apt-get install -y cmake tar bzip2 gzip build-essential libfftw3-dev python3-sphinx python3-breathe
pip install sphinx_rtd_theme
mkdir -p build
cd build
cmake ..
sudo make -j4
sudo make test
sudo make install
sudo ldconfig
