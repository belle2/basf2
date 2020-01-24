#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../BottomoniumGammaUpsilon.udst.root</input>
  <output>BottomoniumGammaUpsilon_Validation.root</output>
  <contact>jiasen@buaa.edu.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdMu, stdPi
from stdPhotons import stdPhotons
from beamparameters import add_beamparameters

# create a new path
BottomoniumGammaUpsilonskimpath = b2.Path()

# set up for running at Y(3S)
beamparameters = add_beamparameters(BottomoniumGammaUpsilonskimpath, "Y3S")

filelist = ['../BottomoniumGammaUpsilon.udst.root']
ma.inputMdstList('default', filelist, path=BottomoniumGammaUpsilonskimpath)

# use standard final state particle lists
stdMu('loose', path=BottomoniumGammaUpsilonskimpath)
stdPi('loose', path=BottomoniumGammaUpsilonskimpath)

stdPhotons('loose', path=BottomoniumGammaUpsilonskimpath)

# Upsilon3S -> gam chib -> gam Y(1S,2S)(ll) decay
ma.reconstructDecay('Upsilon:mumu -> mu+:loose mu-:loose', '', path=BottomoniumGammaUpsilonskimpath)
ma.reconstructDecay('chi_b1(2P):mychibJ -> gamma:loose Upsilon:mumu', '', path=BottomoniumGammaUpsilonskimpath)
ma.reconstructDecay('Upsilon(3S) -> gamma:loose chi_b1(2P):mychibJ', '', path=BottomoniumGammaUpsilonskimpath)

# the variables that are printed out are: the invariant mass of chib, the invariant mass
# of Y(1S,2S), and the invariant mass of Y(3S)
ma.variablesToHistogram(
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

b2.process(BottomoniumGammaUpsilonskimpath)
print(b2.statistics)
