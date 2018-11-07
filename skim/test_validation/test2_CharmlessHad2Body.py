#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

# the variables that are printed out are: Mbc, deltaE.
inputMdst('default', 'CharmlessHad2Body.udst.root')
from variables import variables
variablesToHistogram(
    filename='CharmlessHad2Body_Validation.root',
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
         0,
         1.5),
        ('daughter(0, p)',  # K_S0 momentum
         100,
         0,
         4),
        ('daughter(1, InvM)',  # pi0 invariant mass
         100,
         0,
         1),
        ('daughter(1, p)',  # pi0 momentum
         100,
         0,
         4)])
process(analysis_main)
print(statistics)
