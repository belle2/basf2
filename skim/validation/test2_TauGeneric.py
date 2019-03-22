#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *
from variables import variables
from stdCharged import *
from stdPhotons import *

"""
<header>
  <input>../TauGeneric.udst.root</input>
  <output>TauGeneric_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

taugenericskim = Path()

inputMdst('MC9', '../TauGeneric.udst.root', path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# set variables
from skim.taupair import *
SetTauGenericSkimVariables(path=taugenericskim)

variables.addAlias('Theta_miss', 'formula(missingMomentumOfEvent_theta*180/3.14159)')

# the variables that are printed out are:
variablesToHistogram(
    filename='TauGeneric_Validation.root',
    decayString='',
    variables=[('nGoodTracks', 7, 1, 8),
               ('visibleEnergyOfEventCMS', 60, 0, 12),
               ('E_ECLtrk', 70, 0, 7),
               ('maxPt', 60, 0, 6),
               ('MinvS1', 60, 0, 3),
               ('MinvS2', 60, 0, 3),
               ('Theta_miss', 60, 0, 180)],
    variables_2d=[('MinvS1', 30, 0, 3, 'MinvS2', 30, 0, 3)],
    path=taugenericskim
)
process(taugenericskim)
print(statistics)
