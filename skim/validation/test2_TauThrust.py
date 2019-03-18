#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *
from variables import variables
from stdCharged import *
from stdPhotons import *

"""
<header>
  <input>../TauThrust.udst.root</input>
  <output>../TauThrust_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

tauthrustskim = Path()

inputMdst('MC9', '../TauThrust.udst.root', path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# set variables
from skim.taupair import *
SetTauThrustSkimVariables(path=tauthrustskim)

# the variables that are printed out are:
variablesToHistogram(
    filename='TauThrust_Validation.root',
    decayString='',
    variables=[('nGoodTracks', 7, 1, 8),
               ('visibleEnergyOfEventCMS', 60, 0, 12),
               ('thrust', 60, 0.7, 1)],
    path=tauthrustskim
)
process(tauthrustskim)
print(statistics)
