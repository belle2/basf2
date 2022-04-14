#!/bin/bash

#sample run script

COUNTER=0

outdir=""
mkdir ${outdir}

for dir in /group/belle2/dataprod/Data/release-04-02-08/DB00000498/bucket13/e0012/4S/*/skim/hlt_gamma_gamma/cdst/sub00/
do
  echo ${dir}
  echo " $COUNTER "
  COUNTER=$[$COUNTER +1]
  echo bsub -o logfiles/log_${COUNTER}.log -q s "basf2 extractRawWaveforms.py ${COUNTER} ${outdir} -i \"${dir}*.root\"" 
done
