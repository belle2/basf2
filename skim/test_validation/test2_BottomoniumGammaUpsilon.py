#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu
from stdPhotons import *
from beamparameters import add_beamparameters

# create a new path
BottomoniumGammaUpsilonskimpath = Path()

# set up for running at Y(3S)
beamparameters = add_beamparameters(BottomoniumGammaUpsilonskimpath, "Y3S")

filelist = ['BottomoniumGammaUpsilon.udst.root']
inputMdstList('default', filelist, path=BottomoniumGammaUpsilonskimpath)

# use standard final state particle lists
stdMu('loose', path=BottomoniumGammaUpsilonskimpath)
stdPi('loose', path=BottomoniumGammaUpsilonskimpath)

stdPhotons('loose', path=BottomoniumGammaUpsilonskimpath)

# Upsilon3S -> gam chib -> gam Y(1S,2S)(ll) decay
reconstructDecay('Upsilon:mumu -> mu+:loose mu-:loose', '', path=BottomoniumGammaUpsilonskimpath)
reconstructDecay('chi_b1(2P):mychibJ -> gamma:loose Upsilon:mumu', '', path=BottomoniumGammaUpsilonskimpath)
reconstructDecay('Upsilon(3S) -> gamma:loose chi_b1(2P):mychibJ', '', path=BottomoniumGammaUpsilonskimpath)

from variables import variables
variablesToHistogram(
    filename='BottomoniumGammaUpsilon_Validation.root',
    decayString='Upsilon(3S)',
    variables=[
        ('daughterInvariantMass(1)',
         80,
         9,
         10.6),
        ('daughter(1, daughter(1, M))',
         60,
         9.2,
         10.4),
        ('M',
         50,
         10,
         10.7)
    ], path=BottomoniumGammaUpsilonskimpath)

process(BottomoniumGammaUpsilonskimpath)
print(statistics)
