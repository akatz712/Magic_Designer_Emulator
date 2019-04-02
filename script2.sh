#!/bin/bash
# Build and Install
./configure --prefix=/usr
make
sudo make install
sudo update-desktop-database
sudo update-mime-database /usr/share/mime
