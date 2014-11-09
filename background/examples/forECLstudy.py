#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import glob

# ----------------------------------------------------------------------------------
# This example is maybe useful for tuning ECL digitizer on pure BG.
# If needed, ParticleGun and FullSim can be added as marked below
# ----------------------------------------------------------------------------------

set_log_level(LogLevel.INFO)

# background (collision) files
#   wildcards can be used if there are several files of the same BG type
#   example: 'Coulomb_HER_*.root' if there are several files for Coulomb_HER

dir = '/sw/belle2/bkg/'  # change the directory name if you don't run on KEKCC
bg = [
    dir + 'Coulomb_HER_100us.root',
    dir + 'Coulomb_LER_100us.root',
    dir + 'RBB_HER_100us.root',
    dir + 'RBB_LER_100us.root',
    dir + 'Touschek_HER_100us.root',
    dir + 'Touschek_LER_100us.root',
    ]
  # change the file names if differ, remove some or add some more files

# alternatively glob.glob can be used to get the list of files:
# (the directory must include only BG files!)
# bg = glob.glob(dir + '/*.root')

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# if needed, ParticleGun can be added here

# Mix beam background with wide time window for ECL
bkgmixer = register_module('BeamBkgMixer')
bkgmixer.param('backgroundFiles', bg)  # specify BG files
bkgmixer.param('components', ['ECL'])  # mix BG only for ECL
bkgmixer.param('minTime', -4000)  # set time window start time [ns]
bkgmixer.param('maxTime', 16000)  # set time window stop time [ns]
main.add_module(bkgmixer)

# with ParticleGun in the path, FullSim must be added here

# ECL digitization
ecl_digitizer = register_module('ECLDigitizer')
main.add_module(ecl_digitizer)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'forECLstudy.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics
