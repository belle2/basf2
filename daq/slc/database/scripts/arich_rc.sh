#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

createParamTable copper ${DIR}/
createParamTable copper.setup ${DIR}/
createParamTable copper.fee ${DIR}/
createParamTable copper.fee.register ${DIR}/
createParamTable copper.fee.parameter ${DIR}/
createParamTable ropc ${DIR}/
createParamTable ropc.hostlist ${DIR}/
createParamTable runcontrol ${DIR}/
createParamTable runcontrol.node ${DIR}/
createParamTable runcontrol.nsmdata ${DIR}/

#addParamConfig ARICH copper.hslb ${DIR}/copper.hslb.csv
addParamConfig ARICH copper.fee.parameter ${DIR}/copper.fee.parameter.csv
addParamConfig ARICH copper.fee.register ${DIR}/copper.fee.register.csv
addParamConfig ARICH copper.fee ${DIR}/copper.fee.csv
addParamConfig ARICH copper.setup ${DIR}/copper.setup.csv
addParamConfig ARICH copper ${DIR}/copper.csv
addParamConfig ARICH ropc.hostlist ${DIR}/ropc.hostlist.csv 
addParamConfig ARICH ropc ${DIR}/ropc.csv
addParamConfig DAQ runcontrol.node ${DIR}/runcontrol.node.csv
addParamConfig DAQ runcontrol.nsmdata ${DIR}/runcontrol.nsmdata.csv
addParamConfig DAQ runcontrol ${DIR}/runcontrol.csv
