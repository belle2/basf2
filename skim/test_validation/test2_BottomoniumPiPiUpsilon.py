#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu
from beamparameters import add_beamparameters

# create a new path
BottomoniumPiPiUpsilonskimpath = Path()

# set up for running at Y(3S)
beamparameters = add_beamparameters(BottomoniumPiPiUpsilonskimpath, "Y3S")

filelist = ['BottomoniumPiPiUpsilon.udst.root']
inputMdstList('default', filelist, path=BottomoniumPiPiUpsilonskimpath)

# use standard final state particle lists
stdMu('loose', path=BottomoniumPiPiUpsilonskimpath)
stdPi('loose', path=BottomoniumPiPiUpsilonskimpath)

# [Y(3S) -> pi+pi- [Y(1S,2S) -> mu+mu-]] decay
reconstructDecay('Upsilon:mumu -> mu+:loose mu-:loose', '', path=BottomoniumPiPiUpsilonskimpath)
reconstructDecay('Upsilon(3S):pipirecoil -> pi+:loose pi-:loose', '', path=BottomoniumPiPiUpsilonskimpath)
reconstructDecay('Upsilon(3S):12Smumu -> Upsilon(3S):pipirecoil Upsilon:mumu', '', path=BottomoniumPiPiUpsilonskimpath)

from variables import variables
variablesToHistogram(
    filename='BottomoniumPiPiUpsilon_Validation.root',
    decayString='Upsilon(3S):12Smumu',
    variables=[
        ('daughterInvariantMass(1)',
         70,
         9.0,
         10.4),
        ('InvM',
         70,
         10.0,
         10.7),
        ('daughter(0,mRecoil)',
         50,
         9.2,
         10.2)
    ], path=BottomoniumPiPiUpsilonskimpath)

process(BottomoniumPiPiUpsilonskimpath)
print(statistics)
