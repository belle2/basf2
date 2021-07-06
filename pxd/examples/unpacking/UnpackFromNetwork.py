#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2


deserializerpxd = b2.register_module('DeSerializerPXD')
deserializerpxd.param('Port', 1025)
deserializerpxd.param('HostName', '127.0.0.1')

unpacker = b2.register_module('PXDUnpacker')
unpacker.param('HeaderEndianSwap', False)

histoman = b2.register_module('HistoManager')
histoman.param('histoFileName', 'your_histo_file.root')

simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', 'PXDRawHit.root')
simpleoutput.param('compressionLevel', 0)

main = b2.create_path()

main.add_module(deserializerpxd)
main.add_module(histoman)
main.add_module(unpacker)
main.add_module('PXDRawDQM')
main.add_module('PXDROIDQM')
main.add_module('Progress')
main.add_module(simpleoutput)

b2.process(main)
