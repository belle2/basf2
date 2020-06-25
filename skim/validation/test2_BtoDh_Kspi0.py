#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>14120300.udst.root</input>
    <output>BtoDh_Kspi0_Validation.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPi0s import loadStdSkimPi0
from stdV0s import stdKshorts
from stdPi0s import stdPi0s

kspi0Path = b2.Path()

ma.inputMdst('default', '14120300.udst.root', path=kspi0Path)

stdPi('all', path=kspi0Path)
stdK('all', path=kspi0Path)
loadStdSkimPi0(path=kspi0Path)
stdKshorts(path=kspi0Path)
stdPi0s(listtype='eff50_Jan2020Fit', path=kspi0Path)

ma.reconstructDecay('D0 -> K_S0:merged pi0:eff50_Jan2020Fit', '1.84 < M < 1.89', path=kspi0Path)
ma.reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=kspi0Path)

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

ma.variablesToHistogram(
    filename='BtoDh_Kspi0_Validation.root',
    decayString='B-:ch3',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=kspi0Path)

b2.process(kspi0Path)
print(b2.statistics)
