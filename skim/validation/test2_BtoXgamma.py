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

# the variables that are printed out are: Mbc, deltaE and the daughter particle invariant masses.

vm.addAlias("gamma_E_CMS", "useCMSFrame(E)")
vm.addAlias("gamma_E", 'daughter(1, E)')
vm.addAlias("Kst_M", 'daughter(0, InvM)')

create_validation_histograms(
    rootfile=histogram_filename,
    particlelist='B0:sig',
    variables_1d=[
        ('Mbc', 100, 5.2, 5.3, 'Signal B0 Mbc', my_email,
         'Mbc of the signal B0', '', 'Mbc [GeV/c^2]', 'Candidates'),
        ('deltaE', 100, -1, 1, 'Signal B0 deltaE', my_email,
         'deltaE of the signal B0', '', 'deltaE [GeV]', 'Candidates'),
        ('gamma_E', 100, 1.5, 4, 'Photon Energy', my_email,
         'Energy of photon (B0 daughter)', '', 'Photon E [GeV]', 'Candidates'),
        ('Kst_M', 100, 0.6, 1.6, 'Invariant Mass of K*0', my_email,
         'Invariant Mass of K*0 (B0 daughter)', '', 'K*0 InvM [GeV/c^2]', 'Candidates')
        ],
    variables_2d=[('Mbc', 50, 5.23, 5.31, 'deltaE', 50, -0.7, 0.7, 'Mbc and deltaE of Signal B0', my_email,
                   '2D plot of Mbc and deltaE for the Signal B0', '', 'Mbc [GeV/c^2]', 'deltaE [GeV]')],
    path=path)

create_validation_histograms(
    rootfile=histogram_filename,
    particlelist='gamma:sig',
    variables_1d=[
        ('gamma_E_CMS', 100, 1.5, 3.5, 'Photon Energy', my_email,
         'Energy of photon', '', 'Photon E [GeV]', 'Candidates'),
        ('nTracks', 15, 0, 15, 'Number of Tracks', my_email,
         'Number of Tracks in Event', '', 'Number of Tracks', 'Events')
        ],
    variables_2d=[],
    path=path)

b2.process(path)
print(b2.statistics)
