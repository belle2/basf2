#! /bin/tcsh

# change to your DESY account name
BELLE2_USER=USERNAME
# change if you want a different directory name for basf2 installation
set basf2_top=basf2_firmware_sim

echo We will use http authentication.
echo You will need to enter your password at least 4 times.
echo If you wish to use ssh and have uploaded you public key to Bitbucket/Stash,
echo modify the variable BELLE2_GIT_ACCESS in the script to ssh

set BELLE2_GIT_ACCESS=http
# set BELLE2_GIT_ACCESS ssh

set CENTRAL_RELEASE=release-01-00-01

############################
# ensure that the authentication in this step succeeds
# git ls-remote https://$BELLE2_USER@stash.desy.de/scm/b2/tools.git master || (echo Please use correct DESY username/passwd && exit)
############################

set BELLE2_TOOLS=/sw/belle2/tools/
set FILENAME=${BELLE2_TOOLS}/setup_belle2
source `dirname ${FILENAME}`/setup_belle2.csh
if ( ! -d $basf2_top )  then
    newrel $basf2_top $CENTRAL_RELEASE || (echo Please use correct DESY username/passwd && exit)
endif

cd $basf2_top
setuprel

addpkg trg
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
