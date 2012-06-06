#!/bin/bash
# basf2helper - performs top and tail work to allow basf2 to function well 
#               in a grid environment
# $1 = steering file
# $2 = release
# Tom Fifield - 2010-01
# Yanliang Han -2011-11
##############################################################################
# Initialisation
##############################################################################
VO_BELLE_SW_DIR=${VO_BELLE_SW_DIR:-$OSG_APP/belle}
unset VO_BELLE2_SW_DIR

#avoid basf2 tools check
export BELLE2_NO_TOOLS_CHECK=1

#it used an old copy, (with this $2) so i commented it out, and set it 
#manually:
#. ${VO_BELLE_SW_DIR}/belle2/tools/setup_belle2.sh
#WORKDIR=$PWD
#cd ${VO_BELLE_SW_DIR}/belle2/releases/$2
#setuprel $2
#cd $WORKDIR

. ${VO_BELLE_SW_DIR}/belle2/tools/setup_belle2.sh
WORKDIR=$PWD
cd ${VO_BELLE_SW_DIR}/belle2/releases/build-2012-05-02
setuprel build-2012-05-02
cd $WORKDIR


basf2 -i > basf2.error 2>&1
if [ $? -ne 0 ]; then
  cat basf2.error
  echo "basf2 not installed correctly"
  exit 1
fi

##############################################################################
# Runtime
##############################################################################
basf2 $1


##############################################################################
# Work with output files, metadata
##############################################################################
if [ $? -eq 0 ] 
  then
    ./gbasf2output.py -s $1
  else
    cat basf2.error
    echo "basf2 execution occur error"
fi
