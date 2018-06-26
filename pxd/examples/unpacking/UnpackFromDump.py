#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

from basf2 import Module

rawinput = register_module('PXDReadRawONSEN')
rawinput.param('FileName', 'map.dat')

rawinput2 = register_module('PXDReadRawONSEN')
rawinput2.param('SetEvtMeta', False)
rawinput2.param('FileName', 'map2.dat')

unpacker = register_module('PXDUnpacker')
# unpacker.param('DoNotStore',True);
# unpacker.param('HeaderEndianSwap',False);

histoman = register_module('HistoManager')
histoman.param('histoFileName', 'map_HISTO.root')

simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'map_RAW.root')
simpleoutput.param('compressionLevel', 0)

main = create_path()

main.add_module(rawinput)
main.add_module(rawinput2)
main.add_module(histoman)
main.add_module(unpacker)
main.add_module('PXDRawDQM')
main.add_module('PXDROIDQM')
main.add_module('Progress')
main.add_module(simpleoutput)

process(main)
