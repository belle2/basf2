#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Author: The Belle II Collaboration
# Contributors: Benjamin Oberhof
#
########################################################

from basf2 import *
import glob
from ROOT import Belle2
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from beamparameters import add_beamparameters

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

import random
intseed = random.randint(2, 2)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [.1, 1.],
    'thetaGeneration': 'uniform',
    'thetaParams': [13., 30.],
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

ecl_digitizerPureCsI = register_module('ECLDigitizerPureCsI')
ecl_digitizerPureCsI.param('adcTickFactor', 8)
ecl_digitizerPureCsI.param('sigmaTrigger', 0.)
ecl_digitizerPureCsI.param('elecNoise', 1.3)
ecl_digitizerPureCsI.param('photostatresolution', 0.4)
ecl_digitizerPureCsI.param('sigmaTrigger', 0)
ecl_digitizerPureCsI.param('LastRing', 12)
ecl_digitizerPureCsI.param('NoCovMatrix', 1)
ecl_digitizerPureCsI.param('Background', 0)
main.add_module(ecl_digitizerPureCsI)

ecl_calibrator_PureCsI = register_module('ECLDigitCalibratorPureCsI')
main.add_module(ecl_calibrator_PureCsI)

ecl_crfinder_PureCsI = register_module('ECLCRFinderPureCsI')
main.add_module(ecl_crfinder_PureCsI)

ecl_lmfinder_PureCsI = register_module('ECLLocalMaximumFinderPureCsI')
main.add_module(ecl_lmfinder_PureCsI)

ecl_splitterN1_PureCsI = register_module('ECLSplitterN1PureCsI')
main.add_module(ecl_splitterN1_PureCsI)

ecl_splitterN2_PureCsI = register_module('ECLSplitterN2PureCsI')
main.add_module(ecl_splitterN2_PureCsI)

ecl_showercorrection_PureCsI = register_module('ECLShowerCorrectorPureCsI')
main.add_module(ecl_showercorrection_PureCsI)

ecl_showercalibration_PureCsI = register_module('ECLShowerCalibratorPureCsI')
main.add_module(ecl_showercalibration_PureCsI)

ecl_showershape_PureCsI = register_module('ECLShowerShapePureCsI')
main.add_module(ecl_showershape_PureCsI)

ecl_covariance_PureCsI = register_module('ECLCovarianceMatrixPureCsI')
main.add_module(ecl_covariance_PureCsI)

ecl_finalize_PureCsI = register_module('ECLFinalizerPureCsI')
main.add_module(ecl_finalize_PureCsI)

ecl_mcmatch_PureCsI = register_module('MCMatcherECLClustersPureCsI')
main.add_module(ecl_mcmatch_PureCsI)

ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('writeToRoot', 1)
ecldataanalysis.param('rootFileName', 'Ecl_Pure_CsI_Example.root')
ecldataanalysis.param('doSimulation', 0)
ecldataanalysis.param('doTracking', 1)
ecldataanalysis.param('doPureCsIStudy', 1)
main.add_module(ecldataanalysis)

output = register_module('RootOutput')
output.param('outputFileName', 'Ecl_Pure_rootOutput.root')
main.add_module(output)

process(main)
print(statistics)
