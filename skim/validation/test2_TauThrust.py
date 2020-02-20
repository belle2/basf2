#!/usr/bin/env/python3
# -*-coding: utf-8-*-

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi
from stdPhotons import stdPhotons
from skim.taupair import SetTauThrustSkimVariables

"""
<header>
  <input>../TauThrust.udst.root</input>
  <output>TauThrust_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

tauthrustskim = b2.Path()

ma.inputMdst('default', '../TauThrust.udst.root', path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# set variables
SetTauThrustSkimVariables(path=tauthrustskim)

# the variables that are printed out are:
ma.variablesToHistogram(
    filename='TauThrust_Validation.root',
    decayString='',
    variables=[('nGoodTracksThrust', 7, 1, 8),
               ('visibleEnergyOfEventCMS', 60, 0, 12),
               ('thrust', 50, 0.75, 1)],
    path=tauthrustskim
)
b2.process(tauthrustskim)
print(b2.statistics)

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

import ROOT

f = ROOT.TFile.Open('TauThrust_Validation.root', 'update')

f.Get('nGoodTracksThrust').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('visibleEnergyOfEventCMS').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('thrust').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))

f.Write("", ROOT.TObject.kOverwrite)
f.Close()
