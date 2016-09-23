#!/bin/bash

sudo cp lib/* /usr/local/lib/
sudo cp include/* /usr/local/include/
cd /usr/local/lib
sudo ln -s libfftw3f.so.3.3.5 libfftw3f.so
sudo ln -s libfftw3f.so.3.3.5 libfftw3f.so.3
ldconfig

git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build

git clone https://github.com/matrix-io/matrix-creator-hal.git
cd matrix-creator-hal
mkdir build
cd ./build
cmake ..
make && sudo make install

