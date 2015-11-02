#!/bin/bash

export BELLE2_DAQ_SLC=$PWD
export PATH=$BELLE2_DAQ_SLC/bin:$PATH
export PGSQL_LIB_PATH=/usr/pgsql-9.3/
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC/lib:$PGSQL_LIB_PATH/lib:$LD_LIBRARY_PATH
export NSM2_INCDIR=$BELLE2_DAQ_SLC/data/nsm

## set NOT_USE_PSQL yes if postreSQL is not available
#export NOT_USE_PSQL=yes
#!/bin/bash

