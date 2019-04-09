#!/bin/bash
# Build and Install
if [[ $EUID -ne 0 ]]
then
   echo "This script must be run with sudo." 
   exit 1
fi
if [ ! -f "/usr/include/FL/Fl.H" ] 
then
    echo "FLTK development package is not installed." 
    exit 2
fi
if [ ! -f "/usr/include/hpdf.h" ] 
then
    echo "Haru/hpdf development package is not installed." 
    exit 3
fi
./configure --prefix=/usr  || exit 4
make  || exit 5
make install-strip  || exit 6
update-mime-database /usr/share/mime
update-desktop-database --quiet
echo "Magic Designer Emulator installed successfully."
