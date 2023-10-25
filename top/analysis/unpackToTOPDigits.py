#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Unpack raw data to TOPDigits
# Usage: basf2 unpackToTOPDigits.py -i <input_file.root> -o <output_file.root>
# ---------------------------------------------------------------------------------------

import basf2 as b2

# Define a global tag
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('online')

# Create path
main = b2.create_path()

# input
# roinput = b2.register_module('SeqRootInput')  # sroot files
roinput = b2.register_module('RootInput')  # root files
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
main.add_module(converter)

# output
output = b2.register_module('RootOutput')
output.param('branchNames', ['TOPDigits', 'TOPRawDigits', 'TOPProductionEventDebugs',
                             'TOPProductionHitDebugs', 'TOPRawDigitsToTOPProductionHitDebugs',
                             # 'TOPRawWaveforms', 'TOPRawDigitsToTOPRawWaveforms',
                             ])
main.add_module(output)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
