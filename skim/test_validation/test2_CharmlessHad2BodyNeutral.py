#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
inputMdst('default', 'CharmlessHad2BodyNeutral.udst.root')
from variables import variables
variablesToHistogram(
    filename='CharmlessHad2BodyNeutral_Validation.root',
    decayString='B0:2BodyB0',
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
         1.1),
        ('daughter(1, InvM)',  # pi0 invariant mass
         100,
         0.06,
         0.18)],
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)])
process(analysis_main)
print(statistics)
