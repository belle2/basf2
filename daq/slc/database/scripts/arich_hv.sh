#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

createParamTable hvconfig ${DIR}/
createParamTable hvconfig.channel ${DIR}/
createParamTable hvconfig.valueset ${DIR}/
createParamTable hvconfig.valueset.value ${DIR}/

addParamConfig ARICH hvconfig.valueset.value ${DIR}/arich.hvconfig.valueset.value.csv
addParamConfig ARICH hvconfig.valueset ${DIR}/arich.hvconfig.valueset.csv
addParamConfig ARICH hvconfig.channel ${DIR}/arich.hvconfig.channel.csv
addParamConfig ARICH hvconfig ${DIR}/arich.hvconfig.csv
