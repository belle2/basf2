#!/bin/bash
# basf2helper - performs top and tail work to allow basf2 to function well 
#               in a grid environment
# $1 = steering file
# $2 = release
# Tom Fifield - 2010-01

##############################################################################
# Initialisation
##############################################################################
VO_BELLE_SW_DIR=${VO_BELLE_SW_DIR:-$OSG_APP/belle}
VO_BELLE2_SW_DIR=$VO_BELLE_SW_DIR/belle2
BELLE2_TOOLS=$VO_BELLE2_SW_DIR/tools
WORKDIR=`pwd`

. $BELLE2_TOOLS/setup_belle2.sh

cd $VO_BELLE2_SW_DIR/releases/$2
setuprel
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
./gbasf2output.py -s $1
