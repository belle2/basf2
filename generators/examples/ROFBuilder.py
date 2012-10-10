#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This example steering file generates fake ROFs for the SVD.
# The ROFs are not actually usable, the purpose is to demosntrate the use
# of the ROFBuilder module.
#
# This is phase one of the processing underlying the background mixing.
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

# Register modules

# ROOTInput module
rootinput = register_module('RootInput')
rootinput.param('inputFileName',
                '~/work/belle2/BG/summer2012/output_Touschek_LER_0.root')
rootinput.param('treeName', 'tree')
# rootinput.set_log_level(LogLevel.INFO)

# ROFBulder module
rofbuilder = register_module('ROFBuilder')
rofbuilder.param('Subdetector', 2)  # SVD
rofbuilder.param('SimHitCollectionName', 'SVDSimHits')
rofbuilder.param('SimHitMCPartRelationName', 'MCParticlesToSVDSimHits')
rofbuilder.param('TimeAwareMode', True)
rofbuilder.param('WindowStart', -150.0)  # ns
rofbuilder.param('WindowSize', 330)  # ns
rofbuilder.param('BaseSampleSize', 20)  # us
rofbuilder.param('OutputRootFileName', 'SVDROFs.root')
rofbuilder.param('ComponentName', 'Touschek')
rofbuilder.param('GeneratorName', 'SAD_LER')
rofbuilder.param('MCParticleWriteMode', 0)
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
