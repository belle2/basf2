#!/usr/bin/env python3

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial runs over skims of centrally produced B->KsPi0 and continuum MC
# mdst files and creates flat NTuples of B->KsPi0 decays, which are used in
# tutorials B2A712 AND B2A713 for training, testing, and applying the MVAExpert.
# It's useful to have a look at B2A701-ContinuumSuppression_Input.py first.
# Also have a look at the Continuum suppression section at https://software.belle2.org
# The new variables are described in more detail in http://ekp-invenio.physik.uni-karlsruhe.de/record/48934
#
# Usage:
#   basf2 B2A711-DeepContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>
#
# Contributors: D. Weyland (November 2017), P. Goldenzweig
#
################################################################################

import basf2
import modularAnalysis as ma
from stdV0s import stdKshorts
from stdPi0s import stdPi0s
from vertex import TagV
import glob
import sys
import numpy as np
import variables as v
from root_pandas import read_root

basf2.set_log_level(basf2.LogLevel.ERROR)

# --I/O----------------------------------------------------------------------------------------
step = 'train'

if len(sys.argv) >= 2:
    if sys.argv[1] not in ['train', 'test', 'apply_signal', 'apply_qqbar']:
        sys.exit("usage:\n\tbasf2 B2A711-DeepContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>")
    else:
        step = str(sys.argv[1])

if step == 'train':
    input_file_list = [basf2.find_file('ccbar_sample_to_train.root', 'examples', False),
                       basf2.find_file('Bd2K0spi0_to_train.root', 'examples', False)]
elif step == 'test':
    input_file_list = [basf2.find_file('ccbar_sample_to_test.root', 'examples', False),
                       basf2.find_file('Bd2K0spi0_to_test.root', 'examples', False)]
elif step == 'apply_signal':
    input_file_list = [basf2.find_file('Bd2K0spi0_to_test.root', 'examples', False)]
elif step == 'apply_qqbar':
    input_file_list = [basf2.find_file('ccbar_sample_to_test.root', 'examples', False)]
else:
    sys.exit('Step does not match any of the available samples: `train`, `test`, `apply_signal`or `apply_qqbar`')

outfile = 'DNN_' + step + '.root'
# ---------------------------------------------------------------------------------------------

# Perform analysis.
firstpath = basf2.Path()

ma.inputMdstList('MC10', input_file_list, path=firstpath)

firstpath.add_module('ProgressBar')

# Build B candidate like in B2A701-ContinuumSuppression_Input.py
stdKshorts(path=firstpath)
stdPi0s('eff40_May2020', path=firstpath)
ma.reconstructDecay('B0 -> K_S0:merged pi0:eff40_May2020', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=firstpath)

ma.matchMCTruth('B0', path=firstpath)
ma.buildRestOfEvent('B0', path=firstpath)

cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
ma.appendROEMasks('B0', [cleanMask], path=firstpath)

ma.buildContinuumSuppression('B0', 'cleanMask', path=firstpath)

# Accept only correctly reconstructed B candidates as signal
ma.applyCuts('B0', 'isSignal or isContinuumEvent', path=firstpath)

# Tag B candidate for Vertex information
TagV('B0', path=firstpath)

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

# Aliases from normal coordinates to thrustframe coordinates
for variablename in basic_variables + vertex_variables:
    v.variables.addAlias('thrustsig' + variablename, 'useBThrustFrame(' + variablename + ',Signal)')

cluster_variables = cluster_specific_variables[:]
for variablename in basic_variables:
    cluster_variables.append('thrustsig' + variablename)

track_variables = track_specific_variables
for variablename in basic_variables + vertex_variables:
    track_variables.append('thrustsig' + variablename)

# General variables and training targets, which are nice to have in the Ntuple
variables = ['isContinuumEvent', 'isNotContinuumEvent', 'isSignal', 'M', 'p', 'Mbc', 'DeltaZ',
             'deltaE', 'daughter(0, M)', 'daughter(0, p)', 'daughter(1, M)', 'daughter(1, p)']

# Aliases for variable ranks created by rankByHighest function
for rank in range(10):
    for shortcut, particlelist in [('Croe', 'gamma:roe'), ('Csig', 'gamma:signal')]:
        for variable in cluster_variables:
            v.variables.addAlias(
                '{}_{}{}'.format(
                    variable, shortcut, rank), 'getVariableByRank({}, cmsp, {}, {})'.format(
                    particlelist, variable, rank + 1))
            variables.append(f'{variable}_{shortcut}{rank}')

for rank in range(5):
    for shortcut, particlelist in [('TProe', 'pi+:chargedProe'), ('TPsig', 'pi+:chargedPsignal'),
                                   ('TMroe', 'pi+:chargedMroe'), ('TMsig', 'pi+:chargedMsignal')]:
        for variable in track_variables:
            v.variables.addAlias(
                '{}_{}{}'.format(
                    variable, shortcut, rank), 'getVariableByRank({}, cmsp, {}, {})'.format(
                    particlelist, variable, rank + 1))
            variables.append(f'{variable}_{shortcut}{rank}')

# Create output file.
ma.variablesToNtuple('B0', variables + contVars, treename='tree', filename=outfile, path=roe_path)

# Loop over each possible ROE (1 for every B candidate) in every event
firstpath.for_each('RestOfEvent', 'RestOfEvents', roe_path)

basf2.process(firstpath)
print(basf2.statistics)

# Shuffle Data. Use only if enough Ram is available
try:
    df = read_root(outfile)
    df = df.sample(frac=1)
    df.to_root(outfile, key='tree')
except OSError as e:
    print(e)
