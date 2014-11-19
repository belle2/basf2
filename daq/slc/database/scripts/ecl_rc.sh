#!/bin/bash

DIR=${BELLE2_LOCAL_DIR}/daq/slc/data/database

createParamTable copper ${DIR}/
createParamTable copper.setup ${DIR}/
createParamTable copper.fee ${DIR}/
createParamTable copper.fee.register ${DIR}/
createParamTable copper.fee.parameter ${DIR}/
createParamTable ropc ${DIR}/
createParamTable ropc.copper_from ${DIR}/
createParamTable hlt ${DIR}/
createParamTable storage ${DIR}/
createParamTable ttd ${DIR}/
createParamTable runcontrol ${DIR}/
createParamTable runcontrol.node ${DIR}/
createParamTable runcontrol.nsmdata ${DIR}/
createParamTable ecl ${DIR}/
createParamTable ecl.shaper ${DIR}/
createParamTable ecl.shaper.register ${DIR}/
createParamTable ecl.shaper.parameter ${DIR}/

addParamConfig ECL copper.fee.parameter ${DIR}/ecl.copper.fee.parameter.csv
addParamConfig ECL copper.fee.register ${DIR}/ecl.copper.fee.register.csv
addParamConfig ECL copper.fee ${DIR}/ecl.copper.fee.csv
addParamConfig ECL copper.setup ${DIR}/ecl.copper.setup.csv
addParamConfig ECL copper ${DIR}/ecl.copper.csv
addParamConfig ECL ropc.copper_from ${DIR}/ecl.ropc.copper_from.csv
addParamConfig ECL ropc ${DIR}/ecl.ropc.csv
addParamConfig ECL ecl.shaper.parameter ${DIR}/ecl.shaper.parameter.csv
addParamConfig ECL ecl.shaper.register ${DIR}/ecl.shaper.register.csv
addParamConfig ECL ecl.shaper ${DIR}/ecl.shaper.csv
addParamConfig ECL ecl ${DIR}/ecl.csv
addParamConfig DAQ hlt ${DIR}/hlt.csv
addParamConfig DAQ storage ${DIR}/storage.csv
addParamConfig DAQ ttd ${DIR}/ttd.csv
addParamConfig DAQ runcontrol.node ${DIR}/ecl0102.runcontrol.node.csv
addParamConfig DAQ runcontrol.nsmdata ${DIR}/ecl0102.runcontrol.nsmdata.csv
addParamConfig DAQ runcontrol ${DIR}/ecl0102.runcontrol.csv
addParamConfig DAQ runcontrol.node ${DIR}/ecl.runcontrol.node.csv
addParamConfig DAQ runcontrol.nsmdata ${DIR}/ecl.runcontrol.nsmdata.csv
addParamConfig DAQ runcontrol ${DIR}/ecl.runcontrol.csv
addParamConfig DAQ runcontrol.node ${DIR}/runcontrol.node.csv
addParamConfig DAQ runcontrol.nsmdata ${DIR}/runcontrol.nsmdata.csv
addParamConfig DAQ runcontrol ${DIR}/runcontrol.csv
