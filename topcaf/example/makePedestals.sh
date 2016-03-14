#!/usr/bin/env bash

### Usage: ./makePedestals file1 file2 file3

mkdir garbleStrube
cd garbleStrube

for infile in ${1} ${2} ${3}
do
	basf2 ../topcaf_itop_sroot_ped.py --arg --inputRun ${infile} -l ERROR
 	mv pedout.root $(basename ${infile/.sroot/PedestalCalibration.root})
done

hadd $(basename ${1/.sroot/})_Pedestals.root *PedestalCalibration.root
rm *PedestalCalibration.root
mv *_Pedestals.root ..
cd ..
rm -r garbleStrube

