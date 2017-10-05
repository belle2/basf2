#!/bin/bash

# ---------------------------------------------------------------
# job submission script for time base calibration: pocketDAQ data
#
# Contributors: Marko Staric, Umberto Tamponi 
#
# ---------------------------------------------------------------


echo "Time base calibration with local DAQ data: job submission"
if [ $# -ne 4 ]; then
    echo "usage: $0 <path_to_sroot> <run_number> <calibration_channel> <output_directory_name>"
    echo "Output naming convention: TBCYYYY-MM-comment"
    exit
fi

indir=$1        # path to sroot files
run=$2          # run number
ch=$3           # calibration channel
dtype=pocket    # data type
outmain="./"$4  # output directory 

outdir=${outmain}/run${run}-${dtype}-tbc_ch${ch}
mkdirhier ${outdir}

for file in `ls ${indir}/run*${run}*.sroot`;do
    fname=`basename $file`
    slot=`echo ${fname} | sed -e 's/slot/ /g'|sed -e 's/_/ /g'|awk '{print $2}'`
    logf=${outdir}/slot${slot}.log
    job="runTBC.py -i ${file} ${dtype} ${slot} ${ch} ${outdir} >& ${logf}"
    bsub -q s "basf2 ${job}"
done

