#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Steering file for producing summary ntuple of beam background hit rates
#
# usage: basf2 beamBkgHitRates.py expNo runNo globalTag [outputFolder]
# ---------------------------------------------------------------------------------------

import basf2
import sys
import glob
from rawdata import add_unpackers
from svd import add_svd_reconstruction
from ROOT import Belle2

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "expNo runNo globalTag [outputFolder]")
    sys.exit()

expNo = 'e' + '{:0=4d}'.format(int(argvs[1]))
runNo = 'r' + '{:0=5d}'.format(int(argvs[2]))
global_tag = argvs[3]
trigTypes = [5, 7]  # trigger types for event selection (see TRGSummary.h)

indir = '/hsm/belle2/bdata/Data/Raw/' + expNo + '/' + runNo + '/sub00'
files = sorted(glob.glob(indir + '/*.root'))
if len(files) == 0:
    basf2.B2ERROR('No files found in ' + indir)
    sys.exit()

outdir = '.'
if len(argvs) > 4:
    outdir = argvs[4]
outputFile = outdir + '/beamBkgHitRates-' + expNo + '-' + runNo + '.root'


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
if len(trigTypes) > 0:
    selector = SelectTRGTypes()
    main.add_module(selector)
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
    trgTypes=trigTypes,
    svdShaperDigitsName='SVDShaperDigitsZS5',
    cdcTimeWindowLowerEdgeSmallCell=4720,
    cdcTimeWindowUpperEdgeSmallCell=5020,
    cdcTimeWindowLowerEdgeNormalCell=4720,
    cdcTimeWindowUpperEdgeNormalCell=5020)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
