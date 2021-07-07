#!/bin/bash

# job submission script for l queue 
# generate overlay samples

phase=3     # nominal phase 3
scale=1      # BG scaling factor
nsets=1      # number of mixing sets
nfiles=2    # number of overlay files per mixing set, default 250
nevents=10000 # number of events per overlay file
vtx=VTX-CMOS-7layer


setN=0    # set counter
while [ $setN -lt ${nsets} ]; do 
    export BELLE2_BACKGROUND_MIXING_DIR=./upgrade/mixing/set${setN}
    echo ''
    echo 'mixing samples: '$BELLE2_BACKGROUND_MIXING_DIR

    outdir=./upgrade/overlay/phase${phase}/BGx${scale}/set${setN} # generated samples
    logdir=log-upgrade-ph${phase}-overlay/BGx${scale}/set${setN}    # log files

    mkdirhier ${outdir}
    mkdirhier ${logdir}

    echo 'output to: '${outdir}

    num=0  # file (job) counter
    while [ $num -lt ${nfiles} ]; do 
	logf=${logdir}/overlay-${num}.out
	errf=${logdir}/overlay-${num}.err
	outfile=${outdir}/BGforOverlay-${num}.root
	job="makeBGOverlayFile.py phase${phase} ${scale} ${vtx}  -o ${outfile} -n ${nevents}"
	echo bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
        bsub -q l -o ${logf} -e ${errf} "basf2 ${job}"
	num=$(($num+1))
    done

    setN=$(($setN+1))
done

