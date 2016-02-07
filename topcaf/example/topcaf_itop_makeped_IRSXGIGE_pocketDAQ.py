#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys

import argparse

parser = argparse.ArgumentParser(
    description='Calculate the pedestal calibration. Typically, a dedicated pedestal' +
                ' run that includes data for all channels is used as the input to create this calibration.',
    usage='basf2 %(prog)s -n <number of events> --arg [option1] --arg [option2]')

parser.add_argument(
    '--inputRun',
    metavar='InputRun (i.e. file name = InputRun.dat)',
    required=True,
    help='the root name for the input data files.  myInputRun.dat would have' +
         ' a root name of myInputRun.  This parameter is REQUIRED.')

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
if not args.inputDir.endswith('/'):
    args.inputDir += '/'


print('data file     = ' + args.inputDir + args.inputRun + '.dat')

if args.Conditions:
    print('Using conditions service with IOVi = ' + args.IOVi + ' and IOVf = ' +
          args.IOVf)
    Conditions = 1
else:
    print('Not using conditions service.')
    Conditions = 0

WriteFile = 1
if args.Output:
    OutputFile = args.Output
else:
    OutputFile = args.inputRun + '_PedestalCalibration.root'
print('Writing output calibration file to ' + OutputFile)

eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

# register topcaf modules

itopeventconverter = register_module('iTopRawConverterV3')
itopeventconverter.param('inputFileName', args.inputRun + '.dat')
itopeventconverter.param('inputDirectory', args.inputDir)

pedmodule = register_module('Pedestal')
pedestalDict = {
    'outputFileName': OutputFile,
    'mode': 0,
    'writeFile': WriteFile,
    'conditions': Conditions,
    'iovInitialRunID': args.IOVi,
    'iovFinalRunID': args.IOVf,
}
pedmodule.param(pedestalDict)

main = create_path()
# main.add_module(eventinfosetter)
# main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
process(main)
