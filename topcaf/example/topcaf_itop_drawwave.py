#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import argparse

parser = argparse.ArgumentParser(description='Go through a data file, apply calibration, and write the waveforms to a root file.',
                                 usage='%(prog)s [options]')

parser.add_argument(
    '--inputRun',
    metavar='InputRun (i.e. file name = InputRun.dat)',
    required=True,
    help='the root name for the input data files.  myInputRun.dat would have a root name of myInputRun. REQUIRED')

parser.add_argument('--inputDir', metavar='InputDirectory (path)', required=True,
                    help='the path to the data files (IRS and CAMAC) used for the analysis. This parameter is REQUIRED.')

parser.add_argument(
    '--ped',
    metavar='PedestalFile (path/filename)',
    required=False,
    help='The pedestal file and path to be used for the analysis. If not specified, will use the conditions service.')

parser.add_argument(
    '--s2s',
    metavar='SampleToSampleFile (path/filename)',
    help='The sample to sample calibration file and path to be used for the analysis. \'Conditions\' to try database.')

parser.add_argument('--Output', metavar='Output File (path/filename)',
                    help='the output file name.  A default based on the input will be created if this argument is not supplied')

args = parser.parse_args()

if args.ped:
    print 'pedestal file = ' + args.ped
else:
    print 'using conditions service for pedestal calibration'

print 'data file     = ' + args.inputDir + args.inputRun + '.dat'
print 'camac file    = ' + args.inputDir + args.inputRun + '.cmc'

WriteFile = 1
if args.Output:
    OutputFile = args.Output
else:
    OutputFile = args.inputRun + '_drawwave.root'
print 'Writing output root file to ' + OutputFile

histomanager = register_module("HistoManager")

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

run_info = args.inputRun.split('-')
conditionsDict = {'ExperimentName': run_info[0] + '-' + run_info[2][0:7],
                  'RunName': run_info[2][7:14],
                  'GlobalTag': 'leps2013_InitialTest_GlobalTag',
                  'FileBaseName': 'http://belle2db.hep.pnnl.gov/'}
conditions = register_module('Conditions')
conditions.param(conditionsDict)


output = register_module('RootOutput')
outputDict = {'outputFileName': OutputFile}
output.param(outputDict)

# register topcaf modules
itopconfig = register_module('TopConfiguration')
itopeventconverter = register_module('iTopRawConverter')
evtconvDict = {'InputDirectory': args.inputDir,
               'InputFileName': args.inputRun + '.dat'}
itopeventconverter.param(evtconvDict)

pedmodule = register_module('Pedestal')
pedmodule.param('Mode', 1)
if args.ped:
    pedmodule.param('InputFileName', args.ped)
    pedmodule.param('Conditions', 0)
else:
    pedmodule.param('Conditions', 1)

camacconverter = register_module('Camac')
camacDict = {'InputFilename': args.inputDir + args.inputRun + ".cmc",

             'CrateID': 13369927,  # leps june 2013 itop test beam
             'FTSWslot': 7,  # leps june 2013 itop test beam
             'FTSWword': 3  # leps june 2013 itop test beam

             #             'CrateID'            : 0x00cc0200,  # 2014 itop crt
             #             'FTSWslot'           : 6,  # 2014 itop crt
             #             'FTSWword'           : 5  # 2014 itop crt

             }
camacconverter.param(camacDict)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTiming')
timeDict = {'Time2TDC': 40.0}
timemodule.param(timeDict)

if args.s2s:
    sampletimemodule = register_module('SampleTimeCalibration')
    if args.s2s == 'Conditions':
        sampletimeDict = {'Mode': 1,
                          'WriteFile': 0,
                          'Conditions': 1}
    else:
        sampletimeDict = {'InputFileName': args.s2s,
                          'Mode': 1,
                          'WriteFile': 0,
                          'Conditions': 0}

    sampletimemodule.param(sampletimeDict)

main = create_path()
main.add_module(histomanager)
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(conditions)
main.add_module(itopconfig)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
main.add_module(camacconverter)
main.add_module(mergemodule)
main.add_module(timemodule)

if args.s2s:
    main.add_module(sampletimemodule)

main.add_module(output)

process(main)
