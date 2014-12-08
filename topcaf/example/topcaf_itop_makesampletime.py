#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

output = register_module('RootOutput')

# register topcaf modules

if len(sys.argv) != 6:
    print 'Usage: basf2 topcaf_itop_makesampletime.py -n <number of events> --arg <Input Data Filename> <Input Camac Filename> <Path to Files> <Initial run for interval of validity> <Final run for interval of validity>'
    print '               NULL for IOV if not known/desired'
    sys.exit()

itopeventconverter = register_module('iTopRawConverter')
itopeventconverter.param('InputFileName', str(sys.argv[1]))
itopeventconverter.param('InputDirectory', str(sys.argv[3]))

camacconverter = register_module('Camac')
camacDict = {  # leps june 2013 itop test beam
               # leps june 2013 itop test beam
               # leps june 2013 itop test beam
    'InputFilename': str(sys.argv[3]) + str(sys.argv[2]),
    'CrateID': 13369927,
    'FTSWslot': 7,
    'FTSWword': 3,
    }
camacconverter.param(camacDict)

pedmodule = register_module('Pedestal')
pedestalDict = {'Mode': 1, 'WriteFile': 0, 'Conditions': 1}
pedmodule.param(pedestalDict)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTiming')
timeDict = {'Time2TDC': 40.0}
timemodule.param(timeDict)

sampletimemodule = register_module('SampleTimeCalibration')
sampletimeDict = {
    'OutputFileName': 'test.root',
    'Mode': 0,
    'WriteFile': 1,
    'Conditions': 0,
    'IOV_initialRun': str(sys.argv[4]),
    'IOV_finalRun': str(sys.argv[5]),
    }
sampletimemodule.param(sampletimeDict)

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
main.add_module(camacconverter)
main.add_module(pedmodule)
main.add_module(mergemodule)
main.add_module(timemodule)
main.add_module(sampletimemodule)
# main.add_module(output) # this is for debugging using values of individual events.

process(main)

