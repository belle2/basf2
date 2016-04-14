#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

from basf2 import Module

rawinput = register_module('PXDReadRawONSEN')
rawinput.param('FileName', 'dhh_data_nonref.dat')

unpacker = register_module('PXDUnpacker')
# coment the next three lines in, in order to remap DHP data
# unpacker.param('RemapFlag',True)
# unpacker.param('RemapLUT_IF_OB','LUT_IF_OB.csv');
# unpacker.param('RemapLUT_IB_OF','LUT_IB_OF.csv');

# unpacker.param('DoNotStore',True);
# unpacker.param('HeaderEndianSwap',False);

histoman = register_module('HistoManager')
histoman.param('histoFileName', 'dhh_data_nonref_HISTO.root')

simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'dhh_data_nonref_RAW.root')
simpleoutput.param('compressionLevel', 0)

main = create_path()

main.add_module(rawinput)
main.add_module(histoman)
main.add_module(unpacker)
main.add_module(register_module('PXDRawDQM'))
main.add_module(register_module('PXDROIDQM'))
main.add_module(register_module('Progress'))
main.add_module(simpleoutput)

process(main)
