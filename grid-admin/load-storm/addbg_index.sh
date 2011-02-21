
#!/bin/bash

#

export BHOME=${BHOME:-/belle}

export BELLE_LEVEL=b20090127_0910

export BELLE_TOP_DIR=${BHOME}/belle/${BELLE_LEVEL}

export BELLE_DEBUG=opt

export BELLE_RUN_DIR=${BELLE_TOP_DIR}/x86_64-unknown-linux-gnu/${BELLE_DEBUG}

export CERN=${CERN:-${BHOME}/cern}

export CERN_ROOT=${CERN}/${CERN_LEVEL}

export PATH=${BHOME}/local/bin:/usr/local/bin:/usr/X11R6/bin:/usr/bin:/usr/sbin:/bin

export PATH=${PATH}:${BELLE_RUN_DIR}/bin

[ -n "${EXTRA_PATH}" ] && 
export PATH=${PATH}:${EXTRA_PATH}

export LD_LIBRARY_PATH=${BHOME}/local/lib:${BELLE_RUN_DIR}/lib/so:${BELLE_RUN_DIR}/bin

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CERN_ROOT}/lib64:${CERN_ROOT}/lib64/so

[ -n "${EXTRA_LIBRARY_PATH}" ] && 
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${EXTRA_LIBRARY_PATH}

export BASF_MODULE_DIR=.:${BELLE_RUN_DIR}/bin

export PANTHER_TABLE_DIR=${BELLE_TOP_DIR}/share/tables


for bkgfile in $*

do

indexfile=$( basename $bkgfile .bbs).addbg_index

echo Creating $indexfile ...

dblist_addbg $bkgfile tmpBackground-

mv tmpBackground* $indexfile

echo "Done"

done

