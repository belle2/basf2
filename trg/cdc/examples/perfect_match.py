#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This steering unpacking the CDC and CDC-TRG rawdata,
# and compare them.
#
######################################################

from basf2 import *
from ROOT import Belle2

from interactive import embed
# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# Set Database
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))

# Input file
# Get type of input file to decide, which input module we want to use
input_files = Belle2.Environment.Instance().getInputFilesOverride()
if input_files.empty():
    print('Please specify input file(s). Example usage:',
          'basf2 perfect_match.py -i input.sroot')
    exit(1)
if input_files.front().endswith(".sroot"):
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')

unpacker = register_module('CDCUnpacker')
unpacker.param('enableStoreCDCRawHit', True)
# unpacker.param('enablePrintOut', True)
output = register_module('RootOutput')
output.param('outputFileName', 'UnpackerOutput.root')
output.param('branchNames', ['CDCHits', 'CDCRawHits'])


class PrintTRGTime(Module):
    def initialize(self):
        self.event_info = Belle2.PyStoreObj('EventMetaData')
        self.cdc_hit = Belle2.PyStoreArray('CDCHits')

    def event(self):
        B2INFO('Event {}:'.format(self.event_info.getEvent()))
        for hit in self.cdc_hit:
            cdc_raw_hit = hit.getRelatedTo('CDCRawHits')
            B2INFO('Trigger time: {}'.format(cdc_raw_hit.getTriggerTime()))


# Create main path
main = create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(unpacker)
main.add_module(PrintTRGTime())
# main.add_module(output)

# Process all events
print_path(main)
process(main)

print(statistics)
