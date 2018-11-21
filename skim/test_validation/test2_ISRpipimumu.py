#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu

filelist = ['ISRpipimumu.udst.root']
inputMdstList('default', filelist)

# use standard final state particle lists
stdMu('95eff')
stdPi('95eff')
# [ee -> ISR pi+pi- [J/psi -> mu+mu-]] decay
reconstructDecay('J/psi:mumu -> mu+:95eff mu-:95eff', '2.9 < M < 3.3')
reconstructDecay('vpho:myCombinations -> J/psi:mumu pi+:95eff pi-:95eff', '')

from variables import variables
variablesToHistogram(
    filename='ISRpipimumu_Validation.root',
    decayString='vpho:myCombinations',
    variables=[
        ('daughterInvariantMass(0)',
         80,
         2.9,
         3.3),
        ('useCMSFrame(cosTheta)',
         50,
         -1,
         1),
        ('m2Recoil',
         50,
         -1,
         1)
    ])
process(analysis_main)
print(statistics)
