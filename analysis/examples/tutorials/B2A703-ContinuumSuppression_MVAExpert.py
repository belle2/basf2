#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial runs over skims of centrally produced B->KsPi0 or continuum MC
# mdst files and reconstructs B->KsPi0 decays, applies the MVAExpert module,
# and writes out flat NTuples containing all variables used in the continuum
# suppression training + the (transformed) network output distribution.
#
# This module requires the weightfile produced in B2A702 (MVAFastBDT.root).
#
# Usage:
#   basf2 B2A703-ContinuumSuppression_MVAExpert.py <signal,qqbar>
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
step = 'signal'

if len(sys.argv) >= 2:
    step = str(sys.argv[1])

if step == 'signal':
    input_file = b2.find_file('Bd2K0spi0_to_test.root', 'examples', False)
elif step == 'qqbar':
    input_file = b2.find_file('ccbar_sample_to_test.root', 'examples', False)
else:
    sys.exit('Step does not match any of the available samples: `signal` or `qqbar`')

outfile = 'MVAExpert_fullNTuple_' + step + '.root'
# ---------------------------------------------------------------------------------------------

# Perform analysis.
main = b2.create_path()

ma.inputMdst(environmentType='default',
             filename=input_file,
             path=main)

ma.fillParticleList(decayString='gamma:all',
                    cut='', path=main)
ma.fillParticleList(decayString='pi+:good',
                    cut='chiProb > 0.001 and pionID > 0.5',
                    path=main)
ma.fillParticleList(decayString='pi-:good',
                    cut='chiProb > 0.001 and pionID > 0.5',
                    path=main)

ma.reconstructDecay(decayString='K_S0 -> pi+:good pi-:good',
                    cut='0.480<=M<=0.516',
                    dmID=1,
                    path=main)
ma.reconstructDecay(decayString='pi0  -> gamma:all gamma:all',
                    cut='0.115<=M<=0.152',
                    dmID=1,
                    path=main)
ma.reconstructDecay(decayString='B0   -> K_S0 pi0',
                    cut='5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2',
                    path=main)

ma.matchMCTruth(list_name='B0', path=main)
ma.buildRestOfEvent(target_list_name='B0', path=main)

# The momentum cuts used to be hard-coded in the continuum suppression module. They can now be applied
# via this mask. The nCDCHits requirement is new, and is recommended to remove VXD-only fake tracks.
cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
ma.appendROEMasks(list_name='B0',
                  mask_tuples=[cleanMask],
                  path=main)

ma.buildContinuumSuppression(list_name='B0',
                             roe_mask='cleanMask',
                             path=main)

# Define the variables for training.
#  For details, please see the Continuum suppression section at https://software.belle2.org
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

# Target variable used in training.
targetVar = ['isNotContinuumEvent']

# MVAExpert
main.add_module('MVAExpert', listNames=['B0'], extraInfoName='FastBDT', identifier='MVAFastBDT.root')

# Variables from MVAExpert.
expertVars = ['extraInfo(FastBDT)', 'transformedNetworkOutput(FastBDT,0.1,1.0)']

# Create output file with all sets of variables.
ma.variablesToNtuple(decayString='B0',
                     variables=trainVars + targetVar + expertVars,
                     treename='tree',
                     filename=outfile,
                     path=main)

b2.process(main)
print(b2.statistics)
