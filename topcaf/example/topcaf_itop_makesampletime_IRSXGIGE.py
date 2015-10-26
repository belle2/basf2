#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys
import argparse

parser = argparse.ArgumentParser(
    description='Calculate the sample to sample time calibration (for <=IRS3C).' +
                ' Typically, an electronic pulser run that includes data for all' +
                ' channels is used as the input to create this calibration.',
    usage='%(prog)s [options]')

parser.add_argument('--ped', metavar='PedestalFile (path/filename)',
                    required=True,
                    help='The pedestal file and path to be used for the analysis.' +
                         ' If PedestalFile == Conditions, then DB used. This parameter is REQUIRED.'
                    )

parser.add_argument(
    '--inputRun',
    metavar='InputRun (i.e. file name = InputRun.dat)',
    required=True,
    help='the root name for the input data files.  myInputRun.dat would have a root ' +
         'name of myInputRun.  This parameter is REQUIRED.')

parser.add_argument('--inputDir', metavar='InputDirectory (path)',
                    required=True,
                    help='the path to the data files (IRS and CAMAC) used for the analysis. This parameter is REQUIRED.'
                    )

parser.add_argument('--Output', metavar='Output File (path/filename)',
                    help='the output file name.  A default based on the input will be created if this argument is not supplied'
                    )

parser.add_argument('--Conditions', action='store_true',
                    help='Use the conditions service to store this calibration.'
                    )

parser.add_argument('--IOVi', metavar='Run_initial', default='NULL',
                    help='The initial run ID in the conditions service interval of validity.'
                    )

parser.add_argument('--IOVf', metavar='Run_final', default='NULL',
                    help='The final run ID in the conditions service interval of validity.'
                    )

args = parser.parse_args()

print 'pedestal file = ' + args.ped
print 'data file     = ' + args.inputDir + args.inputRun + '.dat'
print 'camac file    = ' + args.inputDir + args.inputRun + '.cmc'

if args.Conditions:
    print 'Using conditions service with IOVi = ' + args.IOVi + ' and IOVf = ' \
        + args.IOVf
    Conditions = 1
else:
    print 'Not using conditions service.'
    Conditions = 0

WriteFile = 1
if args.Output:
    OutputFile = args.Output
else:
    OutputFile = args.inputRun + '_SampleCalibration.root'
print 'Writing output calibration file to ' + OutputFile

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

output = register_module('RootOutput')

# run_info = args.inputRun.split('-')
# conditionsDict = {'experimentName': run_info[0] + '-' + run_info[2][0:7],
#                  'runName': run_info[2][7:14],
#                  'globalTag': 'leps2013_InitialTest_GlobalTag',
#                  'fileBaseName': 'http://belle2db.hep.pnnl.gov/'}
# conditions = register_module('Conditions')
# conditions.param(conditionsDict)

# register topcaf modules
itopconfig = register_module('TopConfiguration')
itopeventconverter = register_module('iTopRawConverterV2')
itopeventconverter.param('inputFileName', args.inputRun + '.dat')
itopeventconverter.param('inputDirectory', args.inputDir)
itopeventconverter.param('scrod', 16)
itopeventconverter.param('carrier', 0)
itopeventconverter.param('boardstack', 0)


camacconverter = register_module('AsciiCamac')
camacDict = {
    'inputFilename': args.inputDir + args.inputRun + '.cmc',
}
camacconverter.param(camacDict)

pedmodule = register_module('Pedestal')
if args.ped == 'Conditions':
    pedestalDict = {
        'mode': 1,
        'writeFile': 0,
        'conditions': 1,
    }
else:
    pedestalDict = {
        'mode': 1,
        'writeFile': 0,
        'conditions': 0,
        'inputFileName': args.ped,
    }
pedmodule.param(pedestalDict)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTiming')
timeDict = {'time2TDC': 40.0}
timemodule.param(timeDict)

sampletimemodule = register_module('SampleTimeCalibration')

sampletimeDict = {
    'outputFileName': OutputFile,
    'mode': 0,
    'writeFile': 1,
    'conditions': Conditions,
    'iovInitialRunID': args.IOVi,
    'iovFinalRunID': args.IOVf,
}
sampletimemodule.param(sampletimeDict)

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
# main.add_module(conditions)
# main.add_module(itopconfig)
main.add_module(itopeventconverter)
main.add_module(camacconverter)
main.add_module(pedmodule)
main.add_module(mergemodule)
main.add_module(timemodule)
main.add_module(sampletimemodule)
main.add_module(output)  # this is for debugging using values of individual events.

process(main)
