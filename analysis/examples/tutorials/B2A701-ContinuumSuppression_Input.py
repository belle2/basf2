#!/usr/bin/env python3

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
import sys
import os


# --I/O----------------------------------------------------------------------------------------
# Please note that the files here are quite small and needed just to test the script.
# To obtain meaningful output to be used in B2A702 and B2A703 please use grid.
# > gbasf2 B2A701-ContinuumSuppression_Input.py -i <.mdst file addresses> -p <project name> -s <release_number>
#
# For B2A70{2|3} you need four sets of ntuples:
# -Train dataset composed of signal (B->KsPi0 ) and background MC (continuum MC) samples
# -Test dataset composed of signal (B->KsPi0 ) and background MC (continuum MC) samples
# -Signal dataset only
# -Background dataset only
# For each of this four sets you need to submit separate grid job.
# (Or use files that are already prepared for B2A702 and B2A703).


input_file_list = []

# Please pick a meaningful output name when running on the grid
outfile = "B2A702_output.root"

step = 'train'

magnetic_field = 'default'

if len(sys.argv) >= 2:
    if sys.argv[1] not in ['train', 'test', 'apply_signal', 'apply_qqbar']:
        sys.exit("usage:\n\tbasf2 B2A701-ContinuumSuppression_Input.py <train,test,apply_signal,apply_qqbar>")
    else:
        step = str(sys.argv[1])

# MC11 is not ready yet, so we have to manually put MC10 as a magnetic field config.
magnetic_field = 'MC10'
if step == 'train':
    input_file_list = [b2.find_file('ccbar_sample_to_train.root', 'examples', False),
                       b2.find_file('Bd2K0spi0_to_train.root', 'examples', False)]
elif step == 'test':
    input_file_list = [b2.find_file('ccbar_sample_to_test.root', 'examples', False),
                       b2.find_file('Bd2K0spi0_to_test.root', 'examples', False)]
elif step == 'apply_signal':
    input_file_list = [b2.find_file('Bd2K0spi0_to_test.root', 'examples', False)]
elif step == 'apply_qqbar':
    input_file_list = [b2.find_file('ccbar_sample_to_test.root', 'examples', False)]
else:
    sys.exit('Step does not match any of the available samples: `train`, `test`, `apply_signal`or `apply_qqbar`')
outfile = step + '.root'

# ---------------------------------------------------------------------------------------------

# Perform analysis.
my_path = b2.create_path()

ma.inputMdstList(environmentType=magnetic_field,
                 filelist=input_file_list,
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
ma.buildRestOfEvent(target_list_name='B0', path=my_path)

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
#  For details, please see the Continuum suppression section at https://software.belle2.org
#  Note that KSFWVariables takes the optional additional argument FS1, to return the variables calculated from the
#  signal-B final state particles.
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
