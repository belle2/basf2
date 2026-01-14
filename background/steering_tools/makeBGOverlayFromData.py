##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys

import basf2
from background import get_trigger_types_for_bgo, SelectTRGTypes
from rawdata import add_unpackers

# --------------------------------------------------------------------------------------
# Make data sample for BG overlay from experimental raw data
#
# Usage: basf2 makeBGOverlayFromData.py -i <inputFileName> -o <outputFileName> globalTag
#
# --------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 2:
    print("usage: basf2", argvs[0], "globalTag -i <inputFileName> -o <outputFileName>")
    sys.exit()

# Define global tag
globalTag = argvs[1]
basf2.conditions.prepend_globaltag(globalTag)

# Create paths
main = basf2.Path()
emptypath = basf2.Path()

# Input: raw data
main.add_module('RootInput')

# Gearbox
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Unpack TRGSummary
main.add_module('TRGGDLUnpacker')
main.add_module('TRGGDLSummary')

# Show progress of processing
main.add_module('Progress')

# Select specific triggered events
trg_types = get_trigger_types_for_bgo()
selector = main.add_module(
    SelectTRGTypes(trg_types=trg_types)
)
selector.if_false(emptypath)

# Unpack detector data
add_unpackers(
    main,
    components=['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM']
)

# Convert ECLDsps to ECLWaveforms
compress = main.add_module(
    'ECLCompressBGOverlay',
    CompressionAlgorithm=3
)
compress.if_false(emptypath)

# Shift the time of KLMDigits
main.add_module('KLMDigitTimeShifter')

# ECL trigger unpacker and BGOverlay dataobject
main.add_module('TRGECLUnpacker')
main.add_module('TRGECLBGTCHit')

# Output: digitized hits only
main.add_module(
    'RootOutput',
    branchNames=['EventLevelTriggerTimeInfo', 'PXDDigits', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
                 'ARICHDigits', 'ECLWaveforms', 'KLMDigits', 'TRGECLBGTCHits', 'TRGSummary', 'TOPInjectionVeto']
)

# Process events
basf2.process(main)
