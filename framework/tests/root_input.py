#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import multiprocessing

set_random_seed("something important")
# make sure FATAL messages don't have the function signature as this makes
# problems with clang printing namespaces differently
logging.set_info(LogLevel.FATAL, logging.get_info(LogLevel.ERROR))


class NoopModule(Module):
    """Doesn't do anything."""


main = create_path()
# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())
# load all branches, minus PXDClusters (relations with PXDClusters are automatically excluded)
# (yes, we also added PXDClusters in branchNames, to check if it's overridden by excludeBranchNames)
input_module = main.add_module("RootInput", inputFileName=Belle2.FileSystem.findFile('framework/tests/root_input.root'),
                               branchNames=['EventMetaData', 'PXDDigits', 'PXDTrueHits', 'PXDClusters'],
                               excludeBranchNames=['PXDClusters'], skipNEvents=1, logLevel=LogLevel.WARNING)
# print event information and datastore contents
main.add_module("EventInfoPrinter")
main.add_module("PrintCollections", printForEvent=0)

# Process events
process(main)

# Test restricting the number of events per file
# This issues a warning because a part of the events is out of range
input_module.param('entrySequences', ["0,3:10,23"])
process(main)

# Test starting directly with a given event. There will be a fatal error if the
# event is not found in the file so let's call process() in a child process to
# not be aborted
ctx = multiprocessing.get_context("fork")
for evtNo in range(1, 6):
    main = create_path()
    main.add_module("RootInput", inputFileName=Belle2.FileSystem.findFile('framework/tests/root_input.root'),
                    branchNames=["EventMetaData"], skipToEvent=[0, 1, evtNo], logLevel=LogLevel.WARNING)
    main.add_module("EventInfoPrinter")
    child = ctx.Process(target=process, args=(main,))
    child.start()
    child.join()


# Test eventSequences in detail
main = create_path()
input_module = main.add_module('RootInput', inputFileNames=[
    Belle2.FileSystem.findFile('framework/tests/chaintest_1.root'),
    Belle2.FileSystem.findFile('framework/tests/chaintest_2.root')], logLevel=LogLevel.WARNING)
# The first file contains the following event numbers (in this order)
# 2, 6, 5, 9, 10, 11, 8, 12, 0, 13, 15, 16
# We select more event than the file contains, to check if it works anyway
# The second file contains the following event numbers (in this order)
# 7, 6, 3, 8, 9, 12, 4, 11, 10, 16, 13, 17, 18, 14, 15
input_module.param('entrySequences', ['0,3:4,10:100', '1:2,4,12:13'])

expected_event_numbers = [2, 9, 10, 15, 16, 6, 3, 9, 18, 14]
processed_event_numbers = []


class TestingModule(Module):
    """
    Test module which writes out the processed event numbers
    into the global processed_event_numbers list
    """

    def event(self):
        """
        Called for each event
        """
        global processed_event_numbers
        emd = Belle2.PyStoreObj('EventMetaData')
        processed_event_numbers.append(emd.obj().getEvent())

    def terminate(self):
        """
        perform check
        """
        # We only want to do the check if we actually execute the process() but
        # since this script is called from basf2_args with --dry-run this is not
        # always the case
        assert expected_event_numbers == processed_event_numbers


main.add_module(TestingModule())
process(main)

set_random_seed("something important")
# make sure FATAL messages don't have the function signature as this makes
# problems with clang printing namespaces differently
logging.set_info(LogLevel.FATAL, logging.get_info(LogLevel.ERROR))


# Test eventSequences parameter ':' in detail
main = create_path()
input = main.add_module('RootInput', inputFileNames=[
    Belle2.FileSystem.findFile('framework/tests/chaintest_1.root'),
    Belle2.FileSystem.findFile('framework/tests/chaintest_2.root')], logLevel=LogLevel.WARNING)
# The first file contains the following event numbers (in this order)
# 2, 6, 5, 9, 10, 11, 8, 12, 0, 13, 15, 16
# We select the complete first file.
# The second file contains the following event numbers (in this order)
# 7, 6, 3, 8, 9, 12, 4, 11, 10, 16, 13, 17, 18, 14, 15
input.param('entrySequences', [':', '1:2,4,12:13'])

expected_event_numbers = [2, 6, 5, 9, 10, 11, 8, 12, 0, 13, 15, 16, 6, 3, 9, 18, 14]
processed_event_numbers = []

main.add_module(TestingModule())
process(main)
