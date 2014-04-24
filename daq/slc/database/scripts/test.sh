#!/bin/bash

psql -U b2daq -d b2slow < init.sql
createParamTable copper ~/rcconfig/
createParamTable copper.hslb ~/rcconfig/
addParamConfig SVD copper.hslb ~/rcconfig/copper.hslb.csv
addParamConfig SVD copper ~/rcconfig/copper.csv
createParamTable ropc ~/rcconfig/
createParamTable ropc.from_hosts ~/rcconfig/
addParamConfig SVD ropc.from_hosts ~/rcconfig/ropc.from_hosts.csv 
addParamConfig SVD ropc ~/rcconfig/ropc.csv
createParamTable runcontrol ~/rcconfig/
createParamTable runcontrol.node ~/rcconfig/
createParamTable runcontrol.nsmdata ~/rcconfig/
addParamConfig SVD runcontrol.node ~/rcconfig/runcontrol.node.csv
addParamConfig SVD runcontrol.nsmdata ~/rcconfig/runcontrol.nsmdata.csv
addParamConfig SVD runcontrol ~/rcconfig/runcontrol.csv
createParamTable hvconfig ~/rcconfig/
createParamTable hvconfig.channel ~/rcconfig/
addParamConfig ARICH hvconfig.channel ~/rcconfig/arich.csv
addParamConfig ARICH hvconfig ~/rcconfig/arich.hvconfig.csv
