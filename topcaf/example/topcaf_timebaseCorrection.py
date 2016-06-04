#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import argparse
import os.path as path
import os

parser = argparse.ArgumentParser(description='Go through a data file, apply calibration, and write the waveforms to a root file.',
                                 usage='%(prog)s [options]')

parser.add_argument(
    '-t',
    '--topConfiguration',
    required=False,
    default=path.join(os.environ['BELLE2_LOCAL_DIR'], 'topcaf/data/TopConfigurations_large.root'),
    help="Path name of top configuration root file, e.g. ../data/TopConfigurations.root")

parser.add_argument(
    '--ped',
    metavar='PedestalFile (path/filename)',
    required=False,
    help='The pedestal file and path to be used for the analysis. If not specified, will use the conditions service.')

parser.add_argument('--output', metavar='Output File (path/filename)',
                    help='the output file name.  A default based on the input will be created if this argument is not supplied')

parser.add_argument(
    '--timeCalibFile',
    metavar='Time Calibration File (i.e. file name = /path/to/xxx_SampleCalibration.root)',
    default="../data/SampleCalibration.root",
    required=False,
    help='the path/name for the input sample calibration file. This parameter is REQUIRED.')


args = parser.parse_args()
if args.output:
    outputFile = args.output
else:
    outputFile = 'calibrated.root'
print('Writing output root file to ' + outputFile)

itopconfig = register_module('TopConfiguration')
itopconfig.param('filename', args.topConfiguration)

histomanager = register_module("HistoManager")

output = register_module('RootOutput')
outputDict = {'outputFileName': outputFile,
              'excludeBranchNames': []}
output.param(outputDict)

# timecalibmodule = register_module('DoubleCalPulseV5')

sampletimemodule = register_module('SampleTimeCalibrationV5')

sampletimeDict = {
    'inputFileName': args.timeCalibFile,
    'mode': 0,
    'writeFile': 1,
    'conditions': 0
}
sampletimemodule.param(sampletimeDict)

inputmodule = register_module("RootInput")

main = create_path()
main.add_module(inputmodule)
main.add_module(histomanager)

# main.add_module(itopconfig)
# main.add_module(timecalibmodule)
main.add_module(sampletimemodule)

# main.add_module(output)
process(main)
