#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../feiHadronicB0.udst.root</input>
    <output>feiHadronicB0_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables

path = Path()

inputMdst('default', '../feiHadronicB0.udst.root', path=path)

variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
variables.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
variables.addAlias('d0_M', 'daughter(0,M)')
variables.addAlias('decayModeID', 'extraInfo(decayModeID)')
variables.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

histogramFilename = 'feiHadronicB0_Validation.root'

variablesToHistogram(
    filename=histogramFilename,
    decayString='B0:generic',
    variables=[
        ('sigProb', 100, 0.0, 1.0),
        ('nDaug', 6, 0.0, 6),
        ('d0_massDiff', 100, 0.0, 0.5),
        ('d0_M', 100, 0.0, 3.0),
        ('deltaE', 100, -0.2, 0.2),
        ('Mbc', 100, 5.2, 5.3)],
    variables_2d=[('deltaE', 100, -0.2, 0.2, 'Mbc', 100, 5.2, 5.3),
                  ('decayModeID', 26, 0, 26, 'log10_sigProb', 100, -3.0, 0.0)],
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
