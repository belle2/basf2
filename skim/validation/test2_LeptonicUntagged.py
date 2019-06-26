#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../LeptonicUntagged.udst.root</input>
    <output>LeptonicUntagged_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../LeptonicUntagged.udst.root', path=path)

cutAndCopyLists('B-:all', ['B-:L0', 'B-:L1'], '', path=path)

buildRestOfEvent('B-:all', path=path)
appendROEMask('B-:all', 'basic',
              'pt>0.05 and -2<dr<2 and -4.0<dz<4.0',
              'E>0.05',
              path=path)
buildContinuumSuppression('B-:all', 'basic', path=path)

variables.addAlias('d0_p', 'daughter(0,p)')
variables.addAlias('d0_electronID', 'daughter(0,electronID)')
variables.addAlias('d0_muonID', 'daughter(0,muonID)')
variables.addAlias('MissP', 'WE_MissP(basic,0)')

histogramFilename = 'LeptonicUntagged_Validation.root'

variablesToHistogram(
    filename=histogramFilename,
    decayString='B-:all',
    variables=[
        ('Mbc', 100, 4.0, 5.3),
        ('d0_p', 100, 0, 5.2),  # Lepton momentum
        ('d0_electronID', 100, 0, 1),
        ('d0_muonID', 100, 0, 1),
        ('R2', 100, 0, 1),
        ('MissP', 100, 0, 5.3)
    ],
    variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3)],
    path=path)

process(path)
print(statistics)

# Reopen file to add contact details
import ROOT

histfile = ROOT.TFile(histogramFilename, 'UPDATE')

histnames = [histname.GetTitle() for histname in histfile.GetListOfKeys()]
for histname in histnames:
    hist = histfile.Get(histname)

    hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'philip.grace@adelaide.edu.au'))

    hist.Write('', ROOT.TObject.kOverwrite)
histfile.Close()
