#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

charmless3chargedpath = Path()

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
inputMdst('MC9', 'CharmlessHad3BodyCharged.udst.root', path=charmless3chargedpath)

from variables import variables
variablesToHistogram(
    filename='CharmlessHad3BodyCharged_Validation.root',
    decayString='B+:3BodySkim',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 0.3, 1),  # K_S0 invariant mass
        ('daughter(1, InvM)', 100, 0.06, 0.18),  # pi+ invariant mass
        ('daughter(2, InvM)', 100, 0.06, 0.18)],  # pi0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)])

process(charmless3chargedpath)
print(statistics)
