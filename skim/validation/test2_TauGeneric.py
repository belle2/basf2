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

inputMdst('default', '../TauGeneric.udst.root', path=taugenericskim)

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
               ('invMS1', 60, 0, 3),
               ('invMS2', 60, 0, 3),
               ('Theta_miss', 60, 0, 180)],
    variables_2d=[('invMS1', 30, 0, 3, 'invMS2', 30, 0, 3)],
    path=taugenericskim
)
process(taugenericskim)
print(statistics)

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

import ROOT

f = ROOT.TFile.Open('TauGeneric_Validation.root', 'update')

f.Get('nGoodTracks').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('visibleEnergyOfEventCMS').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('E_ECLtrk').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('maxPt').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('invMS1').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('invMS2').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('Theta_miss').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('invMS1invMS2').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))

f.Write("", ROOT.TObject.kOverwrite)
f.Close()
