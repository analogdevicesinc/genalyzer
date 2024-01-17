#!/bin/bash
apt-get update
apt-get install -y sudo
sudo apt-get update
echo 'debconf debconf/frontend select Noninteractive' | sudo debconf-set-selections
sudo apt-get install -q -y cmake tar bzip2 gzip build-essential libfftw3-dev python3-pip doxygen graphviz