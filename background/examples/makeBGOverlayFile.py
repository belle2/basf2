#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
import glob
import sys

# -----------------------------------------------------------------------------------
# This example is for preparing a root file for BG overlay using simulated BG samples
# Output file: BGforOverlay.root
# -----------------------------------------------------------------------------------

argvs = sys.argv
if len(argvs) == 2:
    if argvs[1] == 'phase2':
        compression = 3
    elif argvs[1] == 'phase3':
        compression = 4
    else:
        B2ERROR('The argument can be either phase2 or phase3')
        sys.exit()
else:
    B2ERROR('No argument given specifying the running phase')
    B2INFO('Usage: basf2 ' + argvs[0] + ' phase2/phase3')
    sys.exit()


if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG samples')
    sys.exit()

bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if len(bg) == 0:
    B2ERROR('No root files found in folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

B2INFO('Making BG overlay sample for ' + argvs[1] + ' with ECL compression = ' +
       str(compression))
B2INFO('Using background samples from folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])

set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# Beam background mixer
bkgmixer = register_module('BeamBkgMixer')
bkgmixer.param('backgroundFiles', bg)
main.add_module(bkgmixer)

# PXD digitizer (no data reduction!)
pxd_digitizer = register_module('PXDDigitizer')
main.add_module(pxd_digitizer)

# SVD digitizer
svd_digitizer = register_module('SVDDigitizer')
main.add_module(svd_digitizer)

# CDC digitization
cdc_digitizer = register_module('CDCDigitizer')
cdc_digitizer.param("Output2ndHit", False)
main.add_module(cdc_digitizer)

# TOP digitization
top_digitizer = register_module('TOPDigitizer')
main.add_module(top_digitizer)

# ARICH digitization
arich_digitizer = register_module('ARICHDigitizer')
main.add_module(arich_digitizer)

# ECL digitization
ecl_digitizer = register_module('ECLDigitizer')
main.add_module(ecl_digitizer, WaveformMaker=True, CompressionAlgorithm=compression)

# BKLM digitization
bklm_digitizer = register_module('BKLMDigitizer')
main.add_module(bklm_digitizer)

# EKLM digitization
eklm_digitizer = register_module('EKLMDigitizer')
main.add_module(eklm_digitizer)

# Output: digitized hits only
output = register_module('RootOutput')
output.param('outputFileName', 'BGforOverlay.root')
output.param('branchNames', ['PXDDigits', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
                             'ARICHDigits', 'ECLWaveforms', 'BKLMDigits', 'EKLMDigits'])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
