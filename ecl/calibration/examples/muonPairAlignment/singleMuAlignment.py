#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
# Generate single muons and store quantities needed to study alignment
#
# usage:
#
# basf2 singleMuAlignment.py nEvents name newGeom
#     nEvents = number of events to generate
#     name: ntuple is written to "ntuples/name.root"
#     newGeom = 0 means standard ECL geometry; 1 is aligned geometry from GT; 2 = localdb
#
# (c) Christopher Hearty, 2020 (hearty@physics.ubc.ca)
#
##############################################################################

import sys
from basf2 import (register_module, process, print_params, create_path,
                   statistics, conditions)
from modularAnalysis import fillParticleList
from modularAnalysis import cutAndCopyList
from modularAnalysis import applyEventCuts
from simulation import add_simulation
from reconstruction import add_reconstruction
from variables import variables

# ..Check for the correct number of arguments
narg = len(sys.argv)
if(narg != 4):
    print("Job arguments: nEvents name newGeom; ", narg-1, "provided")
    sys.exit()

# ..Use experiment 10 to match mc13b samples
experiment = 1003
run = 5201
nevt = int(sys.argv[1])
outputFileName = "ntuples/" + sys.argv[2] + ".root"

# ..override ECL geometry if requested
newGeom = int(sys.argv[3])
if(newGeom == 1):
    conditions.globaltags = ['ecl_alignment']
if(newGeom == 2):
    conditions.prepend_testing_payloads("localdb/database.txt")
print(conditions.globaltags)

# ---------------------------------------------------------------------
# ..Set up particle gun
particlegun = register_module('ParticleGun')

particlegun.param('pdgCodes', [13])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [4.15, 4.5])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [127.5, 131.5])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0., 360.])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [-0.05])
particlegun.param('yVertexParams', [0.015])
particlegun.param('zVertexParams', [0.])
particlegun.param('independentVertices', False)

print_params(particlegun)


# ---------------------------------------------------------------------
# ..The path
main = create_path()
main.add_module("EventInfoSetter", expList=experiment, runList=run, evtNumList=nevt)
main.add_module("Progress")
main.add_module(particlegun)

add_simulation(main)
add_reconstruction(main)


# ---------------------------------------------------------------------
# ..Select the muon to store
fillParticleList('mu-:highp', 'useCMSFrame(p) > 3.5 and abs(d0) < 0.5 and abs(z0) < 4 and nCDCHits>0 and nVXDHits>0', path=main)
cutAndCopyList('mu-:highE', 'mu-:highp', 'clusterE>1.', path=main)

variables.addAlias('nhighp', 'nParticlesInList(mu-:highp)')
variables.addAlias('nhighE', 'nParticlesInList(mu-:highE)')
applyEventCuts('[nhighE==0] and [nhighp==1]', main)

# ---------------------------------------------------------------------
# ..Ntuple
variables.addAlias('muThetaLab', 'formula(57.2957795*theta)')
variables.addAlias('muPhiLab', 'formula(57.2957795*phi)')
variables.addAlias('muPcms', 'useCMSFrame(p)')
variables.addAlias('muptLab', 'pt')
variables.addAlias('e1Uncorr', 'formula(clusterHighestE * clusterUncorrE / clusterE)')
variables.addAlias('clustThetaLab', 'formula(57.2957795*clusterTheta)')
variables.addAlias('clustPhiLab', 'formula(57.2957795*clusterPhi)')

# ..dummies to match muon pair selection
variables.addAlias('bestMuonID', 'muonID')
variables.addAlias('worstMuonID', 'muonID')


varsToStore = [
    'charge',
    'p',
    'muPcms',
    'muptLab',
    'muThetaLab',
    'muPhiLab',
    'clusterE',
    'e1Uncorr',
    'clustThetaLab',
    'clustPhiLab',
    'HighLevelTrigger',
    'muonID',
    'bestMuonID',
    'worstMuonID']
main.add_module('VariablesToNtuple', particleList='mu-:highp', variables=varsToStore, fileName=outputFileName)


# ---------------------------------------------------------------------
# Process events
process(main)
print(statistics)
