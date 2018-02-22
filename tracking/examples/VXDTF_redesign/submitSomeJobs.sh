#!/bin/bash

# this is an expample script demonstrating on how to submit a series of jobs for different random seeds to the queue at KEK
# NOTE: you HAVE to adjust the numbers below otherwise you will generate 900k events!!

# list of random seeds for which events should be generated (adjust to your needs)
# for each of these seeds nevt events will be generated
rndseeds=(321 66718 12345 54321 312455 7897 90549 21352   119992 98662 15521 8876  98976 6575  1124   33412 1341   9973  9087  2452
          977 67582 76528) 
         # 5562  88727  6879 1767  9875762 97569  15752 76519 97792 65757 13332 754762 8666  132411 21341 34411 1321)
# the name of the output directory where the root files will be stored
outdir=./datadir/
# number of events generated per random seed (at around 56000 events the cpu limit is reached)
# 22.02.2018: further reduced the number of generated events to 40k as now a fit is included which increases the CPU time
nevt=40000
# the name of the queue at KEK
queue=s

# loop over all random seeds
for seed in ${rndseeds[@]} 
do
  echo $seed
  bsub -q $queue -o ${outdir}log${seed}.log  basf2 eventSimulation.py $seed $outdir -n $nevt
done
