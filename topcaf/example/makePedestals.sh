#!/usr/bin/env bash

### Usage: ./makePedestals nFrames file1 file2 file3

mkdir garbleStrube
cd garbleStrube

for infile in ${1} ${2} ${3}
do
	inputRun=$(basename ${infile})
	inputDir=$(dirname ${infile})
	basf2 ../topcaf_itop_makeped_IRSXGIGE_pocketDAQ.py --arg --inputRun ${inputRun/.dat/} --arg --inputDir ${inputDir}
done

hadd $(basename ${1/.dat/})_Pedestals.root *PedestalCalibration.root
rm *PedestalCalibration.root
mv *_Pedestals.root ..
cd ..
rm -r garbleStrube

