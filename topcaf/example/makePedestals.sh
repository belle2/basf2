#!/usr/bin/env bash

### Usage: ./makePedestals nFrames file1 file2 file3

for infile in ${2} ${3} ${4}
do
	inputRun=$(basename ${infile})
	inputDir=$(dirname ${infile})
	basf2 topcaf_itop_makeped_IRSXGIGE_pocketDAQ.py -n ${1} --arg --inputRun ${inputRun/.dat/} --arg --inputDir ${inputDir}
done

hadd $(basename ${2/.dat/})_Pedestals.root *PedestalCalibration.root
rm *PedestalCalibration.root

