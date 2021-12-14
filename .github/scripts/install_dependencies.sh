#!/bin/bash
sudo apt-get install -y cmake tar bzip2 gzip build-essential libfftw3-dev python3-pip doxygen
sudo pip3 install "sphinx>=2.0" sphinx_rtd_theme "breathe>=4.13.0" pytest pytest-cov