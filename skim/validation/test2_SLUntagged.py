#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../SLUntagged.udst.root</input>
    <output>SLUntagged_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../SLUntagged.udst.root', path=path)

cutAndCopyLists('B+:all', ['B+:SL0', 'B+:SL1', 'B+:SL2', 'B+:SL3'], '', path=path)

buildRestOfEvent('B+:all', path=path)
appendROEMask('B+:all', 'basic',
              'pt>0.05 and -2<dr<2 and -4.0<dz<4.0',
              'E>0.05',
              path=path)
buildContinuumSuppression('B+:all', 'basic', path=path)

variables.addAlias('d1_p', 'daughter(1,p)')
variables.addAlias('MissM2', 'WE_MissM2(basic,0)')

histogramFilename = 'SLUntagged_Validation.root'

variablesToHistogram(
    filename=histogramFilename,
    decayString='B+:all',
    variables=[
        ('cosThetaBetweenParticleAndNominalB', 100, -6.0, 4.0),
        ('Mbc', 100, 4.0, 5.3),
        ('d1_p', 100, 0, 5.2),  # Lepton momentum
        ('MissM2', 100, -5, 5)
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
