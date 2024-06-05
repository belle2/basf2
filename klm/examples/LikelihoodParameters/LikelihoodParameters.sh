#!/bin/bash
# This file provides an example of how to call KLM time calibration examples.
for p in muon pion kaon proton electron deuteron ; do
  for sign in "+" "-" ; do
    for ((str = 0; str<250; ++str)); do
      bsub -ql ParticleGun.py $p$sign 10000 $str
    done
  done
done
