#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import create_path, set_log_level, B2ERROR, B2INFO, LogLevel, process, statistics
import os
from svd import add_svd_simulation
import glob
import sys

# -----------------------------------------------------------------------------------
# Prepare a root file for BG overlay using simulated BG samples
#
# Usage:
#    basf2 makeBGOverlayMCFile.py phase [scaleFactor PXDmode]
# Arguments:
#    phase        one of: phase2, phase3, phase31
#    scaleFactor  overall scaling factor (default=1)
#    PXDmode      one of: ungated, gated (default=ungated)
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
    print('Usage: basf2 ' + argvs[0] + ' phase2/phase3/phase31 [scaleFactor=1 ' +
          'PXDmode=ungated/gated]')
    print()
    sys.exit()

scaleFactor = 1.0
if len(argvs) > 2:
    scaleFactor = float(argvs[2])

gatedMode = False
mode = 'ungated'
if len(argvs) > 3 and argvs[3] == 'gated':
    gatedMode = True
    mode = 'gated'

# background files
if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG samples')
    sys.exit()

bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if len(bg) == 0:
    B2ERROR('No root files found in folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

B2INFO('Making BG overlay sample for ' + argvs[1] + ' with ECL compression = ' +
       str(compression) + ' and PXD in ' + mode + ' mode')
B2INFO('Using background samples from folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
B2INFO('With scaling factor: ' + str(scaleFactor))

set_log_level(LogLevel.WARNING)

# Create path
main = create_path()

# Set number of events to generate
main.add_module('EventInfoSetter', evtNumList=[100], expList=[expNo])

# Gearbox: access to xml files
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

if gatedMode:
    # Beam background mixer
    main.add_module('BeamBkgMixer', backgroundFiles=bg, overallScaleFactor=scaleFactor,
                    minTimePXD=-20000.0, maxTimePXD=20000.0)

    # Emulate injection vetos for PXD
    main.add_module('PXDInjectionVetoEmulator')

    # PXD digitizer (no data reduction!)
    main.add_module('PXDDigitizer')
else:
    # Beam background mixer
    main.add_module('BeamBkgMixer', backgroundFiles=bg, overallScaleFactor=scaleFactor)

    # PXD digitizer (no data reduction!)
    main.add_module('PXDDigitizer', IntegrationWindow=False)

# SVD digitization
add_svd_simulation(main)

# CDC digitization
main.add_module('CDCDigitizer')

# TOP digitization
main.add_module('TOPDigitizer')

# ARICH digitization
main.add_module('ARICHDigitizer')

# ECL digitization
main.add_module('ECLDigitizer', WaveformMaker=True, CompressionAlgorithm=compression)

# KLM digitization
main.add_module('KLMDigitizer')

# Output: digitized hits only
branches = ['PXDDigits', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
            'ARICHDigits', 'ECLWaveforms', 'KLMDigits']
if gatedMode:
    branches += ['PXDInjectionBGTiming']
main.add_module('RootOutput', outputFileName='BGforOverlay.root', branchNames=branches)

# Show progress of processing
main.add_module('Progress')

# Process events
process(main)

# Print call statistics
print(statistics)
