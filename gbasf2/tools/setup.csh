#!/bin/tcsh
set COMMAND=`echo $_`
if ( "${COMMAND}" != "" ) then
  set FILENAME=`echo ${COMMAND} | awk '{print $2}'`
endif
set DIRNAME=`dirname ${FILENAME}`
setenv GBASF2TOOLS `python -c 'import os,sys;print os.path.realpath(sys.argv[1])' ${DIRNAME}`
setenv GBASF2ROOT `dirname ${GBASF2TOOLS}`
setenv GBASF2LIB ${GBASF2ROOT}/lib
setenv GBASF2BIN ${GBASF2ROOT}/bin
setenv PYTHONPATH ${GBASF2LIB}:${PYTHONPATH}
setenv PATH ${GBASF2BIN}/:${PATH}
