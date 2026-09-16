#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import create_path, set_log_level, B2ERROR, B2INFO, LogLevel, process
import os
# from svd import add_svd_simulation
import glob
import sys

# -----------------------------------------------------------------------------------
# Prepare a root file for BG overlay using simulated BG samples
#
# Usage:
#    basf2 makeBGOverlayMCFileUp.py phase [scenario ECLmode ECLnoise evnts scaleFactor]
# Arguments:
#    phase        one of: phase2, phase3, phase31
#    scenario     upgrade scenario (default=S1, options: S2, S3)
#    ECLmode      one of: newDSP, current (default=newDSP)
#    ECLnoise     one of: noise, noNoise (default=noise)
#    evnts        number of events to generate (default=100)
#    scaleFactor  overall scaling factor (default=1)
# Default output file: BGforOverlay.root
# -----------------------------------------------------------------------------------

# argument parsing
argvs = sys.argv
if len(argvs) > 1:
    if argvs[1] == 'phase2':
        compression = 3
        expNo = 1002
    elif argvs[1] == 'phase3':
        compression = 4
        expNo = 0
    elif argvs[1] == 'phase31':
        compression = 4
        expNo = 1003
    else:
        B2ERROR('The argument can be either phase2, phase3 or phase31')
        sys.exit()
else:
    B2ERROR('No argument given specifying the running phase')
    print('Usage: basf2 ' + argvs[0] + ' phase2/phase3/phase31 [scenario=S1 ' +
          'ECLmode=newDSP/current] ' + 'ECLnoise=noise/noNoise]')
    print()
    sys.exit()

scenario = "S1"
if len(argvs) > 2:
    scenario = argvs[2]

SCALE_FACTORS = {
    "S1": [
        ("Coulomb_LER", 8.5432),
        ("Brems_LER", 8.5432),
        ("Touschek_LER", 8.4938),
        ("Coulomb_HER", 1.5556),
        ("Brems_HER", 1.5556),
        ("Touschek_HER", 0.4444),
    ],
    "S2": [
        ("Coulomb_LER", 21.3580),
        ("Brems_LER", 21.3580),
        ("Touschek_LER", 21.2346),
        ("Coulomb_HER", 3.8889),
        ("Brems_HER", 3.8889),
        ("Touschek_HER", 1.1111),
    ],
    "S3": [
        ("Coulomb_LER", 42.7160),
        ("Brems_LER", 42.7160),
        ("Touschek_LER", 42.4691),
        ("Coulomb_HER", 7.7778),
        ("Brems_HER", 7.7778),
        ("Touschek_HER", 2.2222),
    ],
}

if scenario not in SCALE_FACTORS:
    B2ERROR('Upgrade scenario was not correclty setup. ')
    sys.exit()

ECLup = True
ECLmode = 'new'
if len(argvs) > 3:
    if argvs[3] == 'current':
        ECLup = False
        ECLmode = 'current'
    elif argvs[3] == 'newDSP':
        compression = 2
    else:
        B2ERROR("ECLmode must be either 'newDSP' or 'current'")
        sys.exit()
else:
    compression = 2

ECLnoise = True
ECLelec = 'noise'
if len(argvs) > 4:
    if argvs[4] == 'noNoise':
        ECLnoise = False
        ECLelec = 'noNoise'
    elif argvs[4] != 'noise':
        B2ERROR("ECLnoise must be either 'noise' or 'noNoise'")
        sys.exit()

evnt_num = 100
if len(argvs) > 5:
    evnt_num = int(argvs[5])

scaleFactor = 1.0
if len(argvs) > 6:
    scaleFactor = float(argvs[6])

# background files
if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG samples')
    B2INFO('on KEKCC: /sw/belle2/bkg.mixing')
    sys.exit()

bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')

if len(bg) == 0:
    B2ERROR('No root files found in folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

# extra files for adding SVD pick-up noise
if 'BELLE2_BACKGROUND_MIXING_EXTRA_DIRS' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_MIXING_EXTRA_DIRS variable is not set - it must contain the path to extra samples')
    B2INFO('on KEKCC: /sw/belle2/bkg.mixing_extra')
    sys.exit()

SVDOverlayDir = os.environ['BELLE2_BACKGROUND_MIXING_EXTRA_DIRS'] + '/SVD'
SVDOverlayFiles = glob.glob(SVDOverlayDir + '/*_overlay.root')

if len(SVDOverlayFiles) == 0:
    B2ERROR('No root files found in folder ' + SVDOverlayDir)
    sys.exit()

B2INFO('Making BG overlay sample for ' + argvs[1] + ' with ECL compression = ' +
       str(compression) + ' and with ' + ECLmode +
       ' ECL electronics, ' + ECLelec + ' elec for ECL. ')
B2INFO('Using background samples from folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
B2INFO('With scaling factor: ' + str(scaleFactor))
B2INFO('With SVD xTalk&Noise files from folder ' + str(SVDOverlayDir))

set_log_level(LogLevel.WARNING)

# Create path
main = create_path()

# Set number of events to generate
main.add_module('EventInfoSetter', evtNumList=[evnt_num], expList=[expNo])

# Gearbox: access to xml files
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Simulate the EventLevelTriggerTimeInfo (empty object for now)
main.add_module('SimulateEventLevelTriggerTimeInfo')

# Beam background mixer
main.add_module('BeamBkgMixer', backgroundFiles=bg, overallScaleFactor=scaleFactor)

# set background scale factors
for m in main.modules():
    if m.name() == "BeamBkgMixer":
        m.param("scaleFactors", SCALE_FACTORS[scenario])

# ECL digitization
if ECLup:
    main.add_module('ECLDigitizerUp', DiodeDeposition=True, HadronPulseShapes=False,
                    WaveformMaker=True, CompressionAlgorithm=compression)
else:
    main.add_module('ECLDigitizer', WaveformMaker=True, CompressionAlgorithm=compression)

# Output: digitized hits only
branches = ['EventLevelTriggerTimeInfo', 'ECLWaveforms']

main.add_module('RootOutput',
                outputFileName='BGforOverlay_' + argvs[1] + '_' + str(scenario) + '_' + ECLmode + '_' + ECLelec + '.root',
                branchNames=branches)

# Show progress of processing
main.add_module('Progress')

# Process events
process(main)
