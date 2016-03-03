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
    metavar='InputRun (i.e. file name = /path/to/InputRun.dat)',
    required=True,
    help='the name for the input data files.')

args = parser.parse_args()


SRootReader = register_module('SeqRootInput')
SRootReader.param('inputFileName', args.inputRun)

itopeventconverter = register_module('iTopRawConverterSRoot')
itopeventconverter.param('forceTrigger0xF', True)

pedmodule = register_module('Pedestal')
pedmodule.param('mode', 0)
pedmodule.param('writeFile', 1)
pedmodule.param('outputFileName', 'pedout.root')


main = create_path()
main.add_module(SRootReader)
main.add_module(itopeventconverter)
main.add_module(pedmodule)

process(main)
