#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import sys
from rawdata import add_unpackers
from ROOT import Belle2

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

globalTag = argvs[1]
trigTypes = [5, 7]  # trigger types for event selection (see TRGSummary.h)


class SelectTRGTypes(basf2.Module):
    ''' select events according to given trigger types '''

    def event(self):
        ''' event processing '''

        self.return_value(0)
        trg_summary = Belle2.PyStoreObj('TRGSummary')
        if not trg_summary.isValid():
            basf2.B2ERROR('No TRGSummary available - event ignored')
            return

        for trgType in trigTypes:
            if trg_summary.getTimType() == trgType:
                self.return_value(1)
                return


# Define global tag
basf2.conditions.prepend_globaltag(globalTag)

# Create paths
main = basf2.create_path()
emptypath = basf2.create_path()

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

# Select random trigger events
selector = SelectTRGTypes()
main.add_module(selector)
selector.if_false(emptypath)

# Unpack detector data
add_unpackers(main,
              components=['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM'])

# ECL trigger unpacker and BGOverlay dataobject
main.add_module('TRGECLUnpacker')
main.add_module('TRGECLBGTCHit')

# Convert ECLDsps to ECLWaveforms
compress = basf2.register_module('ECLCompressBGOverlay')
main.add_module(compress, CompressionAlgorithm=3)
compress.if_false(emptypath)

# Output: digitized hits only
output = basf2.register_module('RootOutput')
output.param('branchNames', ['EventLevelTriggerTimeInfo', 'PXDDigits', 'SVDShaperDigits', 'CDCHits', 'TOPDigits',
                             'ARICHDigits', 'ECLWaveforms', 'KLMDigits', 'TRGECLBGTCHits'])
main.add_module(output)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
