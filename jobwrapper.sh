#!/bin/bash

#
# this is the executable bash script in the HTCondor Nodes
#

b2Dir="/nfs/dust/belle2/user/mondal/products/basf2/release_local/"

cd $b2Dir
source /cvmfs/belle.cern.ch/tools/b2setup ""



# Replace values in svd/scripts/svd/__init__.py

cat svd/scripts/svd/__init__.py |  sed  's/svdTimeGroupComposer.param(\x27AverageCountPerBin\x27.*/svdTimeGroupComposer.param(\x27AverageCountPerBin\x27, '"${1}"')/g' > tmp.py
cat tmp.py |  sed  's/svdTimeGroupComposer.param(\x27Threshold\x27.*/svdTimeGroupComposer.param(\x27Threshold\x27, '"${2}"')/g' > tmp1.py
# diff tmp1.py svd/scripts/svd/__init__.py
cp tmp1.py svd/scripts/svd/__init__.py

foo1=$(printf "%02d" $1)
foo2=$(printf "%02d" $2)

# exec="b2validation -p 10 -pkg reconstruction analysis background svd tracking -t proposed"

# exec="b2validation -p 10 -s fullTrackingValidationBkg.py -t proposed_28220acce" # TimeGroupComposer (1, 1)
# exec="b2validation -p 10 -s fullTrackingValidationBkg.py -t proposed_fcff92fe5" # TimeGroupComposer (10, 2) got screwed when I ran with -pkg option
# exec="b2validation -p 10 -s fullTrackingValidationBkg.py -t proposed_b6e67e021" # TimeGroupComposer (4, 8)
# exec="b2validation -p 10 -s fullTrackingValidationBkg.py -t proposed_7e200bc44" # TimeGroupComposer (10, 4)

exec='b2validation -o "-n 1000 -p 10" -s fullTrackingValidationBkg.py CombinedSVDTrackingValidationBkg.py VXDTF2TrackingValidationBkg.py toCDCCKFTrackingValidationBkg.py -t proposed_OnOn_'${foo1}'_'${foo2}
echo $exec
echo $exec | sh
