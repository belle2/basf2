#!/bin/bash

#
# this is the executable bash script in the HTCondor Nodes
#

b2Dir="/nfs/dust/belle2/user/mondal/products/basf2/release_local/"

cd $b2Dir
source /cvmfs/belle.cern.ch/tools/b2setup ""


exec='basf2 -n '$1' -p 10 svd/examples/svdEventT0PerformanceTTree.py -- --RootOutput --noEventT0Tree --executionTime --isMC --fileTag 90445963f_OffOn_frac5p_sig3'
echo $exec
echo $exec | sh
