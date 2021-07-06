#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
#
# This steering unpacking the CDC and CDC-TRG rawdata,
# and compare them.
#
######################################################

import basf2 as b2
from ROOT import Belle2

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)

# Set Database
b2.use_database_chain()
b2.use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))

# Input file
# Get type of input file to decide, which input module we want to use
input_files = Belle2.Environment.Instance().getInputFilesOverride()
if input_files.empty():
    print('Please specify input file(s). Example usage:',
          'basf2 perfect_match.py -i input.sroot')
    exit(1)
if input_files.front().endswith(".sroot"):
    root_input = b2.register_module('SeqRootInput')
else:
    root_input = b2.register_module('RootInput')

unpacker = b2.register_module('CDCUnpacker')
unpacker.param('enableStoreCDCRawHit', True)
# unpacker.param('enablePrintOut', True)
output = b2.register_module('RootOutput')
output.param('outputFileName', 'UnpackerOutput.root')
output.param('branchNames', ['CDCHits', 'CDCRawHits'])


class PrintTRGTime(b2.Module):
    """
    Print TRG time
    """

    def initialize(self):
        """
        set CDCHits and EventMetaData
        """
        #: EventMetaData
        self.event_info = Belle2.PyStoreObj('EventMetaData')
        #: CDCHits
        self.cdc_hit = Belle2.PyStoreArray('CDCHits')

    def event(self):
        """
        Print TRG time of an event
        """
        b2.B2INFO('Event {}:'.format(self.event_info.getEvent()))
        for hit in self.cdc_hit:
            cdc_raw_hit = hit.getRelatedTo('CDCRawHits')
            b2.B2INFO('Trigger time: {}'.format(cdc_raw_hit.getTriggerTime()))


# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(unpacker)
main.add_module(PrintTRGTime())
# main.add_module(output)

# Process all events
b2.print_path(main)
b2.process(main)

print(b2.statistics)
