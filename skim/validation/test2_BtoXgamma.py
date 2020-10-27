#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>12160100.udst.root</input>
    <output>BtoXgamma_Validation.root</output>
    <contact>simon.wehle@desy.de</contact>
</header>
"""
__author__ = "Simon Wehle, Trevor Shillington"

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPhotons import stdPhotons
from variables import variables as vm

from validation_tools.metadata import create_validation_histograms
path = b2.Path()

histogram_filename = 'BtoXgamma_Validation.root'
my_email = 'Trevor Shillington <trshillington@hep.physics.mcgill.ca>'

ma.inputMdst('default', '12160100.udst.root', path=path)

stdPi('all', path=path)
stdK('all', path=path)
stdPhotons('cdc', path=path)
ma.cutAndCopyList('gamma:sig', 'gamma:cdc', 'clusterNHits > 1.5 and E > 1.5', True, path)

ma.reconstructDecay('K*0:sig  -> K+:all pi-:all', '0.6 < M < 1.6', path=path)
ma.reconstructDecay('B0:sig ->  K*0:sig gamma:sig', '5.22 < Mbc < 5.3 and  abs(deltaE)< .5', path=path)

# the variables that are printed out are: Mbc and deltaE

create_validation_histograms(
    rootfile=histogram_filename,
    particlelist='B0:sig',
    variables_1d=[
        ('Mbc', 100, 5.2, 5.3, 'Signal B0 Mbc', my_email,
         'Mbc of the signal B0', '', 'Mbc [GeV/c^2]', 'Candidates'),
        ('deltaE', 100, -1, 1, 'Signal B0 deltaE', my_email,
         'deltaE of the signal B0', '', 'deltaE [GeV]', 'Candidates')
        ],
    variables_2d=[],
    path=path)


b2.process(path)
print(b2.statistics)
