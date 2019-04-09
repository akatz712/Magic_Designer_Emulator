#!/bin/bash
# remove
if [[ $EUID -ne 0 ]]
then
   echo "This script must be run with sudo." 
   exit 1
fi
make uninstall || exit 2
update-desktop-database --quiet
update-mime-database /usr/share/mime
echo "Magic Designer Emulator removed."
