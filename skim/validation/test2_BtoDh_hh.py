#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>14140100.udst.root</input>
    <output>BtoDh_hh_Validation.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK

hhpath = Path()

inputMdst('default', '14140100.udst.root', path=hhpath)

stdPi('all', path=hhpath)
stdK('all', path=hhpath)


reconstructDecay('D0 -> K-:all pi+:all', '1.84 < M < 1.89', path=hhpath)
reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=hhpath)

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

from variables import variables
variablesToHistogram(
    filename='BtoDh_hh_Validation.root',
    decayString='B-:ch3',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=hhpath)

process(hhpath)
print(statistics)
