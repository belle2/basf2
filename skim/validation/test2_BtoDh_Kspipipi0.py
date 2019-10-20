#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>14120400.udst.root</input>
    <output>BtoDh_Kspipipi0_Validation.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import loadStdSkimPi0
from stdV0s import *
from stdPi0s import stdPi0s

kspipipi0Path = Path()

inputMdst('default', '14120400.udst.root', path=kspipipi0Path)

stdPi('all', path=kspipipi0Path)
stdK('all', path=kspipipi0Path)
loadStdSkimPi0(path=kspipipi0Path)
stdKshorts(path=kspipipi0Path)
stdPi0s(listtype='looseFit', path=kspipipi0Path)

reconstructDecay('D0 -> K_S0:merged pi-:all pi+:all pi0:looseFit', '1.84 < M < 1.89', path=kspipipi0Path)
reconstructDecay('B-:ch3 ->D0 K-:all', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=kspipipi0Path)

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

from variables import variables
variablesToHistogram(
    filename='BtoDh_Kspipipi0_Validation.root',
    decayString='B-:ch3',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 1.8, 1.9)],  # D0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)], path=kspipipi0Path)

process(kspipipi0Path)
print(statistics)
