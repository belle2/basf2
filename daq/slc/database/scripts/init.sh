#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

cleanTables

dbhost=`getconfig slowcontrol database.host`

psql -U b2daq -d b2slow -h ${dbhost} < ${BELLE2_LOCAL_DIR}/daq/slc/database/scripts/init.sql

