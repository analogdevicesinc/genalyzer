#!/bin/bash
mkdir -p ../../build
cd ../../build
cmake ..
sudo make -j4
sudo make test
sudo make install
sudo ldconfig
