#!/bin/bash

# ---------------------------------------------------------------
# job submission script for time base calibration: local DAQ data
#
# Contributors: Marko Staric, Umberto Tamponi
#
# ---------------------------------------------------------------


echo "Time base calibration with local DAQ data: job submission"
if [ $# -ne 3 ]; then
    echo "usage: $0 <run_number> <calibration_channel> <output_directory_name>"
    echo "Output naming convention: TBCYYYY-MM-comment"
    exit
fi 

indir=/ghi/fs01/belle2/bdata/Data/sRaw/e0001  # path to sroot files
run=$1          # run number
ch=$2           # calibration channel
dtype=local     # data type
outmain="./"$3  # output directory


outdir=${outmain}/run${run}-${dtype}-tbc_ch${ch}
mkdirhier ${outdir}

if [ ! -f ${indir}/r*${run}/sub00/*.sroot ]; then
    echo "*** no files found for run ${run} in ${indir}"
    exit
fi

files=''
for file in `ls -1 ${indir}/r*${run}/sub00/*.sroot`;do
    files="${files} -i ${file}"
done

slot=1
while [ ${slot} -le 16 ]; do
    logf=${outdir}/slot${slot}.log
    job="runTBC.py ${files} ${dtype} ${slot} ${ch} ${outdir} >& ${logf}"
    bsub -q l "basf2 ${job}"
    slot=$((${slot}+1))
done

