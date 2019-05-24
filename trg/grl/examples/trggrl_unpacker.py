#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        CDCT3D trigger Unpacker module
#
#    usage : %> basf2 TrgCdcT3dUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
from basf2 import *
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

set_log_level(LogLevel.ERROR)

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileName', options.filename)

main.add_module(input)


unpacker = register_module('TRGGRLUnpacker')
main.add_module(unpacker)


if int(options.tracking):
    cdcunpacker = register_module('CDCUnpacker')
    cdcunpacker.param('xmlMapFileName', "cdc/data/ch_map.dat")
    cdcunpacker.param('enablePrintOut', False)
    main.add_module(cdcunpacker)
    add_cosmics_reconstruction(main, 'CDC', False)


if int(options.gdl):
    trggdlUnpacker = register_module("TRGGDLUnpacker")
    main.add_module(trggdlUnpacker)
    trggdlsummary = register_module('TRGGDLSummary')
    main.add_module(trggdlsummary)

output = register_module('RootOutput')
output.param("outputFileName", options.output)
main.add_module(output, branchNames=["GRLevt"])

# Process all events
process(main)
