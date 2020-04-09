#!/usr/bin/env python3

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial runs over skims of centrally produced B->KsPi0 or continuum MC
# mdst files and reconstructs B->KsPi0 decays, applies the MVAExpert module,
# and writes out flat NTuples containing all variables used in the deep continuum
# suppression training + the (transformed) network output distribution.
# Also have a look at the Continuum suppression section at https://software.belle2.org
# The techniques are described in more detail in http://ekp-invenio.physik.uni-karlsruhe.de/record/48934
#
# This module requires the weightfile produced in B2A712 (Deep_Feed_Forward.xml).
#
# Usage:
#   basf2 B2A713-ContinuumSuppression_MVAExpert.py <signal,qqbar>
#
# Contributors: D. Weyland (November 2017), P. Goldenzweig (October 2016)
#
################################################################################

import basf2
import modularAnalysis as ma
from vertex import TagV
import variables as v
import glob
import sys
import numpy as np

basf2.set_log_level(basf2.LogLevel.ERROR)

# --I/O----------------------------------------------------------------------------------------
if (len(sys.argv) < 2 or sys.argv[1] not in ['signal', 'qqbar']):
    sys.exit("usage:\n\tbasf2 B2A703-ContinuumSuppression_MVAExpert.py <signal,qqbar>")

step = str(sys.argv[1])

path = '/group/belle2/tutorial/release_01-00-00/DCS_Bd_KsPi0'
signal_inputfiles = glob.glob(path + '/*/*/*1.root.mdst')
qqbar_inputfiles = glob.glob(path + '/*/*/*7.root.mdst')

np.random.shuffle(signal_inputfiles)
np.random.shuffle(qqbar_inputfiles)

if step == 'signal':
    input = signal_inputfiles
elif step == 'qqbar':
    input = qqbar_inputfiles
else:
    sys.exit('Step does not match any of the available samples: `signal` or `qqbar`')

outfile = 'MVAExpert_fullNTuple_' + step + '.root'
# ---------------------------------------------------------------------------------------------

main = basf2.create_path()

# Perform analysis.
ma.inputMdstList('default', input, path=main)

main.add_module('ProgressBar')

# Build B candidate like in B2A701-ContinuumSuppression_Input.py
ma.fillParticleList('gamma', 'goodGamma == 1', path=main)

ma.fillParticleList('pi+:good', 'chiProb > 0.001 and pionID > 0.5', path=main)

ma.reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.516', path=main)
ma.reconstructDecay('pi0  -> gamma gamma', '0.115<=M<=0.152', path=main)
ma.reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=main)

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
                '{}_{}{}'.format(
                    variable, shortcut, rank), 'getVariableByRank({}, cmsp, {}, {})'.format(
                    particlelist, variable, rank + 1))
            variables.append('{}_{}{}'.format(variable, shortcut, rank))

for rank in range(5):
    for shortcut, particlelist in [('TProe', 'pi+:chargedProe'), ('TPsig', 'pi+:chargedPsignal'),
                                   ('TMroe', 'pi+:chargedMroe'), ('TMsig', 'pi+:chargedMsignal')]:
        for variable in track_variables:
            v.variables.addAlias(
                '{}_{}{}'.format(
                    variable, shortcut, rank), 'getVariableByRank({}, cmsp, {}, {})'.format(
                    particlelist, variable, rank + 1))
            variables.append('{}_{}{}'.format(variable, shortcut, rank))

# MVAExpert
# In this path there are already several trained weightfiles. Look at README for a short explanation
path = '/gpfs/fs02/belle2/users/pablog/inputForDNNContinuumSuppression/'

roe_path.add_module('MVAExpert', listNames=['B0'], extraInfoName='Deep_CS', identifier=path + 'Deep_Feed_Forward.xml')

# Variables from MVAExpert.
expertVars = ['extraInfo(Deep_CS)', 'transformedNetworkOutput(Deep_CS,0.1,1.0)']

# Create output file with all sets of variables.
ma.variablesToNtuple('B0', variables + expertVars, treename='tree', filename=outfile, path=roe_path)

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

basf2.process(main)
print(basf2.statistics)
