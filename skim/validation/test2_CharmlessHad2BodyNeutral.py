#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>../CharmlessHad2BodyNeutral.udst.root</input>
    <output>CharmlessHad2BodyNeutral_Validation.root</output>
    <contact>khsmith@student.unimelb.edu.au</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from validation_tools.metadata import create_validation_histograms

charmless2neutralpath = b2.Path()
myEmail = 'khsmith@student.unimelb.edu.au'

# the variables that are printed out are: Mbc, deltaE and Mbc vs deltaE
ma.inputMdst('default', '../CharmlessHad2BodyNeutral.udst.root', path=charmless2neutralpath)

create_validation_histograms(
    rootfile='CharmlessHad2BodyNeutral_Validation.root',
    particlelist='B0:2BodySkim',
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
    path=charmless2neutralpath
)

b2.process(charmless2neutralpath)
print(b2.statistics)
