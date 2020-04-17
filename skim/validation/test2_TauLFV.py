#!/usr/bin/env/python3
# -*-coding: utf-8-*-

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons

"""
<header>
  <input>../TauLFV.udst.root</input>
  <output>TauLFV_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

taulfvskim = b2.Path()

ma.inputMdst('default', '../TauLFV.udst.root', path=taulfvskim)

stdPi('loose', path=taulfvskim)
stdK('loose', path=taulfvskim)
stdPr('loose', path=taulfvskim)
stdE('loose', path=taulfvskim)
stdMu('loose', path=taulfvskim)
stdPhotons('loose', path=taulfvskim)
stdPi0s('eff40_Jan2020', path=taulfvskim)
loadStdSkimPi0(path=taulfvskim)
stdKshorts(path=taulfvskim)
loadStdLightMesons(path=taulfvskim)

from skim.taupair import TauLFVList
tauList = TauLFVList(0, path=taulfvskim)
ma.copyLists('tau+:LFV', tauList, path=taulfvskim)

# the variables that are printed out are: M, deltaE
ma.variablesToHistogram(
    filename='TauLFV_Validation.root',
    decayString='tau+:LFV',
    variables=[('M', 100, 1.50, 2.00), ('deltaE', 120, -1.1, 1.1)],
    variables_2d=[('M', 50, 1.50, 2.00, 'deltaE', 60, -1.1, 1.1)],
    path=taulfvskim
)
b2.process(taulfvskim)
print(b2.statistics)

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

import ROOT

f = ROOT.TFile.Open('TauLFV_Validation.root', 'update')

f.Get('M').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('deltaE').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
f.Get('MdeltaE').GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))

f.Write("", ROOT.TObject.kOverwrite)
f.Close()
