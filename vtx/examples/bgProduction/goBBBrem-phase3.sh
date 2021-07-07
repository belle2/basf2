#!/bin/bash

# job submission script for l queue 
# all samples of one set except BBBrem

nsets=1      # number of mixing sets
phase=3
vtx=VTX-CMOS-7layer

setN=0    # set counter
while [ $setN -lt ${nsets} ]; do

  outdir=./upgrade/mixing/set${setN} # generated samples
  logdir=log-ph${phase}/set${setN}    # log files

  mkdirhier ${outdir}
  mkdirhier ${logdir}

  # RBB (BBBrem) samples
  t=1     # equivalent time per job in [us] 
  Nusual=600  # number of jobs for 'usual'
  NECL=600    # number of jobs for 'ECL'

  for typ in usual ECL; do
    nnn=N${typ}
    njobs=$((${nnn}))
    num=0
    while [ $num -lt ${njobs} ]; do 
      logf=${logdir}/RBB_${typ}-phase${phase}-${num}.out
      errf=${logdir}/RBB_${typ}-phase${phase}-${num}.err
      job="generateRadBhabhas.py bbbrem ${t} ${num} ${typ} ${phase} ${outdir} ${vtx}"
      bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
      num=$(($num+1))
    done
  done

  echo "jobs for set ${setN} submitted"
  echo "- output files in ${outdir}"
  echo "- log files in ${logdir}"
  
  setN=$(($setN+1))
done
