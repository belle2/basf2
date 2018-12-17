#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>CharmlessHad2BodyCharged.udst.root</input>
    <output>CharmlessHad2BodyCharged_Validation.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *

charmless2chargedpath = Path()

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.
inputMdst('MC9', 'CharmlessHad2BodyCharged.udst.root', path=charmless2chargedpath)

from variables import variables
variablesToHistogram(
    filename='CharmlessHad2BodyCharged_Validation.root',
    decayString='B-:2BodyBm',
    variables=[
        ('Mbc', 100, 5.2, 5.3),
        ('deltaE', 100, -1, 1),
        ('daughter(0, InvM)', 100, 0.5, 1.2),  # K*+ invariant mass
        ('daughter(1, InvM)', 100, 0.5, 1.2)],  # rho0 invariant mass
    variables_2d=[
        ('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7)])

process(charmless2chargedpath)
print(statistics)
