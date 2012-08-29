#!/bin/bash
#Add gbasf2 path to the environment PATH
GBASF2ROOT=`pwd`/..
GBASF2LIB=${GBASF2ROOT}/lib
GBASF2BIN=${GBASF2ROOT}/bin

echo GBASF2ROOT=${GBASF2ROOT} >setup.sh
echo GBASF2LIB=${GBASF2LIB}   >>setup.sh
echo GBASF2BIN=${GBASF2BIN}   >>setup.sh
echo "export PYTHONPATH=${GBASF2ROOT}/lib/:$PYTHONPATH" >>setup.sh
echo "export PATH=${GBASF2ROOT}/bin/:$PATH" >>setup.sh
