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


input = b2.register_module('SeqRootInput')

unpacker = b2.register_module('PXDUnpacker')
# unpacker.param('DoNotStore',True);

histoman = b2.register_module('HistoManager')
histoman.param('histoFileName', 'your_histo_file.root')

simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', 'PXDRawHit.root')
simpleoutput.param('compressionLevel', 0)

main = b2.create_path()

main.add_module(input)
main.add_module(histoman)
main.add_module(unpacker)
main.add_module(b2.register_module('PXDRawDQM'))
main.add_module(b2.register_module('PXDROIDQM'))
main.add_module(b2.register_module('Progress'))
main.add_module(simpleoutput)

b2.process(main)
