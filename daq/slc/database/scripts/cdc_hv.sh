#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

createParamTable hvconfig ${DIR}/
createParamTable hvconfig.channel ${DIR}/
createParamTable hvconfig.valueset ${DIR}/
createParamTable hvconfig.valueset.value ${DIR}/

addParamConfig CDC hvconfig.valueset.value ${DIR}/cdc.hvconfig.valueset.value.csv
addParamConfig CDC hvconfig.valueset ${DIR}/cdc.hvconfig.valueset.csv
addParamConfig CDC hvconfig.channel ${DIR}/cdc.hvconfig.channel.csv
addParamConfig CDC hvconfig ${DIR}/cdc.hvconfig.csv
