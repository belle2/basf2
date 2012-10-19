#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This example steering file generates readout frames (ROFs) for the SVD.
#
# This is phase one of the processing underlying the background mixing. You have
# to produce a "catalogue" of ROFs for individual background/generator combinations.
# The produced ROF files contain collections of background SimHits to be mixed
# into events of simulated data by the MixBkg module - this is phase 2 of the
# background mixing-related processing.
# The ROF file has a special format, different from that of ROOT files produced by
# the ROOTOutput module, They are read and used by the MixBkg module, not the
# ROOTInput module.
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
    'C3DC': 3,
    'TOP': 4,
    'ARICH': 5,
    'ECL': 6,
    'EKLM': 7,
    'BKLM': 8,
    }

# *******************************************************************************
# USER SETTINGS
subdetectorName = 'SVD'
bgType = 'RBB'
bgSource = 'HER'
bgGenerator = 'BBBREMS'

inputDir = '~/work/belle2/BG/summer2012'
# A single background file !
inputName = '{d}/output_{t}_{s}_*.root'.format(d=inputDir, t=bgType,
        s=bgSource)
bgTime = 20  # us

outputDir = '.'
outputName = '{d}/rof_{det}_{t}_{s}.root'.format(d=outputDir,
        det=subdetectorName, t=bgType, s=bgSource)
windowStart = -150  # ns
windowSize = 330  # ns
# *******************************************************************************

# Register modules

# ROOTInput module
rootinput = register_module('RootInput')
# CHANGE THIS TO POINT TO THE APPROPRIATE FILE(S) ON YOUR FILESYSTEM!!!
rootinput.param('inputFileName', inputName)
rootinput.param('treeName', 'tree')
# rootinput.set_log_level(LogLevel.INFO)

# ROFBulder module
rofbuilder = register_module('ROFBuilder')
rofbuilder.param('Subdetector', subdetectorCodes[subdetectorName])
rofbuilder.param('SimHitCollectionName', subdetectorName + 'SimHits')
rofbuilder.param('SimHitMCPartRelationName', 'MCParticlesTo' + subdetectorName
                 + 'SimHits')
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
# Set this to True only if RBB data have to be over-used, ie, used to generate
# more frames than their nominal number.
rofbuilder.param('RandomizeNonSAD', False)  # This is the default
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
