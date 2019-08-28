#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../feiSLBplus.udst.root</input>
    <output>feiSLBplus_Validation.root</output>
    <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables
from validation_tools.metadata import create_validation_histograms

path = Path()

inputMdst('default', '../feiSLBplus.udst.root', path=path)

variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
variables.addAlias('log10_sigProb', 'log10(extraInfo(SignalProbability))')
variables.addAlias('d0_massDiff', 'daughter(0,massDifference(0))')
variables.addAlias('d0_M', 'daughter(0,M)')
variables.addAlias('decayModeID', 'extraInfo(decayModeID)')
variables.addAlias('nDaug', 'countDaughters(1>0)')  # Dummy cut so all daughters are selected.

histogramFilename = 'feiSLBplus_Validation.root'
myEmail = 'Phil Grace <philip.grace@adelaide.edu.au>'

create_validation_histograms(
    rootfile=histogramFilename,
    particlelist='B+:semileptonic',
    variables_1d=[
        ('sigProb', 100, 0.0, 1.0, 'Signal probability', myEmail, '', ''),
        ('nDaug', 6, 0.0, 6, 'Number of daughters of Btag', myEmail, '', ''),
        ('cosThetaBetweenParticleAndNominalB', 100, -6.0, 4.0, 'cosThetaBY', myEmail, '', ''),
        ('d0_massDiff', 100, 0.0, 0.5, 'Mass difference of D* and D', myEmail, '', ''),
        ('d0_M', 100, 0.0, 3.0, 'Mass of zeroth daughter (D* or D)', myEmail, '', '')],
    variables_2d=[('decayModeID', 8, 0, 8, 'log10_sigProb', 100, -3.0, 0.0,
                   'Signal probability for each decay mode ID', myEmail, '', '')],
    path=path)

process(path)
print(statistics)
