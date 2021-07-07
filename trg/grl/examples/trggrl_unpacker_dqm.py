#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -----------------------------------------------------------------------------------
#
#                        CDCT3D trigger Unpacker module
#
#    usage : %> basf2 TrgCdcT3dUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

import basf2 as b2
import os
from optparse import OptionParser
from reconstruction import add_cosmics_reconstruction
home = os.environ['BELLE2_LOCAL_DIR']

parser = OptionParser()
parser.add_option(
    '-f',
    '--file',
    dest='filename',
    default='hsm/belle2/bdata/Data/Raw/e0007/r01640/sub00/cosmic.0007.01640.HLT1.f00000.root')
parser.add_option('-o', '--output', dest='output', default='trggrl_unpacker.root')
parser.add_option('-t', '--tracking', dest='tracking', default=0)
parser.add_option('-g', '--gdl', dest='gdl', default=0)
(options, args) = parser.parse_args()

runID = str(options.run)

b2.set_log_level(b2.LogLevel.ERROR)

main = b2.create_path()

# input
input = b2.register_module('RootInput')
input.param('inputFileName', options.filename)

main.add_module(input)

histo = b2.register_module('HistoManager')
histo.param('histoFileName', "dqm.root")  # File to save histograms
main.add_module(histo)

unpacker = b2.register_module('TRGGRLUnpacker')
main.add_module(unpacker)

trggrldqm = b2.register_module('TRGGRLDQM')
main.add_module(trggrldqm)


if int(options.tracking):
    cdcunpacker = b2.register_module('CDCUnpacker')
    cdcunpacker.param('xmlMapFileName', "cdc/data/ch_map.dat")
    cdcunpacker.param('enablePrintOut', False)
    main.add_module(cdcunpacker)
    add_cosmics_reconstruction(main, 'CDC', False)


if int(options.gdl):
    trggdlUnpacker = b2.register_module("TRGGDLUnpacker")
    main.add_module(trggdlUnpacker)
    trggdlsummary = b2.register_module('TRGGDLSummary')
    main.add_module(trggdlsummary)

output = b2.register_module('RootOutput')
output.param("outputFileName", options.output)
main.add_module(output, branchNames=["TRGGRLUnpackerStore"])

# Process all events
b2.process(main)
