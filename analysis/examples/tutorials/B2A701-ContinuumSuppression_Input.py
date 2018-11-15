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
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma

# --I/O----------------------------------------------------------------------------------------
if (len(sys.argv) < 2 or sys.argv[1] not in ['train', 'test', 'apply_signal', 'apply_qqbar']):
    sys.exit("usage:\n\tbasf2 B2A701-ContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>")

import os
if not os.getenv('BELLE2_EXAMPLES_DATA_DIR'):
    b2.B2FATAL("You need the example data installed. Run `b2install-data example` in terminal for it.")

step = str(sys.argv[1])

path = '$BELLE2_EXAMPLES_DATA_DIR/mva/'
input_file = ''

if step == 'train':
    input_file = [path + '*_train/*']
elif step == 'test':
    input_file = [path + '*_test/*']
elif step == 'apply_signal':
    input_file = [path + 'Bd_KsPi0_expert/*']
elif step == 'apply_qqbar':
    input_file = [path + '*bar_expert/*']
else:
    sys.exit('Step does not match any of the available samples: `train`, `test`, `apply_signal`or `apply_qqbar`')

outfile = step + '.root'
# ---------------------------------------------------------------------------------------------

# Perform analysis.
my_path = b2.create_path()

ma.inputMdst(environmentType='default',
             filename=input_file,
             path=my_path)

ma.fillParticleList(decayString='gamma:all',
                    cut='',
                    path=my_path)
ma.fillParticleList(decayString='pi+:good',
                    cut='chiProb > 0.001 and pionID > 0.5',
                    path=my_path)
ma.fillParticleList(decayString='pi-:good',
                    cut='chiProb > 0.001 and pionID > 0.5',
                    path=my_path)

ma.reconstructDecay(decayString='K_S0 -> pi+:good pi-:good',
                    cut='0.480<=M<=0.516',
                    dmID=1,
                    path=my_path)
ma.reconstructDecay(decayString='pi0  -> gamma:all gamma:all',
                    cut='0.115<=M<=0.152',
                    dmID=1,
                    path=my_path)
ma.reconstructDecay(decayString='B0   -> K_S0 pi0',
                    cut='5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2',
                    path=my_path)

ma.matchMCTruth(list_name='B0', path=my_path)
ma.buildRestOfEvent(list_name='B0', path=my_path)

# The momentum cuts used to be hard-coded in the continuum suppression module. They can now be applied
# via this mask. The nCDCHits requirement is new, and is recommended to remove VXD-only fake tracks.
cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
ma.appendROEMasks(list_name='B0',
                  mask_tuples=[cleanMask],
                  path=my_path)

ma.buildContinuumSuppression(list_name='B0',
                             roe_mask='cleanMask',
                             path=my_path)

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
ma.variablesToNtuple(decayString='B0',
                     variables=trainVars + targetVar,
                     treename='tree',
                     filename=outfile,
                     path=my_path)

b2.process(my_path)
print(b2.statistics)
