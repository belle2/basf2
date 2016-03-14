#!/bin/bash
INPUTFILE=${1}
PEDESTAL=${2}
basf2 topcaf_itop_sroot_waves.py --arg --inputRun ${INPUTFILE} --arg --ped ${PEDESTAL} -n 100 -l ERROR
mv histofile.root $(basename ${INPUTFILE/.sroot/_waves.root})

