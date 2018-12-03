#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to effectively access all ECLClusters via
# the particle loader
#
# Contributors: Torben Ferber (2018)
#
################################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from modularAnalysis import printDataStore
from modularAnalysis import reconstructDecay
from modularAnalysis import copyLists
from modularAnalysis import variablesToNtuple
from variables import variables

# create path
mypath = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu_evtgen.root', 'examples', False),
          path=mypath)

# fill an all photon and all charged particle (does not really matter which one) list
fillParticleList('gamma:all', 'clusterE > 0.1', path=mypath)  # neutral clusters
fillParticleList('e-:all', 'clusterE > 0.1', path=mypath)  # track matched clusters

# reconstruct a pseudo particles with different combinations (implicit charge conjugation)
reconstructDecay('vpho:1 -> gamma:all gamma:all', '', 1, path=mypath)  # two neutral '00'
reconstructDecay('vpho:2 -> gamma:all e-:all', '', 2, path=mypath)  # neutral and charged '0+' and '0-'
reconstructDecay('vpho:3 -> e-:all e+:all', '', 3, path=mypath)  # different charge '+-' and '-+'
reconstructDecay('vpho:4 -> e-:all e-:all', '', 4, path=mypath)  # same charge '++' and '--'
copyLists('vpho:bhabha', ['vpho:1', 'vpho:2', 'vpho:3', 'vpho:4'], path=mypath)

# aliases to make output better readable
variables.addAlias('combinationID', 'extraInfo(decayModeID)')
variables.addAlias('deltaPhi', 'daughterDiffOfClusterPhi(0, 1)')
variables.addAlias('deltaTheta', 'formula(daughter(0, clusterTheta) - daughter(1, clusterTheta))')
variables.addAlias('charge_0', 'daughter(0, charge)')
variables.addAlias('charge_1', 'daughter(1, charge)')
variables.addAlias('clusterE_0', 'daughter(0, clusterE)')
variables.addAlias('clusterE_1', 'daughter(1, clusterE)')
variables.addAlias('clusterTheta_0', 'daughter(0, clusterTheta)')
variables.addAlias('clusterTheta_1', 'daughter(1, clusterTheta)')
variables.addAlias('clusterPhi_0', 'daughter(0, clusterPhi)')
variables.addAlias('clusterPhi_1', 'daughter(1, clusterPhi)')

variables.addAlias('clusterECMS_0', 'daughter(0, useCMSFrame(clusterE))')
variables.addAlias('clusterECMS_1', 'daughter(1, useCMSFrame(clusterE))')
variables.addAlias('clusterThetaCMS_0', 'daughter(0, useCMSFrame(clusterTheta))')
variables.addAlias('clusterThetaCMS_1', 'daughter(1, useCMSFrame(clusterTheta))')
variables.addAlias('clusterPhiCMS_0', 'daughter(0, useCMSFrame(clusterPhi))')
variables.addAlias('clusterPhiCMS_1', 'daughter(1, useCMSFrame(clusterPhi))')
variables.addAlias('deltaPhiCMS', 'daughterDiffOfClusterPhiCMS(0, 1)')

# variables to ntuple
vars = ['combinationID',
        'deltaPhi',
        'deltaPhiCMS',
        'deltaTheta',
        'charge_0',
        'charge_1',
        'clusterE_0',
        'clusterE_1',
        'clusterTheta_0',
        'clusterTheta_1',
        'clusterPhi_0',
        'clusterPhi_1',
        'clusterECMS_0',
        'clusterECMS_1',
        'clusterPhiCMS_0',
        'clusterPhiCMS_1',
        'clusterThetaCMS_0',
        'clusterThetaCMS_1'
        ]

# store variables
variablesToNtuple('vpho:bhabha', vars, filename='bhabha.root', path=mypath)

# Process the events
b2.process(mypath)

# print out the summary
print(b2.statistics)
