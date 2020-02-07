#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Author: The Belle II Collaboration
# Contributors: Benjamin Oberhof
#
########################################################

import os
import glob
import random
import basf2 as b2
from ROOT import Belle2
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from beamparameters import add_beamparameters

main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000],
                       'runList': [1],
                       'expList': [0]})

main.add_module(eventinfosetter)

intseed = random.randint(2, 2)

pGun = b2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [.1],
    'thetaGeneration': 'uniform',
    'thetaParams': [13., 150.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
}
pGun.param(param_pGun)
main.add_module(pGun)

bkg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '*.root')

add_simulation(main)
add_reconstruction(main)

ecl_digitizerPureCsI = b2.register_module('ECLDigitizerPureCsI')
ecl_digitizerPureCsI.param('adcTickFactor', 8)
ecl_digitizerPureCsI.param('sigmaTrigger', 0.)
ecl_digitizerPureCsI.param('elecNoise', 1.3)
ecl_digitizerPureCsI.param('photostatresolution', 0.4)
ecl_digitizerPureCsI.param('sigmaTrigger', 0)
ecl_digitizerPureCsI.param('LastRing', 12)
ecl_digitizerPureCsI.param('NoCovMatrix', 1)
ecl_digitizerPureCsI.param('Background', 0)
main.add_module(ecl_digitizerPureCsI)

# Ii is IMPORTANT to set 'simulatePure' to 1 for pure CsI simulation!"
ecl_calibrator_PureCsI = b2.register_module('ECLDigitCalibratorPureCsI')
ecl_calibrator_PureCsI.param('simulatePure', 1)
main.add_module(ecl_calibrator_PureCsI)

ecl_crfinder_PureCsI = b2.register_module('ECLCRFinderPureCsI')
main.add_module(ecl_crfinder_PureCsI)

ecl_lmfinder_PureCsI = b2.register_module('ECLLocalMaximumFinderPureCsI')
main.add_module(ecl_lmfinder_PureCsI)

ecl_splitterN1_PureCsI = b2.register_module('ECLSplitterN1PureCsI')
main.add_module(ecl_splitterN1_PureCsI)

ecl_splitterN2_PureCsI = b2.register_module('ECLSplitterN2PureCsI')
main.add_module(ecl_splitterN2_PureCsI)

ecl_showercorrection_PureCsI = b2.register_module('ECLShowerCorrectorPureCsI')
main.add_module(ecl_showercorrection_PureCsI)

ecl_showercalibration_PureCsI = b2.register_module('ECLShowerCalibratorPureCsI')
main.add_module(ecl_showercalibration_PureCsI)

ecl_showershape_PureCsI = b2.register_module('ECLShowerShapePureCsI')
main.add_module(ecl_showershape_PureCsI)

ecl_covariance_PureCsI = b2.register_module('ECLCovarianceMatrixPureCsI')
main.add_module(ecl_covariance_PureCsI)

ecl_finalize_PureCsI = b2.register_module('ECLFinalizerPureCsI')
main.add_module(ecl_finalize_PureCsI)

ecl_mcmatch_PureCsI = b2.register_module('MCMatcherECLClustersPureCsI')
main.add_module(ecl_mcmatch_PureCsI)

ecldataanalysis = b2.register_module('ECLDataAnalysis')
ecldataanalysis.param('writeToRoot', 1)
ecldataanalysis.param('rootFileName', 'Ecl_Pure_CsI_Example.root')
ecldataanalysis.param('doSimulation', 0)
ecldataanalysis.param('doTracking', 1)
ecldataanalysis.param('doPureCsIStudy', 1)
main.add_module(ecldataanalysis)

output = b2.register_module('RootOutput')
output.param('outputFileName', 'Ecl_Pure_mdst.root')
main.add_module(output)

b2.process(main)
print(b2.statistics)
