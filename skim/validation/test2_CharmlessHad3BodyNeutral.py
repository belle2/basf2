#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../CharmlessHad3BodyNeutral.udst.root</input>
    <output>CharmlessHad3BodyNeutral_Validation.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
</header>
"""

from basf2 import *
from modularAnalysis import *
from variables import variables
from validation_tools.metadata import create_validation_histograms

charmless3neutralpath = Path()
myEmail = 'Kim Smith <khsmith@student.unimelb.edu.au>'

# the variables that are printed out are: Mbc, deltaE and Mbc vs deltaE
inputMdst('default', '../CharmlessHad3BodyNeutral.udst.root', path=charmless3neutralpath)

create_validation_histograms(
    rootfile='CharmlessHad3BodyNeutral_Validation.root',
    particlelist='B0:3BodySkim',
    variables_1d=[(
        'deltaE', 100, -1, 1,
        '#Delta E',
        myEmail,
        '$\\Delta E$ of event',
        'Peak around zero',
        '#Delta E [GeV]', 'Candidates',
        'shifter'
    ), (
        'Mbc', 100, 5.2, 5.3,
        'Mbc',
        myEmail,
        'Beam-constrained mass of event',
        'Peaking around B mass (5.28 GeV)',
        'M_{bc} [GeV]', 'Candidates',
        'shifter'
    )],
    variables_2d=[(
        'deltaE', 50, -0.7, 0.7,
        'Mbc', 50, 5.23, 5.31,
        'Mbc vs deltaE',
        myEmail,
        'Plot of the $\\Delta E$ of the event against the beam constrained mass',
        'Peak of $\\Delta E$ around zero, and $M_{bc}$ around B mass (5.28 GeV)',
        '#Delta E [GeV]', 'M_{bc} [GeV]',
        'colz, shifter'
    )],
    path=charmless3neutralpath
)

process(charmless3neutralpath)
print(statistics)
