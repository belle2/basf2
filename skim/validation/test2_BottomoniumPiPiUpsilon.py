#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../BottomoniumPiPiUpsilon.udst.root</input>
  <output>BottomoniumPiPiUpsilon_Validation.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdMu, stdPi
from beamparameters import add_beamparameters

# create a new path
BottomoniumPiPiUpsilonskimpath = b2.Path()

# set up for running at Y(3S)
beamparameters = add_beamparameters(BottomoniumPiPiUpsilonskimpath, "Y3S")

filelist = ['../BottomoniumPiPiUpsilon.udst.root']
ma.inputMdstList('default', filelist, path=BottomoniumPiPiUpsilonskimpath)

# use standard final state particle lists
stdMu('loose', path=BottomoniumPiPiUpsilonskimpath)
stdPi('all', path=BottomoniumPiPiUpsilonskimpath)

# [Y(3S) -> pi+pi- [Y(1S,2S) -> mu+mu-]] decay
ma.reconstructDecay('Upsilon:mumu -> mu+:loose mu-:loose', '', path=BottomoniumPiPiUpsilonskimpath)
ma.reconstructDecay('Upsilon(3S):pipirecoil -> pi+:all pi-:all', '', path=BottomoniumPiPiUpsilonskimpath)
ma.reconstructDecay('Upsilon(3S):12Smumu -> Upsilon(3S):pipirecoil Upsilon:mumu', '', path=BottomoniumPiPiUpsilonskimpath)

# the variables that are printed out are: the invariant mass of Y(1S,2S), the invariant mass
# of Y(3S), and the recoil mass of pi+pi-
ma.variablesToHistogram(
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

b2.process(BottomoniumPiPiUpsilonskimpath)
print(b2.statistics)
