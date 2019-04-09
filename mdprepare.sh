#!/bin/bash
# Set up complete version
aclocal
autoconf
automake --add-missing
./configure --prefix=/usr
make distcheck
