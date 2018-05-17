#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial runs over skims of centrally produced B->KsPi0 and continuum MC
# mdst files and creates flat NTuples of B->KsPi0 decays, which are used in
# tutorials B2A702 AND B2A703 for training, testing, and applying the MVAExpert.
#
# Usage:
#   basf2 B2A701-ContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>
#
# Contributors: P. Goldenzweig (October 2016)
#
################################################################################

from modularAnalysis import *

set_log_level(LogLevel.ERROR)

# --I/O----------------------------------------------------------------------------------------
if (len(sys.argv) < 2 or sys.argv[1] not in ['train', 'test', 'apply_signal', 'apply_qqbar']):
    sys.exit("usage:\n\tbasf2 B2A701-ContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>")

step = str(sys.argv[1])

path = '/group/belle2/tutorial/release_01-00-00/Bd_KsPi0/mdst/'
input = ''

if step == 'train':
    input = [path + '*_train/*']
elif step == 'test':
    input = [path + '*_test/*']
elif step == 'apply_signal':
    input = [path + 'Bd_KsPi0_expert/*']
elif step == 'apply_qqbar':
    input = [path + '*bar_expert/*']
else:
    sys.exit('Step does not match any of the available samples: `train`, `test`, `apply_signal`or `apply_qqbar`')

outfile = step + '.root'
# ---------------------------------------------------------------------------------------------

# Perform analysis.
main = create_path()

inputMdstList('default', input, path=main)

fillParticleList('gamma:all', '', path=main)
fillParticleList('pi+:good', 'chiProb > 0.001 and pionID > 0.5', path=main)
fillParticleList('pi-:good', 'chiProb > 0.001 and pionID > 0.5', path=main)

reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.516', 1, path=main)
reconstructDecay('pi0  -> gamma:all gamma:all', '0.115<=M<=0.152', 1, path=main)
reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=main)

matchMCTruth('B0', path=main)
buildRestOfEvent('B0', path=main)

# The momentum cuts used to be hard-coded in the continuum suppression module. They can now be applied
# via this mask. The nCDCHits requirement is new, and is recommended to remove VXD-only fake tracks.
cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
appendROEMasks('B0', [cleanMask], path=main)

buildContinuumSuppression('B0', 'cleanMask', path=main)

# Define the variables for training.
#  For details, please see: https://confluence.desy.de/display/BI/Continuum+Suppression+Framework
#  Note that KSFWVariables takes the optional additional argument FS1, to return the variables calculated from the
#  signal-B final state particles.
#  CleoCone also takes the optional additional argument ROE, to return the cones calculated from ROE particles only.
trainVars = [
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

# Save target variable necessary for training.
targetVar = ['isNotContinuumEvent']

# Create output file.
variablesToNTuple('B0', trainVars + targetVar, treename='tree', filename=outfile, path=main)

process(main)
print(statistics)
