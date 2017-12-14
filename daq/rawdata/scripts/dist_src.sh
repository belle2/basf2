#!/bin/sh
#
# Update basf2 program and do compiling on remote hosts.
# Made for HLT test with dummy data 
# 2016.1.4 S. Yamada
# 

ssh -n -n -XY svd02 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY svd03 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY svd04 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY svd05 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY svd06 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY svd07 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY svd08 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"

ssh -n -n -XY cdc01 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc02 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc03 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc04 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc05 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc06 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc07 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc08 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc09 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY cdc10 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"

ssh -n -n -XY top01 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY top02 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY top03 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY top04 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"

ssh -n -n -XY arich01 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY arich02 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"


ssh -n -n -XY ecl01 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY ecl02 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY ecl03 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY ecl04 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY ecl05 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY ecl06 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"

ssh -n -n -XY klm01 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY klm02 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
ssh -n -n -XY klm03 "echo \${HOSTNAME}; cd ~/basf2/release/daq/rawdata/src; svn update; g++ -o dummy_data_src dummy_data_src.cc;"
