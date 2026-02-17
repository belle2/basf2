#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Steering file for producing summary ntuple of beam background hit rates
#
# usage: basf2 beamBkgHitRates.py expNo runNo globalTag [outputFolder]
# ---------------------------------------------------------------------------------------

import basf2
import sys
import glob
from background import get_trigger_types_for_bgo, SelectTRGTypes
from rawdata import add_unpackers
from svd import add_svd_reconstruction

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "expNo runNo globalTag [outputFolder]")
    sys.exit()

expNo = 'e' + '{:0=4d}'.format(int(argvs[1]))
runNo = 'r' + '{:0=5d}'.format(int(argvs[2]))
global_tag = argvs[3]

indir = '/hsm/belle2/bdata/Data/Raw/' + expNo + '/' + runNo + '/sub00'
files = sorted(glob.glob(indir + '/*.root'))
if len(files) == 0:
    basf2.B2ERROR('No files found in ' + indir)
    sys.exit()

outdir = '.'
if len(argvs) > 4:
    outdir = argvs[4]
outputFile = outdir + '/beamBkgHitRates-' + expNo + '-' + runNo + '.root'

# Define global tag
basf2.conditions.prepend_globaltag(global_tag)

# Create paths
main = basf2.create_path()
emptypath = basf2.create_path()

# Input (raw data)
main.add_module('RootInput', inputFileNames=files)

# Gearbox
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Unpack TRGSummary
main.add_module('TRGGDLUnpacker')
main.add_module('TRGGDLSummary')

# Show progress of processing
main.add_module('Progress')

# Skip events not matching given trigger types, if the list is not empty
trg_types = get_trigger_types_for_bgo()
selector = main.add_module(
    SelectTRGTypes(trg_types=trg_types)
)
selector.if_false(emptypath)

# Unpack detector data
add_unpackers(path=main,
              components=['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM'])

# additional modules if needed for hit processing
main.add_module('ARICHFillHits')
main.add_module('TOPChannelMasker')
main.add_module('ActivatePXDGainCalibrator')
main.add_module('PXDClusterizer')
add_svd_reconstruction(main, applyMasking=True)
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigitsUnmasked',
    ShaperDigitsIN='SVDShaperDigitsZS5')
main.add_module('TFCDC_WireHitPreparer',
                wirePosition='aligned',
                flightTimeEstimation='outwards',
                filter='cuts_from_DB')

# Beam background rate monitor: output to flat ntuple
main.add_module(
    'BeamBkgHitRateMonitor',
    outputFileName=outputFile,
    trgTypes=trg_types,
    svdShaperDigitsName='SVDShaperDigitsZS5',
    cdcTimeWindowLowerEdgeSmallCell=4720,
    cdcTimeWindowUpperEdgeSmallCell=5020,
    cdcTimeWindowLowerEdgeNormalCell=4720,
    cdcTimeWindowUpperEdgeNormalCell=5020)

# Process events
basf2.process(main)
