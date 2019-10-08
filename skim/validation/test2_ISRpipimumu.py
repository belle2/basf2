#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../ISRpipimumu.udst.root</input>
  <output>ISRpipimumu_Validation.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu

# create a new path
ISRskimpath = Path()

filelist = ['../ISRpipimumu.udst.root']
inputMdstList('default', filelist, path=ISRskimpath)

# use standard final state particle lists
stdMu('95eff', path=ISRskimpath)
stdPi('95eff', path=ISRskimpath)
# [ee -> ISR pi+pi- [J/psi -> mu+mu-]] decay
reconstructDecay('J/psi:mumu -> mu+:95eff mu-:95eff', '2.9 < M < 3.3', path=ISRskimpath)
reconstructDecay('vpho:myCombinations -> J/psi:mumu pi+:95eff pi-:95eff', '', path=ISRskimpath)

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
    ], path=ISRskimpath)
process(ISRskimpath)
print(statistics)
