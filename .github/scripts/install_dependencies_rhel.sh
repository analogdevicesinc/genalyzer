#!/bin/bash
# yum group install -y "Development Tools"
# yum install -y gcc gcc-c++ git wget openssl-devel
# yum install -y epel-release
# yum install -q -y cmake tar bzip2 gzip gcc gcc-c++ make fftw-devel python3-pip doxygen graphviz
yum install -q -y fftw-devel python3-pip
yum install -y centos-release-scl
yum install -y devtoolset-7-gcc-c++ --enablerepo='centos-sclo-rh'
scl enable devtoolset-7 'bash'
gcc --version

source scl_source enable devtoolset-7
gcc --version

pip3 install -r requirements_test.txt
pip3 install -r requirements_doc.txt