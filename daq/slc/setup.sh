#!/bin/bash

#HOSTNAME=`hostname`
## setup for Basf2 framework ##
#if [ ! -n "${BELLE2_LOCAL_DIR}" ]; then 
#  source $HOME/belle2/tools/setup_belle2
#  cd $HOME/belle2/release/
#  setuprel
#  export BELLE2_OPTION=opt
#  export BELLE2_SUBDIR=${BELLE2_ARCH}/${BELLE2_OPTION}
#  cd -
#fi

## setup for slow control system libraries ##
export B2SLC_PATH=$BELLE2_LOCAL_DIR/daq/slc
export B2SLC_PATH=$PWD
export PATH=$PATH:$B2SLC_PATH/bin
export LD_LIBRARY_PATH=$B2SLC_PATH/lib:$LD_LIBRARY_PATH

## setup for slow control system java libraries ##
export JAVA_HOME=/usr/java/latest/
export PATH=$JAVA_HOME/bin:$PATH
export CLASSPATH=.:$JAVA_HOME/jre/lib:$JAVA_HOME/lib\
:$JAVA_HOME/lib/tools.jar\
:$B2SLC_PATH/javalib/mysql-connector-java-5.1.26-bin.jar
export B2SC_SERVER_HOST="belle-rpc1";
#export B2SC_SERVER_HOST="b2slow2.kek.jp";

## NSM configuration ##
export NSM2_HOST=`/sbin/ifconfig | grep "192\.168\.10\." | sed "s/:/ /g" | awk '{print $3}'`
export NSM2_HOST=130.87.227.252
export NSM2_PORT=8222
export NSM2_SHMKEY=8222
export NSM2_INCDIR=$B2SLC_PATH/bin
export NSMD2_DEBUG=1
export NSMD2_LOGDIR=$B2SLC_PATH/log/nsm2

## setup for slow control database access ##
export B2SC_DB_HOST=${B2SC_SERVER_HOST};
export B2SC_DB_NAME="b2slow_test";
export B2SC_DB_USER="slcdaq";
export B2SC_DB_PASS="slcdaq";
export B2SC_DB_PORT=3306;

## setup for slow control database access ##
export B2SC_XML_ENTRY="CDC"
export B2SC_XML_PATH=$B2SLC_PATH/config/cdc_test
export B2SC_CPRLIB_PATH=$B2SLC_PATH/lib/cdc_test

## setup for slow control database access ##
export B2SC_DQM_CONFIG_PATH=$B2SLC_PATH/dqmserver/config/test.conf
export B2SC_DQM_MAP_PATH=$B2SLC_PATH/log
export B2SC_DQM_LIB_PATH=$B2SLC_PATH/dqmserver/lib

## setup for directory 
#export HSLB_FIRMEWATE_PATH=/home/usr/yamadas/bit/
export HSLB_FIRMEWATE_PATH=${B2SLC_PATH}/cprcontrold/mgt/
export TTRX_FIRMEWATE_PATH=/home/usr/nakao/daq/ftsw/
export FTSW_FIRMEWATE_PATH=/home/usr/nakao/daq/ftsw/
export FEE_FIRMWARE_PATH=/home/usr/tkonno/cdc