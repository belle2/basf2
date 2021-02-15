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
from vertex import TagV
import glob
import sys
import numpy as np
import variables as v
from root_pandas import read_root

basf2.set_log_level(basf2.LogLevel.ERROR)

# --I/O----------------------------------------------------------------------------------------
if (len(sys.argv) < 2 or sys.argv[1] not in ['train', 'test', 'apply_signal', 'apply_qqbar']):
    sys.exit("usage:\n\tbasf2 B2A701-ContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>")

step = str(sys.argv[1])

path = '/group/belle2/tutorial/release_01-00-00/DCS_Bd_KsPi0'
train_inputfiles = glob.glob(path + '/*/*/*8.root.mdst')
val_inputfiles = glob.glob(path + '/*/*/*2.root.mdst')
signal_inputfiles = glob.glob(path + '/*/*/*1.root.mdst')
qqbar_inputfiles = glob.glob(path + '/*/*/*7.root.mdst')

# shuffle file names
np.random.shuffle(train_inputfiles)
np.random.shuffle(val_inputfiles)
np.random.shuffle(signal_inputfiles)
np.random.shuffle(qqbar_inputfiles)

if step == 'train':
    input = train_inputfiles
elif step == 'test':
    input = val_inputfiles
elif step == 'apply_signal':
    input = signal_inputfiles
elif step == 'apply_qqbar':
    input = qqbar_inputfiles
else:
    sys.exit('Step does not match any of the available samples: `train`, `test`, `apply_signal`or `apply_qqbar`')

outfile = step + '.root'
# ---------------------------------------------------------------------------------------------

# Perform analysis.
firstpath = basf2.Path()

ma.inputMdstList('default', input, path=firstpath)

firstpath.add_module('ProgressBar')

# Build B candidate like in B2A701-ContinuumSuppression_Input.py
ma.fillParticleList('gamma', 'goodGamma == 1', path=firstpath)

ma.fillParticleList('pi+:good', 'chiProb > 0.001 and pionID > 0.5', path=firstpath)

ma.reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.516', path=firstpath)
ma.reconstructDecay('pi0  -> gamma gamma', '0.115<=M<=0.152', path=firstpath)
ma.reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=firstpath)

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
ma.fillParticleList('gamma:roe', 'isInRestOfEvent == 1 and goodGamma == 1', path=roe_path)
ma.fillParticleList('gamma:signal', 'isInRestOfEvent == 0 and goodGamma == 1', path=roe_path)
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
    'CleoCone(1)',
    'CleoCone(2)',
    'CleoCone(3)',
    'CleoCone(4)',
    'CleoCone(5)',
    'CleoCone(6)',
    'CleoCone(7)',
    'CleoCone(8)',
    'CleoCone(9)'
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
df = read_root(outfile)
df = df.sample(frac=1)
df.to_root(outfile, key='tree')
