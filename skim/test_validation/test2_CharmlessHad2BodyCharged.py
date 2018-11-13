#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
inputMdst('default', 'CharmlessHad2BodyCharged.udst.root')
from variables import variables
variablesToHistogram(
    filename='CharmlessHad2BodyCharged_Validation.root',
    decayString='B-:2BodyBm',
    variables=[
        ('Mbc',
         100,
         5.2,
         5.3),
        ('deltaE',
         100,
         -1,
         1),
        ('daughter(0, InvM)',  # K*+ invariant mass
         100,
         0.5,
         1.2),
        ('daughter(1, InvM)',  # rho0 invariant mass
         100,
         0.5,
         1.2)],
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)])
process(analysis_main)
print(statistics)
