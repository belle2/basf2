#!/usr/bin/env/python3
# -*-coding: utf-8-*-

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
from stdCharged import stdPi
from stdPhotons import stdPhotons
from skim.taupair import TauGeneric

"""
<header>
  <input>../TauGeneric.udst.root</input>
  <output>TauGeneric_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

taugenericskim = b2.Path()

ma.inputMdst('default', '../TauGeneric.udst.root', path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# set variables
TauGeneric().additional_setup(path=taugenericskim)

vm.addAlias('Theta_miss', 'formula(missingMomentumOfEvent_theta*180/3.14159)')

# the variables that are printed out are:
ma.variablesToHistogram(
    filename='TauGeneric_Validation.root',
    decayString='',
    variables=[('nGoodTracks', 7, 1, 8),
               ('visibleEnergyOfEventCMS', 40, 0, 12),
               ('E_ECLtrk', 70, 0, 7),
               ('maxPt', 30, 0, 6),
               ('invMS1', 60, 0, 3),
               ('invMS2', 60, 0, 3),
               ('Theta_miss', 30, 0, 180)],
    variables_2d=[('invMS1', 30, 0, 3, 'invMS2', 30, 0, 3)],
    path=taugenericskim
)
b2.process(taugenericskim)
print(b2.statistics)

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
