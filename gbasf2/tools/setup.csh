#!/bin/tcsh

#By Yanliang Han
#2012-09-17
#Set gbasf2 environment variables.
set COMMAND=`echo $_`
if ( "${COMMAND}" != "" ) then
  set FILENAME=`echo ${COMMAND} | awk '{print $2}'`
endif
set DIRNAME=`dirname ${FILENAME}`
setenv GBASF2TOOLS `python -c 'import os,sys;print os.path.realpath(sys.argv[1])' ${DIRNAME}`
unset DIRNAME
setenv GBASF2ROOT `dirname ${GBASF2TOOLS}`
setenv GBASF2LIB ${GBASF2ROOT}/lib
setenv GBASF2BIN ${GBASF2ROOT}/bin
setenv GBASF2ETC ${GBASF2ROOT}/etc
setenv PYTHONPATH ${GBASF2LIB}:${PYTHONPATH}
setenv PATH ${GBASF2BIN}/:${PATH}

########## Set the variable DIRACROOT. The variable value is saved in etc/dirac.ini with one line.
if -e ${GBASF2ETC}/dirac.ini then
  set DIRACTMP=`python -c 'import sys;file=open(sys.argv[1]);print file.read();file.close()' ${GBASF2ETC}/dirac.ini`
  if -e ${DIRACTMP}/bashrc then
    setenv DIRACROOT ${DIRACTMP}
  else
    echo "The dirac.ini is not correct. You can modify it or just remove it and then source the setup script again."
  endif
  unset DIRACTMP
else
  echo "There is no DIRAC client found. Please input the DIRAC client location:"
  set DIRACTMP=`python -c 'print raw_input()'`
  if -e ${DIRACTMP}/bashrc then
    echo ${DIRACTMP} > ${GBASF2ETC}/dirac.ini
    setenv DIRACROOT ${DIRACTMP}
  else
    echo "There is no DIRAC client found in your location. Please check the DIRAC client location"
  endif
  unset DIRACTMP
endif
