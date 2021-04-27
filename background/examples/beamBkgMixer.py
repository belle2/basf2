#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import os
import sys
import glob

# ----------------------------------------------------------------------------------
# This example shows some of the possibilities to steer BG mixing with BeamBkgMixer.
#
# For simplicity no particle generator and/or geant simulation is included,
# digitization and event reconstruction is also not done. The output root file
# will contain SimHits from BG.
#
# Note: in multiprocessing mode put BG mixer in the path first then FullSim
# ----------------------------------------------------------------------------------

b2.set_log_level(b2.LogLevel.INFO)


if 'BELLE2_BACKGROUND_MIXING_DIR' not in os.environ:
    b2.B2ERROR('BELLE2_BACKGROUND_MIXING_DIR variable is not set - it must contain the path to BG mixing samples')
    sys.exit()

# define background (collision) files
#    glob.glob is the prefered way to get the list of files:
#    (the directory must include only BG files!)

bg = glob.glob(os.environ['BELLE2_BACKGROUND_MIXING_DIR'] + '/*.root')
if len(bg) == 0:
    b2.B2ERROR('No files found in ', os.environ['BELLE2_BACKGROUND_MIXING_DIR'])
    sys.exit()

# alternative: you can specify files explicitely
#
# dir = '/sw/belle2/bkg/'  # change the directory name if you don't run on KEKCC
# bg = [
#    dir + 'Coulomb_HER_100us.root',
#    dir + 'Coulomb_LER_100us.root',
#    dir + 'RBB_HER_100us.root',
#    dir + 'RBB_LER_100us.root',
#    dir + 'Touschek_HER_100us.root',
#    dir + 'Touschek_LER_100us.root',
#    ]
# change the file names if differ, remove some or add some more files

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# particle generator can be put here

# Mix beam background
bkgmixer = b2.register_module('BeamBkgMixer')
bkgmixer.param('backgroundFiles', bg)  # specify BG files
bkgmixer.param('components', ['CDC', 'TOP', 'ECL'])  # mix BG only for those components
bkgmixer.param('minTime', -5000)  # set time window start time [ns]
bkgmixer.param('maxTime', 10000)  # set time window stop time [ns]
bkgmixer.param('scaleFactors', [('Coulomb_LER', 1.05), ('Coulomb_HER', 1.08), ('RBB_LER', 0.8)])  # scale rates of some backgrounds
main.add_module(bkgmixer)

# FullSim, digitizers, clusterizers and reconstruction modules can be put here

# Output
output = b2.register_module('RootOutput')
output.param('outputFileName', 'testMixer.root')
main.add_module(output)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
