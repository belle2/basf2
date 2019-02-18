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
  <output>../TauGeneric_Validation.root</output>
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

variables.addAlias('nGoodGammas', 'nParticlesInList(gamma:all)')
variables.addAlias('Theta_miss', 'formula(missingMomentumOfEvent_theta*180/3.14159)')
variables.addAlias('Pmiss', 'missingMomentumOfEvent')
variables.addAlias('M2miss', 'missingMass2OfEvent')

# the variables that are printed out are:
variablesToHistogram(
    filename='../TauGeneric_Validation.root',
    decayString='',
    variables=[('nGoodTracks', 7, 1, 8),
               ('nGoodGammas', 15, 0, 15),
               ('thrust', 60, 0.7, 1)],
    variables_2d=[('Theta_miss', 30, 0, 180, 'Pmiss', 30, 0, 6),
                  ('Theta_miss', 30, 0, 180, 'M2miss', 30, -20, 100),
                  ('E_S1', 30, 0, 6, 'MinvS1', 20, 0, 2),
                  ('E_S2', 30, 0, 6, 'MinvS2', 20, 0, 2),
                  ('E_ECL', 30, 0, 12, 'Etot', 30, 0, 12)],
    path=taugenericskim
)
process(taugenericskim)
print(statistics)
