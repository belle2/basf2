#!/bin/bash
#Add gbasf2 path to the environment PATH
export GBASF2TOOLS=`python -c 'import os,sys;print os.path.realpath(sys.argv[1])' $(dirname ${BASH_SOURCE:-$0})`
export GBASF2ROOT=`dirname ${GBASF2TOOLS}`
export GBASF2LIB=${GBASF2ROOT}/lib
export GBASF2BIN=${GBASF2ROOT}/bin
export PYTHONPATH=${GBASF2LIB}:${PYTHONPATH}
export PATH=${GBASF2BIN}/:${PATH}
