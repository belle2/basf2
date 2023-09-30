#! /bin/tcsh

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# change if you want a different directory name for basf2 installation
set basf2_top=basf2_firmware_sim

echo We will use http authentication.
echo You will need to enter your password at least 4 times.
echo If you wish to use ssh and have uploaded your public key to GitLab,
echo modify the variable BELLE2_GIT_ACCESS in the script to ssh

set BELLE2_GIT_ACCESS=http
# set BELLE2_GIT_ACCESS ssh

############################
# ensure that the authentication in this step succeeds
git ls-remote https://gitlab.desy.de/belle2/software/tools.git main || (echo Please use correct DESY username/passwd && exit)
############################

source ${BELLE2_TOOLS:-/cvmfs/belle.cern.ch/tools}/b2setup.csh
if ( ! -d $basf2_top )  then
    b2code-create $basf2_top || (echo Please use correct DESY username/passwd && exit)
endif

cd $basf2_top
b2setup

b2code-package-add trg
########################
#  start from here if you have already installed basf2
git fetch
git checkout feature/trg-cdc-firmware-cosimulation
scons trg/cdc

# set xsim runtime library path
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:/home/belle2/tasheng/Vivado_2017.2/lib/lnx64.o
# link to 2D snapshot
if ( ! -d 'xsim.dir' ) then
    ln -s /home/belle2/tasheng/tsim/xsim.dir
endif

echo all done. Now please run TSF firmware simulation with
echo basf2 trg/cdc/examples/firmware_tsf.py
