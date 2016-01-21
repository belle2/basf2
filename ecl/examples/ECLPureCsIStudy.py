#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Dump digit information for pure CsI vs CsI(Tl)
# using particle gun to generate single particles
# enabling pure CsI digitization
# dump useful in a flat ntuple + enriched mdst
#
# Guglielmo De Nardo - 2015 Belle II Collaboration
########################################################

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from beamparameters import add_beamparameters

import sys
import glob

par = sys.argv
argc = len(par)

pdg = int(par[1])
momentum = float(par[2])
filenameprefix = par[3]
elenoise = float(par[4])
photo = float(par[5])

withbg = 0
if (argc == 7):
    isbg = par[6]
    withbg = 1
mdstfile = par[3]
mdstfile += '-mdst.root'
digistudyfile = par[3]
digistudyfile += '-digi'

generator = 0
if (pdg == 0):
    generator = 1

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
main.add_module(eventinfosetter)

if (generator == 0):
    # single particle generator settings
    pGun = register_module('ParticleGun')
    param_pGun = {
        'pdgCodes': [pdg],
        'nTracks': 1,
        'momentumGeneration': 'fixed',
        'momentumParams': [momentum],
        'thetaGeneration': 'uniform',
        'thetaParams': [13.0, 30.0],
        'phiGeneration': 'uniform',
        'phiParams': [0., 360.],
        'vertexGeneration': 'uniform',
        'xVertexParams': [0.0, 0.0],
        'yVertexParams': [0.0, 0.0],
        'zVertexParams': [0.0, 0.0],
    }

    pGun.param(param_pGun)
    main.add_module(pGun)
else:
    # beam parameters
    beamparameters = add_beamparameters(main, "Y4S")
    evtgeninput = register_module('EvtGenInput')
    main.add_module(evtgeninput)


if (withbg == 1):
    bg = glob.glob(isbg + '/*.root')
    add_simulation(main, bkgfiles=bg)
else:
    add_simulation(main)

add_reconstruction(main, components='ECL')

ecl_digitizerPureCsI = register_module('ECLDigitizerPureCsI')
ecl_digitizerPureCsI.param('adcTickFactor', 8)
ecl_digitizerPureCsI.param('sigmaTrigger', 0.)
ecl_digitizerPureCsI.param('elecNoise', elenoise)
ecl_digitizerPureCsI.param('photostatresolution', photo)
ecl_digitizerPureCsI.param('sigmaTrigger', 0)
ecl_digitizerPureCsI.param('LastRing', 12)
ecl_digitizerPureCsI.param('NoCovMatrix', 1)

if (withbg == 1):
    ecl_digitizerPureCsI.param('Background', 1)

main.add_module(ecl_digitizerPureCsI)

ecl_calibrator_PureCsI = register_module('ECLDigitCalibratorPureCsI')
main.add_module(ecl_calibrator_PureCsI)
ecl_shower_rec_PureCsI = register_module('ECLReconstructorPureCsI')
main.add_module(ecl_shower_rec_PureCsI)

ecl_digistudy = register_module('ECLDigiStudy')
ecl_digistudy.param('outputFileName', digistudyfile)
main.add_module(ecl_digistudy)

# display = register_module('Display')
# main.add_module(display)

add_mdst_output(
    main,
    mc=True,
    filename=mdstfile,
    additionalBranches=[
        'ECLHits',
        'ECLClustersPureCsI',
        'ECLDsps',
        'ECLDspsPureCsI',
        'ECLDigits',
        'ECLCalDigits',
        'ECLDigitsPureCsI',
        'ECLCalDigitsPureCsI',
        'ECLDigitsPureCsIToECLDspsPureCsI',
        'ECLDigitsPureCsIToECLHits'])

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

process(main)
print(statistics)
