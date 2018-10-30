#!/bin/bash

# this is an expample script demonstrating on how to submit a series of jobs for different random seeds to the queue at KEK
# NOTE: you HAVE to adjust the numbers below otherwise you will generate 2000k events!!

# list of random seeds for which events should be generated (adjust to your needs)
# for each of these seeds nevt events will be generated
# the sequence of the 40 first seeds I took from my "head" the second sequence of 40 numbers
# was generated with this shell command: for i in `seq 1 40`; do echo $RANDOM; done
rndseeds=(321 66718 12345 54321 312455 7897 90549 21352   119992 98662 15521 8876  98976 6575  1124   33412 1341   9973  9087  2452
          977 67582 76528 5562  88727  6879 1767  9875762 97569  15752 76519 97792 65757 13332 754762 8666  132411 21341 34411 1321
          13709 29094 1319 28865 20363 24046 28268 8875 10452 2371 3867 22832 4052 17901 1397 22989 23889 6280  5079 14206  23738
          24192 9881  10480 21755 15912 26724  528 3651 24222 5063 29354 6673 30708 17755 26302 27499 29168 23817   45)

# the name of the output directory where the root files will be stored
outdir=./datadir/
# number of events generated per random seed (at around 56000 events the cpu limit is reached)
nevt=25000
# the name of the queue at KEK
queue=s

# loop over all random seeds
for seed in ${rndseeds[@]} 
do
  echo $seed
  bsub -q $queue -o ${outdir}log${seed}.log  basf2 eventSimulation.py $seed $outdir -n $nevt
done
