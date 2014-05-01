#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

cleanTables

psql -U b2daq -d b2slow < ${BELLE2_LOCAL_DIR}/daq/slc/database/scripts/init.sql

createParamTable copper ${DIR}/
createParamTable copper.hslb ${DIR}/
createParamTable ropc ${DIR}/
createParamTable ropc.from_hosts ${DIR}/
createParamTable runcontrol ${DIR}/
createParamTable runcontrol.node ${DIR}/
createParamTable runcontrol.nsmdata ${DIR}/
createParamTable hvconfig ${DIR}/
createParamTable hvconfig.channel ${DIR}/

addParamConfig ARICH copper.hslb ${DIR}/copper.hslb.csv
addParamConfig ARICH copper ${DIR}/copper.csv
addParamConfig ARICH ropc.from_hosts ${DIR}/ropc.from_hosts.csv 
addParamConfig ARICH ropc ${DIR}/ropc.csv
addParamConfig ARICH runcontrol.node ${DIR}/runcontrol.node.csv
addParamConfig ARICH runcontrol.nsmdata ${DIR}/runcontrol.nsmdata.csv
addParamConfig ARICH runcontrol ${DIR}/runcontrol.csv
addParamConfig ARICH hvconfig.channel ${DIR}/arich.csv
addParamConfig ARICH hvconfig ${DIR}/arich.hvconfig.csv
