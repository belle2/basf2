#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial runs over skims of centrally produced B->KsPi0 or        #
# continuum MC mdst files and reconstructs B->KsPi0 decays, applies the  #
# MVAExpert module, and writes out flat NTuples containing all           #
# variables used in the deep continuum suppression training + the        #
# (transformed) network output distribution. Also have a look at the     #
# Continuum suppression section at https://software.belle2.org The       #
# techniques are described in more detail in                             #
# http://ekp-invenio.physik.uni-karlsruhe.de/record/48934                #
#                                                                        #
# This module requires the weightfile produced in B2A712                 #
# (Deep_Feed_Forward.xml).                                               #
#                                                                        #
# Usage:                                                                 #
#   basf2 B2A713-ContinuumSuppression_MVAExpert.py <signal,qqbar>        #
#                                                                        #
##########################################################################

import basf2
import modularAnalysis as ma
from stdV0s import stdKshorts
from stdPi0s import stdPi0s
from vertex import TagV
import variables as v
import glob
import sys
import numpy as np

basf2.set_log_level(basf2.LogLevel.ERROR)

# --I/O----------------------------------------------------------------------------------------
step = 'signal'

if len(sys.argv) >= 2:
    if sys.argv[1] not in ['signal', 'qqbar']:
        sys.exit("usage:\n\tbasf2 B2A713-DeepContinuumSuppression_MVAExpert.py <signal,qqbar>")
    else:
        step = str(sys.argv[1])

if step == 'signal':
    input_file_list = [basf2.find_file('Bd2K0spi0_to_test.root', 'examples', False)]
elif step == 'qqbar':
    input_file_list = [basf2.find_file('ccbar_sample_to_test.root', 'examples', False)]
else:
    sys.exit('Step does not match any of the available samples: `signal` or `qqbar`')

outfile = 'MVAExpert_fullNTuple_' + step + '.root'
# ---------------------------------------------------------------------------------------------

main = basf2.create_path()

# Perform analysis.
ma.inputMdstList('default', input_file_list, path=main)

main.add_module('ProgressBar')

# Build B candidate like in B2A701-ContinuumSuppression_Input.py
stdKshorts(path=main)
stdPi0s('eff40_May2020', path=main)
ma.reconstructDecay('B0 -> K_S0:merged pi0:eff40_May2020', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=main)

ma.matchMCTruth('B0', path=main)
ma.buildRestOfEvent('B0', path=main)

cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
ma.appendROEMasks('B0', [cleanMask], path=main)

ma.buildContinuumSuppression('B0', 'cleanMask', path=main)

# Accept only correctly reconstructed B candidates as signal
ma.applyCuts('B0', 'isSignal or isContinuumEvent', path=main)

# Tag B candidate for Vertex information
TagV('B0', path=main)

# Loop over each possible ROE (1 for every B candidate) in every event
roe_path = basf2.create_path()

deadEndPath = basf2.create_path()

ma.signalSideParticleFilter('B0', '', roe_path, deadEndPath)

# Build particle lists for low level variables
ma.fillParticleList('gamma:roe', 'isInRestOfEvent == 1 and goodBelleGamma == 1', path=roe_path)
ma.fillParticleList('gamma:signal', 'isInRestOfEvent == 0 and goodBelleGamma == 1', path=roe_path)
ma.fillParticleList('pi+:chargedProe', 'isInRestOfEvent == 1', path=roe_path)
ma.fillParticleList('pi+:chargedPsignal', 'isInRestOfEvent == 0', path=roe_path)
ma.fillParticleList('pi-:chargedMroe', 'isInRestOfEvent == 1', path=roe_path)
ma.fillParticleList('pi-:chargedMsignal', 'isInRestOfEvent == 0', path=roe_path)

v.variables.addAlias('cmsp', 'useCMSFrame(p)')

ma.rankByHighest('gamma:roe', 'cmsp', path=roe_path)
ma.rankByHighest('gamma:signal', 'cmsp', path=roe_path)
ma.rankByHighest('pi+:chargedProe', 'cmsp', path=roe_path)
ma.rankByHighest('pi+:chargedPsignal', 'cmsp', path=roe_path)
ma.rankByHighest('pi-:chargedMroe', 'cmsp', path=roe_path)
ma.rankByHighest('pi-:chargedMsignal', 'cmsp', path=roe_path)

# Define traditional Continuum Suppression Variables
contVars = [
    'R2',
    'thrustBm',
    'thrustOm',
    'cosTBTO',
    'cosTBz',
    'KSFWVariables(et)',
    'KSFWVariables(mm2)',
    'KSFWVariables(hso00)',
    'KSFWVariables(hso02)',
    'KSFWVariables(hso04)',
    'KSFWVariables(hso10)',
    'KSFWVariables(hso12)',
    'KSFWVariables(hso14)',
    'KSFWVariables(hso20)',
    'KSFWVariables(hso22)',
    'KSFWVariables(hso24)',
    'KSFWVariables(hoo0)',
    'KSFWVariables(hoo1)',
    'KSFWVariables(hoo2)',
    'KSFWVariables(hoo3)',
    'KSFWVariables(hoo4)',
    'CleoConeCS(1)',
    'CleoConeCS(2)',
    'CleoConeCS(3)',
    'CleoConeCS(4)',
    'CleoConeCS(5)',
    'CleoConeCS(6)',
    'CleoConeCS(7)',
    'CleoConeCS(8)',
    'CleoConeCS(9)'
]

# Define additional low level variables
basic_variables = ['p', 'phi', 'cosTheta', 'pErr', 'phiErr', 'cosThetaErr']
vertex_variables = ['distance', 'dphi', 'dcosTheta']
cluster_specific_variables = ['clusterNHits', 'clusterTiming', 'clusterE9E25', 'clusterReg', 'isInRestOfEvent']
track_specific_variables = ['kaonID', 'electronID', 'muonID', 'protonID', 'pValue', 'nCDCHits', 'isInRestOfEvent', 'charge']

for variablename in basic_variables + vertex_variables:
    v.variables.addAlias('thrustsig' + variablename, 'useBThrustFrame(' + variablename + ',Signal)')

cluster_variables = cluster_specific_variables[:]
for variablename in basic_variables:
    cluster_variables.append('thrustsig' + variablename)

track_variables = track_specific_variables
for variablename in basic_variables + vertex_variables:
    track_variables.append('thrustsig' + variablename)

variables = ['isContinuumEvent', 'isNotContinuumEvent', 'isSignal', 'M', 'p', 'Mbc', 'DeltaZ',
             'deltaE', 'daughter(0, M)', 'daughter(0, p)', 'daughter(1, M)', 'daughter(1, p)']
for rank in range(10):
    for shortcut, particlelist in [('Croe', 'gamma:roe'), ('Csig', 'gamma:signal')]:
        for variable in cluster_variables:
            v.variables.addAlias(
                f'{variable}_{shortcut}{rank}', f'getVariableByRank({particlelist}, cmsp, {variable}, {rank + 1})')
            variables.append(f'{variable}_{shortcut}{rank}')

for rank in range(5):
    for shortcut, particlelist in [('TProe', 'pi+:chargedProe'), ('TPsig', 'pi+:chargedPsignal'),
                                   ('TMroe', 'pi+:chargedMroe'), ('TMsig', 'pi+:chargedMsignal')]:
        for variable in track_variables:
            v.variables.addAlias(
                f'{variable}_{shortcut}{rank}', f'getVariableByRank({particlelist}, cmsp, {variable}, {rank + 1})')
            variables.append(f'{variable}_{shortcut}{rank}')

# MVAExpert
roe_path.add_module('MVAExpert', listNames=['B0'], extraInfoName='Deep_CS', identifier='Deep_Feed_Forward.xml')

# Variables from MVAExpert.
expertVars = ['extraInfo(Deep_CS)', 'transformedNetworkOutput(Deep_CS,0.1,1.0)']

# Create output file with all sets of variables.
ma.variablesToNtuple('B0', variables + expertVars, treename='tree', filename=outfile, path=roe_path)

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

basf2.process(main)
print(basf2.statistics)
