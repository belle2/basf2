#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond

import basf2
from ROOT import Belle2


main = basf2.Path()
indep = basf2.Path()

# input (if you're doing signal embedding, this should be the ROE from data)
input1 = basf2.register_module('RootInput')
input1.param('inputFileName', '/nfs/dust/belle2/user/glazov/test_embedding/jpsiExp12_skim.root')
main.add_module(input1).set_name("--input_main--")

# and the other input (if you're doing signal embedding, this should be the signal from MC)
input2 = basf2.register_module('RootInput')
input2.param('inputFileName', '/nfs/dust/belle2/user/glazov/test_embedding/mcsig_skim_166.root')
input2.param('isSecondaryInput', True)  # <- set flag for secondary input module
indep.add_module(input2).set_name("input_indep")

# convert the EventExtraInfo to a framework kind of object (temporary fix?)
conversion1 = basf2.register_module('EventExtraInfoConverter')
main.add_module(conversion1).set_name("conversion_main")
conversion2 = basf2.register_module('EventExtraInfoConverter')
indep.add_module(conversion2).set_name("conversion_indep")

# merge it!
# Use merge_back_event=['ALL'] to merge everything
# or specify explicitly merge_back_event=['EventMetaData', 'ECLClusters', 'Tracks', 'TracksToECLClusters']
# NOTE: StoreArrays have to be merged before their Relations (and EventMetaData is strictly required!)
main.add_independent_merge_path(
    indep,
    consistency_check="charge",  # <- skip event if charge of ROE/sig (or ROE/ROE) does not match
    # [None or empty string if you don't want to do this]
    merge_back_event=['ALL'],  # <- list of content to be merged
    event_mixing=False,  # <- signal embedding or event mixing
    merge_same_file=False  # <- for event mixing, you can mix a file with itself
)

main.add_module('FixMergedObjects')

# output
output = basf2.register_module('RootOutput')
output.param('outputFileName', '/nfs/dust/belle2/user/kurzsimo/testSample/merged_10evts.root')
main.add_module(output)

# progress
main.add_module('Progress')

basf2.print_path(main)
basf2.process(main)

print(basf2.statistics)

# @endcond
