#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
inputMdst('default', 'CharmlessHad3BodyCharged.udst.root')
from variables import variables
variablesToHistogram(
    filename='CharmlessHad3BodyCharged_Validation.root',
    decayString='B+:3BodySkim',
    variables=[
        ('Mbc',
         100,
         5.2,
         5.3),
        ('deltaE',
         100,
         -1,
         1),
        ('daughter(0, InvM)',  # K_S0 invariant mass
         100,
         0.3,
         1),
        ('daughter(1, InvM)',  # pi+ invariant mass
         100,
         0.06,
         0.18),
        ('daughter(2, InvM)',  # pi0 invariant mass
         100,
         0.06,
         0.18)],
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)])
process(analysis_main)
print(statistics)
