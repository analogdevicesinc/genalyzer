#!/bin/bash
mkdir -p build
cd build
cmake .. -DPYTHON_BINDINGS=ON
make -j4
sudo make install
sudo ldconfig
cd ..
