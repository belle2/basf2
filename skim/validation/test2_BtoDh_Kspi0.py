#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>14120300.udst.root</input>
    <output>BtoDh_Kspi0_Validation.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import loadStdSkimPi0
from stdV0s import *
from stdPi0s import stdPi0s

kspi0Path = Path()

inputMdst('default', '14120300.udst.root', path=kspi0Path)

stdPi('all', path=kspi0Path)
stdK('all', path=kspi0Path)
loadStdSkimPi0(path=kspi0Path)
stdKshorts(path=kspi0Path)
stdPi0s(listtype='veryLooseFit', path=kspi0Path)

reconstructDecay('D0 -> K_S0:merged pi0:veryLooseFit', '1.84 < M < 1.89', path=kspi0Path)
reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=kspi0Path)

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

from variables import variables
variablesToHistogram(
    filename='BtoDh_Kspi0_Validation.root',
    decayString='B-:ch3',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=kspi0Path)

process(kspi0Path)
print(statistics)
