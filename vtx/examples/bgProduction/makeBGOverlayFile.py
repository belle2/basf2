#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from svd import add_svd_simulation
from vtx import add_vtx_simulation
import basf2 as b2
from basf2 import register_module
from pybasf2 import B2WARNING, B2ERROR, B2INFO
from simulation import add_simulation
import glob
import sys
import os

# -----------------------------------------------------------------------------------
# This example is for preparing a root file for BG overlay using simulated BG samples
# Output file: BGforOverlay.root
# -----------------------------------------------------------------------------------

vtx_flavor = "VTX-CMOS-7layer"    # Geometry components for VTX

argvs = sys.argv
if len(argvs) > 1:
    if argvs[1] == 'phase2':
        phase = 2
        compression = 3
    elif argvs[1] == 'phase3':
        phase = 3
        compression = 4
    elif argvs[1] == 'phase31':
        phase = 31
        compression = 4
    else:
        B2ERROR('The argument can be either phase2, phase3, or phasse31')
        sys.exit()
else:
    B2ERROR('No argument given specifying the running phase')
    B2INFO('Usage: basf2 ' + argvs[0] + ' phase2/phase3/phase31' + ' [scaleFactor=1]')
    sys.exit()

scaleFactor = 1.0
if len(argvs) > 2:
    scaleFactor = float(argvs[2])

if len(argvs) > 3:
    vtx_flavor = str(argvs[3])


if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG samples')
    sys.exit()

bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if len(bg) == 0:
    B2ERROR('No root files found in folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

if phase == 2:
    for fileName in bg:
        if 'phase2' not in fileName:
            B2ERROR('BG mixing samples given in BELLE2_BACKGROUND_MIXING_DIR are not for phase 2')
            B2INFO('Try:\n export BELLE2_BACKGROUND_MIXING_DIR=/group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set0/')
            sys.exit()

B2INFO('Making BG overlay sample for ' + argvs[1] + ' with ECL compression = ' +
       str(compression))
B2INFO('Using background samples from folder ' + os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
B2INFO('With scaling factor: ' + str(scaleFactor))

b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
if phase == 2:
    gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
if phase == 31:
    gearbox.param('fileName', 'geometry/Belle2_earlyPhase3.xml')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', False)
# Create vtx upgrade geometry for phase3
if phase == 3:
    geometry.param({"excludedComponents": ["PXD", "SVD"],
                    "additionalComponents": [vtx_flavor, ], })
main.add_module(geometry)

# Beam background mixer
bkgmixer = register_module('BeamBkgMixer')
bkgmixer.param('backgroundFiles', bg)
bkgmixer.param('overallScaleFactor', scaleFactor)
main.add_module(bkgmixer)

# PXD digitizer (no data reduction!)
pxd_digitizer = register_module('PXDDigitizer')
pxd_digitizer.param('IntegrationWindow', False)
main.add_module(pxd_digitizer)

# VTX digitization
add_vtx_simulation(main)

# SVD digitization
add_svd_simulation(main)

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

# KLM digitization
klm_digitizer = register_module('KLMDigitizer')
main.add_module(klm_digitizer)

# Output: digitized hits only
output = register_module('RootOutput')
output.param('outputFileName', 'BGforOverlay.root')
output.param('branchNames', ['PXDDigits', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
                             'ARICHDigits', 'ECLWaveforms', 'BKLMDigits', 'EKLMDigits', 'VTXDigits'])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
