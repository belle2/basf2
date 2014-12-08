#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

output = register_module('RootOutput')

# register topcaf modules

if len(sys.argv) == 3:
    IOVinitial = 'NULL'
    IOVfinal = 'NULL'
elif len(sys.argv) == 4:
    IOVinitial = str(sys.argv[4])
    IOVfinal = 'NULL'
elif len(sys.argv) == 5:
    IOVinitial = str(sys.argv[4])
    IOVfinal = str(sys.argv[5])
else:
    print 'Usage: basf2 topcaf_itop_makesampletime.py -n <number of events> --arg <Path to Files> <Input Data Rootname> <Initial run for interval of validity> <Final run for interval of validity>'
    print '               IOV inputs are optional, can also be set to NULL'
    sys.exit()

itopeventconverter = register_module('iTopRawConverter')
itopeventconverter.param('InputFileName', str(sys.argv[2]) + '.dat')
itopeventconverter.param('InputDirectory', str(sys.argv[1]))

camacconverter = register_module('Camac')
camacDict = {  #             'CrateID'            : 13369927,  # leps june 2013 itop test beam
               #             'FTSWslot'           : 7,  # leps june 2013 itop test beam
               #             'FTSWword'           : 3 } # leps june 2013 itop test beam
               # 2014 itop crt
               # 2014 itop crt
               # 2014 itop crt
    'InputFilename': str(sys.argv[1]) + str(sys.argv[2]) + '.cmc',
    'CrateID': 0x00cc0200,
    'FTSWslot': 6,
    'FTSWword': 5,
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
    'IOV_initialRun': IOVinitial,
    'IOV_finalRun': IOVfinal,
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

