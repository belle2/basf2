#!/bin/bash

export B2SC_SERVER_HOST=`hostname`;

## setup for slow control system libraries ##
export BELLE2_DAQ_SLC_DIR=$PWD
export PATH=$PATH:$BELLE2_DAQ_SLC_DIR/bin
export LD_LIBRARY_PATH=$BELLE2_DAQ_SLC_DIR/lib:$LD_LIBRARY_PATH

## setup for slow control system java libraries ##
export JAVA_HOME=/usr/java/latest/
export PATH=$JAVA_HOME/bin:$PATH
export CLASSPATH=.:$JAVA_HOME/jre/lib:$JAVA_HOME/lib\
:$JAVA_HOME/lib/tools.jar\
:$BELLE2_DAQ_SLC_DIR/javalib/mysql-connector-java-5.1.26-bin.jar

## NSM configuration ##
export NSM2_HOST=${B2SC_SERVER_HOST}
export NSM2_PORT=8222
export NSM2_SHMKEY=8222
export NSM2_INCDIR=$BELLE2_DAQ_SLC_DIR/data/nsm
export NSMD2_DEBUG=1
export NSMD2_LOGDIR=$BELLE2_DAQ_SLC_DIR/log/nsm2

## setup for slow control database access ##
export B2SC_DB_HOST=${B2SC_SERVER_HOST};
export B2SC_DB_NAME="b2slow_test";
export B2SC_DB_USER="slcdaq";
export B2SC_DB_PASS="slcdaq";
export B2SC_DB_PORT=3306;

## setup for slow control database access ##
export B2SC_XML_ENTRY="CDC"
export B2SC_XML_PATH=$BELLE2_DAQ_SLC_DIR/config/cdc_test
export B2SC_CPRLIB_PATH=$BELLE2_DAQ_SLC_DIR/lib/cdc_test

## setup for slow control database access ##
export B2SC_DQM_CONFIG_PATH=$BELLE2_DAQ_SLC_DIR/dqmserver/config/test.conf
export B2SC_DQM_MAP_PATH=$BELLE2_DAQ_SLC_DIR/log
export B2SC_DQM_LIB_PATH=$BELLE2_DAQ_SLC_DIR/dqmserver/lib

## setup for directory 
export HSLB_FIRMEWATE_PATH=${BELLE2_DAQ_SLC_DIR}/cprcontrold/mgt/
export TTRX_FIRMEWATE_PATH=/home/usr/nakao/daq/ftsw/
export FTSW_FIRMEWATE_PATH=/home/usr/nakao/daq/ftsw/
export FEE_FIRMWARE_PATH=/home/usr/tkonno/cdc