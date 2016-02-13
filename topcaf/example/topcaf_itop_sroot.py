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


args = parser.parse_args()

SRootReader = register_module('SeqRootInput')
SRootReader.param('inputFileName', args.inputRun)

itopconfig = register_module('TopConfiguration')
itopconfig.param('filename', args.topConfiguration)

itopeventconverter = register_module('iTopRawConverterSRoot')


main = create_path()
main.add_module(SRootReader)

main.add_module(itopeventconverter)
process(main)
