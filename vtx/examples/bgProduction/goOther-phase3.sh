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


  # SAD samples
  t=100     # equivalent time per job in [us] 
  Nusual=6  # number of jobs for 'usual'
  NECL=6    # number of jobs for 'ECL'

  for s in Touschek Coulomb Brems; do
    for r in HER LER; do
      for typ in usual ECL; do
        nnn=N${typ}
        njobs=$((${nnn}))
        num=0
        while [ $num -lt ${njobs} ]; do 
          logf=${logdir}/${s}_${r}_${typ}-phase${phase}-${num}.out
          errf=${logdir}/${s}_${r}_${typ}-phase${phase}-${num}.err
          job="generateSADBg.py ${s} ${r} ${t} ${num} ${typ} ${phase} ${outdir} ${vtx}"
 	  bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
          num=$(($num+1))
        done
      done
    done
  done

  # BHWide samples
  t=100     # equivalent time per job in [us] 
  Nusual=6  # number of jobs for 'usual'
  NECL=12    # number of jobs for 'ECL'

  for typ in usual ECL; do
    nnn=N${typ}
    njobs=$((${nnn}))
    num=0
    while [ $num -lt ${njobs} ]; do 
      logf=${logdir}/BHWide_${typ}-phase${phase}-${num}.out
      errf=${logdir}/BHWide_${typ}-phase${phase}-${num}.err
      job="generateRadBhabhas.py bhwide ${t} ${num} ${typ} ${phase} ${outdir} ${vtx}"
      bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
      num=$(($num+1))
    done
  done

  # BHWide large angle samples
  t=1000     # equivalent time per job in [us] 
  Nusual=7  # number of jobs for 'usual'
  NECL=70    # number of jobs for 'ECL'
  
  for typ in usual ECL; do
    nnn=N${typ}
    njobs=$((${nnn}))
    num=0
    while [ $num -lt ${njobs} ]; do 
      logf=${logdir}/BHWideLargeAngle_${typ}-phase${phase}-${num}.out
      errf=${logdir}/BHWideLargeAngle_${typ}-phase${phase}-${num}.err
      job="generateRadBhabhas.py bhwide_largeangle ${t} ${num} ${typ} ${phase} ${outdir} ${vtx}"
      bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
      num=$(($num+1))
    done
  done

  # two photon samples
  t=100     # equivalent time per job in [us] 
  Nusual=6  # number of jobs for 'usual'
  NECL=6    # number of jobs for 'ECL'
  
  for typ in usual ECL; do
    nnn=N${typ}
    njobs=$((${nnn}))
    num=0
    while [ $num -lt ${njobs} ]; do 
      logf=${logdir}/twoPhoton_${typ}-phase${phase}-${num}.out
      errf=${logdir}/twoPhoton_${typ}-phase${phase}-${num}.err
      job="generateTwoPhoton.py ${t} ${num} ${typ} ${phase} ${outdir} ${vtx}"
      bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
      num=$(($num+1))
    done
  done
  
     
  echo "jobs for set ${setN} submitted"
  echo "- output files in ${outdir}"
  echo "- log files in ${logdir}"
  setN=$(($setN+1))
done

