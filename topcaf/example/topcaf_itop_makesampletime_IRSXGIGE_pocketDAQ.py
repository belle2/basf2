#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys
import argparse
import os.path as path

parser = argparse.ArgumentParser(
    description='Calculate the sample to sample time calibration (for <=IRS3C).' +
                ' Typically, an electronic pulser run that includes data for all' +
                ' channels is used as the input to create this calibration.',
    usage='%(prog)s [options]')

parser.add_argument(
    '--inputFile',
    metavar='InputFile (i.e. file name = /path/to/xxx_writehits.root)',
    required=True,
    nargs='*',
    help='the path/name for the input data files. This parameter is REQUIRED.')

parser.add_argument('--Output', metavar='Output File (path/filename)',
                    help='the output file name.  A default based on the input will be created if this argument is not supplied'
                    )

parser.add_argument('--Conditions', action='store_true',
                    help='Use the conditions service to store this calibration.'
                    )

parser.add_argument(
    '-t',
    '--topConfiguration',
    required=False,
    default=path.join(os.environ['BELLE2_LOCAL_DIR'], 'topcaf/data/TopConfigurations.root'),
    help="Path name of top configuration root file, e.g. ../data/TopConfigurations.root")

args = parser.parse_args()

if args.Conditions:
    print('Using conditions service with IOVi = ' + args.IOVi + ' and IOVf = ' + args.IOVf)
    Conditions = 1
else:
    print('Not using conditions service.')
    Conditions = 0

WriteFile = 1
if args.Output:
    OutputFile = args.Output
else:
    OutputFile = 'SampleCalibration.root'
#    OutputFile = os.path.splitext(args.inputFile)[0] + '_SampleCalibration.root'

print('Writing output calibration file to ' + OutputFile)

input = register_module('RootInput')
input.param('inputFileNames', args.inputFile)

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

# output = register_module('RootOutput')
# outputDict = {'outputFileName': OutputFile,
#              'excludeBranchNames': ["EventWaveformPackets"]}
# output.param(outputDict)

# run_info = args.inputRun.split('-')
# conditionsDict = {'experimentName': run_info[0] + '-' + run_info[2][0:7],
#                  'runName': run_info[2][7:14],
#                  'globalTag': 'leps2013_InitialTest_GlobalTag',
#                  'fileBaseName': 'http://belle2db.hep.pnnl.gov/'}
# conditions = register_module('Conditions')
# conditions.param(conditionsDict)

# register topcaf modules
itopconfig = register_module('TopConfiguration')
itopconfig.param('filename', args.topConfiguration)

# itopeventconverter = register_module('iTopRawConverterV3')
# itopeventconverter.param('inputFileName', )
# itopeventconverter.param('inputDirectory', args.inputDir )


# pedmodule = register_module('Pedestal')
# if args.ped == 'Conditions':
#    pedestalDict = {
#        'mode': 1,
#        'writeFile': 0,
#        'conditions': 1,
#    }
# else:
#    pedestalDict = {
#        'mode': 1,
#        'writeFile': 0,
#        'conditions': 0,
#        'inputFileName': args.ped,
#    }
# pedmodule.param(pedestalDict)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTimingV2')
timeDict = {'time2TDC': 1.0}
timemodule.param(timeDict)

timecalibmodule = register_module('DoubleCalPulse')

sampletimemodule = register_module('SampleTimeCalibrationV2')

sampletimeDict = {
    'outputFileName': OutputFile,
    'mode': 0,
    'writeFile': 1,
    'conditions': Conditions,
}
sampletimemodule.param(sampletimeDict)

main = create_path()
main.add_module(input)
# main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
# main.add_module(conditions)
# main.add_module(itopconfig)
# main.add_module(itopeventconverter)
# main.add_module(pedmodule)
# main.add_module(mergemodule)
# main.add_module(timemodule)
# main.add_module(timecalibmodule)
main.add_module(sampletimemodule)
# main.add_module(output)  # this is for debugging using values of individual events.

process(main)
