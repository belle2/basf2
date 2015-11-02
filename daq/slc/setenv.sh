#!/bin/bash

export BELLE2_DAQ_SLC=$PWD
export PATH=$BELLE2_DAQ_SLC/bin:$PATH
if ! test "$BELLE2_LOCAL_DIR" = "" ; then
#if [ -z ${BELLE2_LOCAL_DIR+x} ]; then
  if [ -d $BELLE2_EXTERNALS_DIR/lib/$BELLE2_EXTERNALS_SUBDIR/ ]; then
    export PGSQL_LIB_PATH=$BELLE2_EXTERNALS_DIR/lib/$BELLE2_EXTERNALS_SUBDIR
  else 
    export PGSQL_LIB_PATH=$BELLE2_EXTERNALS_DIR/$BELLE2_EXTERNALS_SUBDIR/lib
  fi
  export PGSQL_INC_PATH=$BELLE2_EXTERNALS_DIR/include/pgsql
  export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$LD_LIBRARY_PATH
else
  export PGSQL_LIB_PATH=/usr/pgsql-9.3/lib
  export PGSQL_INC_PATH=/usr/pgsql-9.3/include
  export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$PGSQL_LIB_PATH/lib:$LD_LIBRARY_PATH
fi
export NSM2_INCDIR=$BELLE2_DAQ_SLC/data/nsm
## set NOT_USE_PSQL yes if postreSQL is not available
#export NOT_USE_PSQL=yes
#!/bin/bash

