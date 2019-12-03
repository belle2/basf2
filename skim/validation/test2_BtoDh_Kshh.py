#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>14140200.udst.root</input>
    <output>BtoDh_Kshh_Validation.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdV0s import stdKshorts

kshhPath = b2.Path()

ma.inputMdst('default', '14140200.udst.root', path=kshhPath)

stdPi('all', path=kshhPath)
stdK('all', path=kshhPath)
stdKshorts(path=kshhPath)

ma.reconstructDecay('D0 -> K_S0:merged pi+:all pi-:all', '1.84 < M < 1.89', path=kshhPath)
ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=kshhPath)

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

ma.variablesToHistogram(
    filename='BtoDh_Kshh_Validation.root',
    decayString='B-:ch3',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=kshhPath)

b2.process(kshhPath)
print(b2.statistics)
