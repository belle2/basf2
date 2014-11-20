#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys

eventinfosetter = register_module('EventInfoSetter')
# eventinfosetter.param('expList', [900000001])
# eventinfosetter.param('evtNumList', [100000])
eventinfoprinter = register_module('EventInfoPrinter')

# register topcaf modules

if len(sys.argv) != 5:
    print 'Usage: basf2 topcaf_itop_makeped.py -n <number of events> --arg <Input Pedestal Filename> <Path to Pedestal File> <Initial run for interval of validity> <Final run for interval of validity>'
    print '         NULL for IOV if not known/desired'
    sys.exit()

itopeventconverter = register_module('iTopRawConverter')
itopeventconverter.param('InputFileName', str(sys.argv[1]))
itopeventconverter.param('InputDirectory', str(sys.argv[2]))

pedmodule = register_module('Pedestal')
pedestalDict = {
    'OutputFileName': 'test.root',
    'Mode': 0,
    'WriteFile': 1,
    'Conditions': 1,
    'IOV_initialRun': str(sys.argv[3]),
    'IOV_finalRun': str(sys.argv[4]),
    }
pedmodule.param(pedestalDict)

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
process(main)

