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
  <output>TauThrust_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

tauthrustskim = Path()

inputMdst('default', '../TauThrust.udst.root', path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# set variables
from skim.taupair import *
SetTauThrustSkimVariables(path=tauthrustskim)

# the variables that are printed out are:
variablesToHistogram(
    filename='TauThrust_Validation.root',
    decayString='',
    variables=[('nGoodTracksThrust', 7, 1, 8),
               ('visibleEnergyOfEventCMS', 60, 0, 12),
               ('thrust', 50, 0.75, 1)],
    path=tauthrustskim
)
process(tauthrustskim)
print(statistics)

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

import ROOT

f = ROOT.TFile.Open('TauThrust_Validation.root', 'update')

f.Get('nGoodTracksThrust').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('visibleEnergyOfEventCMS').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('thrust').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))

f.Write("", ROOT.TObject.kOverwrite)
f.Close()
