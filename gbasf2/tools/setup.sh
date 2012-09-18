#!/bin/bash

#By Yanliang Han
#2012-09-17
#Set gbasf2 environment variables.

export GBASF2TOOLS=`python -c 'import os,sys;print os.path.realpath(sys.argv[1])' $(dirname ${BASH_SOURCE:-$0})`
export GBASF2ROOT=`dirname ${GBASF2TOOLS}`
export GBASF2LIB=${GBASF2ROOT}/lib
export GBASF2BIN=${GBASF2ROOT}/bin
export GBASF2ETC=${GBASF2ROOT}/etc
export PYTHONPATH=${GBASF2LIB}:${PYTHONPATH}
export PATH=${GBASF2BIN}/:${PATH}

########## Set the variable DIRACROOT. The variable value is saved in etc/dirac.ini with one line.
if [ -e ${GBASF2ETC}/dirac.ini ]
  then
    DIRACTMP=`python -c 'import sys;file=open(sys.argv[1]);print file.read();file.close()' ${GBASF2ETC}/dirac.ini`
    if [ -e ${DIRACTMP}/bashrc ]
      then
        export DIRACROOT=${DIRACTMP}
      else
        echo "The dirac.ini is not correct. You can modify it or just remove it and then source the setup script again."
    fi
    unset DIRACTMP
  else
    echo "There is no DIRAC client found. Please input the DIRAC client location:"
    DIRACTMP=`python -c 'print raw_input()'`
    if [ -e ${DIRACTMP}/bashrc ]
      then
        echo ${DIRACTMP} > ${GBASF2ETC}/dirac.ini
        export DIRACROOT=${DIRACTMP}
      else
        echo "There is no DIRAC client found in your location. Please check the DIRAC client location"
    fi
      unset DIRACTMP
fi
