#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
    <input>12160400.udst.root</input>
    <output>inclusiveBplusToKplusNuNu_Validation.root</output>
    <contact>cyrille.praz@desy.de</contact>
</header>
"""
__author__ = "Cyrille Praz"

import basf2 as b2
import modularAnalysis as ma
from variables import variables
from validation_tools.metadata import create_validation_histograms

histogram_filename = 'inclusiveBplusToKplusNuNu_Validation.root'


my_email = 'Cyrille Praz <cyrille.praz@desy.de>'

path = b2.Path()

ma.inputMdst('default', '12160400.udst.root', path=path)

# Build the event sphericity
ma.buildEventShape(inputListNames=[],
                   default_cleanup=True,
                   allMoments=False,
                   cleoCones=False,
                   collisionAxis=False,
                   foxWolfram=False,
                   harmonicMoments=False,
                   jets=False,
                   sphericity=True,
                   thrust=False,
                   checkForDuplicates=False,
                   path=path)

# Default cleanup also used in and ma.buildEventShape
track_cleanup = 'pt > 0.1'
track_cleanup += ' and thetaInCDCAcceptance'
track_cleanup += ' and abs(dz) < 3.0'
track_cleanup += ' and abs(dr) < 0.5'

# Define a couple of aliases
variables.addAlias('kaon_pt', 'daughter(0,pt)')
variables.addAlias('nCleanedTracks_simple_cleanup', 'nCleanedTracks({})'.format(track_cleanup))

# Output validation histograms
create_validation_histograms(
    rootfile=histogram_filename,
    particlelist='B+:inclusiveBplusToKplusNuNu',
    variables_1d=[
        ('kaon_pt',
         10,
         0,
         5,
         'Kaon pt',
         my_email,
         'Transverse momentum of the kaon candidate',
         'Maximum between 1.5 and 2 GeV/c',
         'Kaon pt [GeV/c]',
         'Candidates'),
        ('nCleanedTracks_simple_cleanup',
         12,
         0,
         12,
         'Number of cleaned tracks',
         my_email,
         'Number of cleaned tracks in the event',
         'Should be between 4 and 10, with two local maxima at 4 and 6',
         'Number of cleaned tracks',
         'Events'),
        ('sphericity',
         10,
         0,
         1,
         'Event Sphericity',
         my_email,
         'Sphericity computed by ma.buildEventShape',
         'Maximum around 0.3',
         'Event Sphericity',
         'Events')],
    variables_2d=[],
    path=path)

b2.process(path)
print(b2.statistics)
