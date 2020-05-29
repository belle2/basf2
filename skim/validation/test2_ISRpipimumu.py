#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../ISRpipimumu.udst.root</input>
  <output>ISRpipimumu_Validation.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdMu, stdPi

# create a new path
ISRskimpath = b2.Path()

filelist = ['../ISRpipimumu.udst.root']
ma.inputMdstList('default', filelist, path=ISRskimpath)

# use standard final state particle lists
stdMu('95eff', path=ISRskimpath)
stdPi('95eff', path=ISRskimpath)
# [ee -> ISR pi+pi- [J/psi -> mu+mu-]] decay
ma.reconstructDecay('J/psi:mumu_validation -> mu+:95eff mu-:95eff', '2.9 < M < 3.3', path=ISRskimpath)
ma.reconstructDecay('vpho:myCombinations -> J/psi:mumu_validation pi+:95eff pi-:95eff', '', path=ISRskimpath)

ma.variablesToHistogram(
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
b2.process(ISRskimpath)
print(b2.statistics)
