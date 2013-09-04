#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This example steering file generates readout frames (ROFs) for the SVD.
#
# This is phase one of the processing underlying the background mixing. A user
# has to produce a "catalog" of ROFs for individual background/generator
# combinations.
# The produced ROF files contain collections of background SimHits to be mixed
# into events of simulated data by the MixBkg module.
# NOTES
# 1. The event-randomization feature has been removed from ROFBuilder.
# To randomize the input event sequence, use the EventRandomizer module.
# 2. The ROF file has a special format, different from that of the ROOT files
# produced by the ROOTOutput module, They are read and used by the MixBkg
# module, not the ROOTInput module.
#
##############################################################################

from basf2 import *

# show warnings during processing
set_log_level(LogLevel.ERROR)

# NOTE: To produce ROFs for a subdetector, its SimHits have to inherit from
# SimHitBase classs (generators/dataobjects), and for timed background, they
# have to override the SimHitBase::shiftInTime method.
subdetectorCodes = {
    'PXD': 1,
    'SVD': 2,
    'CDC': 3,
    'TOP': 4,
    'ARICH': 5,
    'ECL': 6,
    'EKLM': 7,
    'BKLM': 8,
    'ECLsim': 9,
    }
# ECL is a special case, use 'ECL' to produce ROFs with ECLHits, 'ECLsim' for
# ROFs with ECLSimHits.
# ****************************************************************************
# USER SETTINGS
subdetectorName = 'SVD'
bgType = 'Touschek'
bgSource = 'LER'
bgGenerator = 'SAD'

# Replace this with paths on your own system!
inputDir = '/data/belle2/BG/summer2012'
# A single background file !
inputName = '{d}/output_{t}_{s}_*.root'.format(d=inputDir, t=bgType,
        s=bgSource)
bgTime = 80  # us

outputDir = '.'
outputName = '{d}/rof_{det}_{t}_{s}.root'.format(d=outputDir,
        det=subdetectorName, t=bgType, s=bgSource)
windowStart = -150  # ns
windowSize = 330  # ns

collectionName = subdetectorName + 'SimHits'
if subdetectorName == 'ECL':
    collectionName = 'ECLHits'
elif subdetectorName == 'ECLsim':
    collectionName = 'ECLSimHits'

# *****************************************************************************

# Register modules

# ROOTInput module
rootinput = register_module('RootInput')
# CHANGE THIS TO POINT TO THE APPROPRIATE FILE(S) ON YOUR FILESYSTEM!!!
rootinput.param('inputFileName', inputName)
# rootinput.set_log_level(LogLevel.INFO)

# ROFBulder module
rofbuilder = register_module('ROFBuilder')
rofbuilder.param('Subdetector', subdetectorCodes[subdetectorName])
rofbuilder.param('SimHitCollectionName', collectionName)
rofbuilder.param('SimHitMCPartRelationName', 'MCParticlesTo' + collectionName)
rofbuilder.param('TimeAwareMode', True)
# rofbuilder.param('EventsPerReadoutFrame',0.33)
rofbuilder.param('WindowStart', windowStart)  # ns
rofbuilder.param('WindowSize', windowSize)  # ns
rofbuilder.param('BaseSampleSize', bgTime)  # us
rofbuilder.param('OutputRootFileName', outputName)
rofbuilder.param('ComponentName', bgType)
rofbuilder.param('GeneratorName', bgGenerator + '_' + bgSource)
# With SimHits, save MCParticles that caused them and their predecessors.
rofbuilder.param('MCParticleWriteMode', 2)
rofbuilder.set_log_level(LogLevel.INFO)

# Show progress of processing
progress = register_module('Progress')

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(progress)
main.add_module(rootinput)
main.add_module(rofbuilder)

# Process events
process(main)

# Print call statistics
print statistics
#
