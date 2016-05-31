#!/bin/bash
#
# Script for the creation of the topcaf Hits. Uses the "self suptracted pedestals".
# Arguments:
# 1 - absolute path to the sroot file
# 2 - output file name
# 
# Author: U. Tamponi
#         tamponi@to.infn.it

mkdir tmp/topcaftmp
cd tmp/topcaftmp
oldpwd=$(pwd)
cd ../..
echo ${oldpwd}/$(basename ${1/.sroot/ped.root})
echo ${1}
echo ${2}
 basf2 topcaf_itop_sroot_ped.py --arg --inputRun=${1} --arg --outputRun=${oldpwd}/$(basename ${1/.sroot/ped.root}) -l ERROR -n 3200
 basf2 topcaf_itop_sroot_hits.py --arg --inputRun=${1} --arg --ped=${oldpwd}/$(basename ${1/.sroot/ped.root}) --arg --output=${oldpwd}/$(basename ${2}) 
chmod go+w *.root
mv ${oldpwd}/$(basename ${2}) ${2}
