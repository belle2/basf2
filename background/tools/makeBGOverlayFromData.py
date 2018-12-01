#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from rawdata import add_unpackers
from ROOT import Belle2

# -----------------------------------------------------------------------------------
# Make data sample for BG overlay from experimental raw data
#
# Usage: basf2 makeBGOverlayFromData.py -i <inputFileName> -o <outputFileName>
#   <inputFileName> must be raw data in a root format
#
# Note: use proper global tag!
# -----------------------------------------------------------------------------------

# define global tag
globalTag = 'data_reprocessing_prod6'


class SelectTRGTypeRandom(Module):
    ''' select random triggered events from TRGSummary '''

    def event(self):
        ''' event processing '''

        evtmetadata = Belle2.PyStoreObj('EventMetaData')

        trg_summary = Belle2.PyStoreObj('TRGSummary')
        if not trg_summary.isValid():
            B2ERROR('No TRGSummary available - event ignored')
            self.return_value(0)
            return

        # to be replaced with TTYP_DPHY when those triggers implemented
        if trg_summary.getTimType() == Belle2.TRGSummary.TTYP_RAND:
            self.return_value(1)
        else:
            self.return_value(0)


class AdjustTOPDigitsTime(Module):
    ''' adjust time of TOPDigits: a temporary solution for exp 3 '''

    def event(self):
        ''' event processing '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            digit.subtractT0(330.0)  # set t = 0 to the average bunch crossing time


use_central_database(globalTag)

# Create path
main = create_path()
emptypath = create_path()

# Input: raw data
roinput = register_module('RootInput')
main.add_module(roinput)

# Unpackers
add_unpackers(main)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Select random trigger events
selector = SelectTRGTypeRandom()
main.add_module(selector)
selector.if_false(emptypath)

# Convert ECLDsps to ECLWaveforms
compress = register_module('ECLCompressBGOverlay')
main.add_module(compress, CompressionAlgorithm=3)
compress.if_false(emptypath)

# Adjust time of TOPDigits
main.add_module(AdjustTOPDigitsTime())

# Output: digitized hits only
output = register_module('RootOutput')
output.param('branchNames', ['PXDDigits', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
                             'ARICHDigits', 'ECLWaveforms', 'BKLMDigits', 'EKLMDigits'])
main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
