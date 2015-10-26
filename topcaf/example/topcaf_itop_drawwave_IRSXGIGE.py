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
# print 'camac file    = ' + args.inputDir + args.inputRun + '.cmc'

WriteFile = 1
if args.Output:
    OutputFile = args.Output
else:
    OutputFile = args.inputRun + '_drawwave.root'
print 'Writing output root file to ' + OutputFile

histomanager = register_module("HistoManager")

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

# run_info = args.inputRun.split('-')
# conditionsDict = {'experimentName': run_info[0] + '-' + run_info[2][0:7],
#                  'runName': run_info[2][7:14],
#                  'globalTag': 'leps2013_InitialTest_GlobalTag',
#                  'fileBaseName': 'http://belle2db.hep.pnnl.gov/'}
# conditions = register_module('Conditions')
# conditions.param(conditionsDict)


output = register_module('RootOutput')
outputDict = {'outputFileName': OutputFile}
output.param(outputDict)

# register topcaf modules
# itopconfig = register_module('TopConfiguration')
itopeventconverter = register_module('iTopRawConverterV2')
evtconvDict = {'inputDirectory': args.inputDir,
               'inputFileName': args.inputRun + '.dat'}
itopeventconverter.param(evtconvDict)
itopeventconverter.param('scrod', 16)
itopeventconverter.param('carrier', 0)
itopeventconverter.param('boardstack', 0)


pedmodule = register_module('Pedestal')
pedmodule.param('mode', 1)
if args.ped:
    pedmodule.param('inputFileName', args.ped)
    pedmodule.param('conditions', 0)
else:
    pedmodule.param('conditions', 1)

camacconverter = register_module('Camac')
camacDict = {'inputFilename': args.inputDir + args.inputRun + ".cmc",

             #             'crateID': 13369927,  # leps june 2013 itop test beam
             #             'ftswSlot': 7,  # leps june 2013 itop test beam
             #             'ftswWord': 3  # leps june 2013 itop test beam

             'crateID': 0x00cc0200,  # 2014 itop crt
             'ftswSlot': 6,  # 2014 itop crt
             'ftswWord': 5  # 2014 itop crt

             }
camacconverter.param(camacDict)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTiming')
timeDict = {'time2TDC': 40.0}
timemodule.param(timeDict)

if args.s2s:
    sampletimemodule = register_module('SampleTimeCalibration')
    if args.s2s == 'Conditions':
        sampletimeDict = {'mode': 1,
                          'writeFile': 0,
                          'conditions': 1}
    else:
        sampletimeDict = {'inputFileName': args.s2s,
                          'mode': 1,
                          'writeFile': 0,
                          'conditions': 0}

    sampletimemodule.param(sampletimeDict)

main = create_path()
main.add_module(histomanager)
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
# main.add_module(conditions)
# main.add_module(itopconfig)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
main.add_module(camacconverter)
main.add_module(mergemodule)
main.add_module(timemodule)

if args.s2s:
    main.add_module(sampletimemodule)

main.add_module(output)

process(main)
