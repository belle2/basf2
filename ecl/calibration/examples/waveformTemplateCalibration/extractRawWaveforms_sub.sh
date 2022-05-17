#/**************************************************************************
# * basf2 (Belle II Analysis Software Framework)                           *
# * Author: The Belle II Collaboration                                     *
# *                                                                        *
# * See git log for contributors and copyright holders.                    *
# * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
# **************************************************************************/

COUNTER=0

# example:/group/belle2/dataprod/Data/release-04-02-08/DB00000498/bucket13/e0012/4S/*/skim/hlt_gamma_gamma/cdst/sub00/
inputDirectory=""

# example:logfiles/
logfileDirectory=""

[[  -z  $param  ]] && echo "input directory not set" && exit
[[  -z  $param  ]] && echo "logfile directory not set." && exit

outdir=""
mkdir ${outdir}

for dir in ${inputDirectory}
do
  echo ${dir}
  echo " $COUNTER "
  COUNTER=$[$COUNTER +1]
  echo bsub -o ${logfileDirectory}log_${COUNTER}.log -q s "basf2 extractRawWaveforms.py ${COUNTER} ${outdir} -i \"${dir}*.root\"" 
done
