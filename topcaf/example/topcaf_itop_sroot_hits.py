#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import argparse
import os.path as path
import os

parser = argparse.ArgumentParser(description='Go through a data file, apply calibration, and write the waveforms to a root file.',
                                 usage='%(prog)s [options]')

parser.add_argument(
    '--inputRun',
    metavar='InputRun (i.e. file name = InputRun.dat)',
    required=True,
    help='the name for the input data files.')

parser.add_argument(
    '-t',
    '--topConfiguration',
    required=False,
    default=path.join(os.environ['BELLE2_LOCAL_DIR'], 'topcaf/data/TopConfigurations.root'),
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
    required=False,
    help='the path/name for the input sample calibration file. This parameter is REQUIRED.')

parser.add_argument('--Conditions', action='store_true',
                    help='Use the conditions service to store this calibration.'
                    )

args = parser.parse_args()

if args.Conditions:
    print('Using conditions service with IOVi = ' + args.IOVi + ' and IOVf = ' + args.IOVf)
    Conditions = 1
else:
    print('Not using conditions service.')
    Conditions = 0

if args.output:
    outputFile = args.output
else:
    outputFile = args.inputRun.replace('.sroot', '_writehits.root')
print('Writing output root file to ' + outputFile)


SRootReader = register_module('SeqRootInput')
SRootReader.param('inputFileName', args.inputRun)

itopconfig = register_module('TopConfiguration')
itopconfig.param('filename', args.topConfiguration)

itopeventconverter = register_module('iTopRawConverterSRoot')
itopeventconverter.param('forceTrigger0xF', True)

histomanager = register_module("HistoManager")

output = register_module('RootOutput')
outputDict = {'outputFileName': outputFile,
              'excludeBranchNames': ["EventWaveformPackets", "RawDataBlocks"]}
output.param(outputDict)

pedmodule = register_module('Pedestal')
pedmodule.param('mode', 1)
if args.ped:
    pedmodule.param('inputFileName', args.ped)
    pedmodule.param('conditions', 0)
else:
    pedmodule.param('conditions', 1)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTimingFast')
timeDict = {'time2TDC': 1.0}
timemodule.param(timeDict)
timemodule.param('threshold', 50.)  # always
timemodule.param('threshold_n', -300.)  # must be -150 for "small calpulse"


timecalibmodule = register_module('DoubleCalPulse')
timecalibmodule.param('calibrationWidthMax', 20)
timecalibmodule.param('calibrationWidthMin', 8)
timecalibmodule.param('calibrationADCThresholdMaxNeg', -300)  # must be -150 for "small calpulse"
timecalibmodule.param('calibrationADCThresholdMinNeg', -800)
timecalibmodule.param('calibrationADCThresholdMax', 850)
timecalibmodule.param('calibrationADCThresholdMin', 450)


main = create_path()
main.add_module(SRootReader)
main.add_module(histomanager)

main.add_module(itopconfig)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
main.add_module(mergemodule)
main.add_module(timemodule)
main.add_module(timecalibmodule)
# main.add_module(sampletimemodule)

main.add_module(output)
process(main)
