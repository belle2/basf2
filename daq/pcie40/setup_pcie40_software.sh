#!/bin/sh
CUR_DIR=`pwd`
SCRIPT_DIR=$(cd $(dirname $0); pwd)
if [ -e $SCRIPT_DIR/Pcie40Software ]; then
    echo "$SCRIPT_DIR/Pcie40Software is already there. Exiting..."
    exit
fi

# download PCIe40 software
git clone ssh://git@stash.desy.de:7999/bidu/software.git $SCRIPT_DIR/Pcie40Software

# Set environmental variables
source $SCRIPT_DIR/Pcie40Software/Scripts/setup.sh

# build PCIe40 software
cd $SCRIPT_DIR/Pcie40Software
mkdir -p build
cd build
cmake ../
make

cd $CUR_DIR

echo "Downloading and compiling PCIe40 library were finished."
