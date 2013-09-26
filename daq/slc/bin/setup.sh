
#!/bin/bash

## setup for Basf2 framework ##
if [ ! -n "${BELLE2_LOCAL_DIR}" ]; then 
source $HOME/tools/setup_belle2
cd $HOME/release/
setuprel
cd -
fi

## setup for slow control system libraries ##
export B2SLC_PATH=$BELLE2_LOCAL_DIR/daq/slc
export PATH=$PATH:$HOME/bin:$B2SLC_PATH/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$B2SLC_PATH/lib

## NSM configuration ##
export NSM2_HOST=`/sbin/ifconfig | grep "192\.168\.10\." | sed "s/:/ /g" | awk '{print $3}'`
#export NSM2_HOST=130.87.227.252
#export NSM2_HOST=192.168.244.136
export NSM2_PORT=8122
export NSM2_SHMKEY=8122
export NSMD2_DEBUG=1
export NSMD2_LOGDIR=$B2SLC_PATH/log/nsm2

## setup for slow control system java libraries ##
export JAVA_HOME=/usr/java/latest/
export PATH=$JAVA_HOME/bin:$PATH
export CLASSPATH=.:$JAVA_HOME/jre/lib:$JAVA_HOME/lib\
:$JAVA_HOME/lib/tools.jar\
:$B2SLC_PATH/javalib/mysql-connector-java-5.1.26-bin.jar

## setup for slow control database access ##
export B2SC_DB_HOST="localhost";
export B2SC_DB_NAME="b2slow_test";
export B2SC_DB_USER="slcdaq";
export B2SC_DB_PASS="slcdaq";
export B2SC_DB_PORT=3306;

## setup for directory 
export HSLB_FIRMEWATE_PATH=/home/usr/yamadas/bit/
export FTSW_FIRMEWATE_PATH=/home/usr/tkonno
