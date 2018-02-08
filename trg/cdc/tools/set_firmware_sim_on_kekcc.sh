#! /bin/bash

# change to your DESY account name
BELLE2_USER=USERNAME
# change if you want a different directory name for basf2 installation
basf2_top=basf2_firmware_sim

echo We will use http authentication.
echo You will need to enter your password at least 4 times.
echo If you wish to use ssh and have uploaded you public key to Bitbucket/Stash,
echo modify the variable BELLE2_GIT_ACCESS in the script to ssh
BELLE2_GIT_ACCESS=http
# BELLE2_GIT_ACCESS=ssh

CENTRAL_RELEASE=release-01-00-01

############################
# ensure that the authentication in this step succeeds
# git ls-remote https://$BELLE2_USER@stash.desy.de/scm/b2/tools.git master || (echo Please use correct DESY username/passwd && return)
############################

source /sw/belle2/tools/setup_belle2
if [ ! -d $basf2_top ]; then
    newrel $basf2_top $CENTRAL_RELEASE || (echo Please use correct DESY username/passwd && return)
fi

cd $basf2_top
setuprel

addpkg trg
########################
#  start from here if you have already installed basf2
git fetch
git checkout feature/trg-cdc-firmware-cosimulation
scons trg/cdc

# set xsim runtime library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/belle2/tasheng/Vivado_2017.2/lib/lnx64.o
# link to 2D snapshot
if [ ! -d 'xsim.dir' ]; then
    ln -s /home/belle2/tasheng/tsim/xsim.dir
fi
