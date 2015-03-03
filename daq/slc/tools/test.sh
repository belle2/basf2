psql -U b2daq -d b2slow -h b2slow2.kek.jp -c "drop table ecl;"
psql -U b2daq -d b2slow -h b2slow2.kek.jp -c "drop table daq;"
addParamConfig ecl /home/usr/tkonno/slc/data/database/copper.fee.register.conf /home/usr/tkonno/slc/data/database/ecl.copper.fee.register.csv register 
addParamConfig ecl /home/usr/tkonno/slc/data/database/copper.fee.parameter.conf /home/usr/tkonno/slc/data/database/ecl.copper.fee.parameter.csv parameter 
addParamConfig ecl /home/usr/tkonno/slc/data/database/copper.fee.conf /home/usr/tkonno/slc/data/database/ecl.copper.fee.csv fee 
addParamConfig ecl /home/usr/tkonno/slc/data/database/copper.setup.conf /home/usr/tkonno/slc/data/database/ecl.copper.setup.csv setup 
addParamConfig ecl /home/usr/tkonno/slc/data/database/copper.conf /home/usr/tkonno/slc/data/database/ecl.copper.csv
addParamConfig ecl /home/usr/tkonno/slc/data/database/ropc.sender.conf /home/usr/tkonno/slc/data/database/ecl.ropc.sender.csv sender 
addParamConfig ecl /home/usr/tkonno/slc/data/database/ropc.conf /home/usr/tkonno/slc/data/database/ecl.ropc.csv
addParamConfig ecl /home/usr/tkonno/slc/data/database/ecl.shaper.register.conf /home/usr/tkonno/slc/data/database/ecl.shaper.register.csv register 
addParamConfig ecl /home/usr/tkonno/slc/data/database/ecl.shaper.parameter.conf /home/usr/tkonno/slc/data/database/ecl.shaper.parameter.csv parameter 
addParamConfig ecl /home/usr/tkonno/slc/data/database/ecl.shaper.conf /home/usr/tkonno/slc/data/database/ecl.shaper.csv shaper 
addParamConfig ecl /home/usr/tkonno/slc/data/database/ecl.conf /home/usr/tkonno/slc/data/database/ecl.csv
addParamConfig ecl /home/usr/tkonno/slc/data/database/runcontrol.conf /home/usr/tkonno/slc/data/database/ecl0102.runcontrol.node.csv node 
addParamConfig ecl /home/usr/tkonno/slc/data/database/runcontrol.conf /home/usr/tkonno/slc/data/database/ecl.runcontrol.node.csv node 

addParamConfig daq /home/usr/tkonno/slc/data/database/runcontrol.conf /home/usr/tkonno/slc/data/database/runcontrol.node.csv node 
addParamConfig daq /home/usr/tkonno/slc/data/database/ttd.conf /home/usr/tkonno/slc/data/database/ttd.csv
addParamConfig daq /home/usr/tkonno/slc/data/database/hlt.conf /home/usr/tkonno/slc/data/database/hlt.csv
#addParamConfig daq /home/usr/tkonno/slc/data/database/storage.conf /home/usr/tkonno/slc/data/database/storage.csv
