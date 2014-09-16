#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

cleanTables

psql -U b2daq -d b2slow -h b2db1.daqnet.kek.jp < ${BELLE2_LOCAL_DIR}/daq/slc/database/scripts/init.sql

