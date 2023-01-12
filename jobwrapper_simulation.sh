#!/bin/bash

#
# this is the executable bash script in the HTCondor Nodes
#

b2Dir="/nfs/dust/belle2/user/mondal/products/basf2/release_local/"

cd $b2Dir
source /cvmfs/belle.cern.ch/tools/b2setup ""

# run 0 : OnOn
# run 1 : OffOn_frac5p_sig3
# run 2 : OffOff_frac5p_sig3
# run 3 : OnOn_frac5p_sig3

exec='basf2 --exp 0 --run 1 -n '$1' -p 10 svd/examples/svdEventT0PerformanceTTree.py -- --RootOutput --noEventT0Tree --executionTime --is3sample --isMC --fileTag bcb937ab8_OffOn_frac5p_sig3'
echo $exec
echo $exec | sh
