#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from ROOT import Belle2
from b2test_utils import safe_process, clean_working_directory

basf2.conditions.disable_globaltag_replay()
basf2.set_random_seed("something important")
# make sure FATAL messages don't have the function signature as this makes
# problems with clang printing namespaces differently
basf2.logging.set_info(basf2.LogLevel.FATAL, basf2.logging.get_info(basf2.LogLevel.ERROR))
basf2.logging.enable_summary(False)


class NoopModule(basf2.Module):
    """Doesn't do anything."""


with clean_working_directory():
    for filename in ["root_input.root", "chaintest_empty.root", "chaintest_1.root", "chaintest_2.root"]:
        os.symlink(basf2.find_file(f'framework/tests/{filename}'), filename)

    main = basf2.Path()
    # not used for anything, just checking wether the master module
    # can be found if it's not the first module in the path.
    main.add_module(NoopModule())
    # load all branches, minus PXDClusters (relations with PXDClusters are automatically excluded)
    # (yes, we also added PXDClusters in branchNames, to check if it's overridden by excludeBranchNames)
    input_module = main.add_module("RootInput", inputFileName="root_input.root",
                                   branchNames=['EventMetaData', 'PXDDigits', 'PXDTrueHits', 'PXDClusters'],
                                   excludeBranchNames=['PXDClusters'], skipNEvents=1, logLevel=basf2.LogLevel.WARNING)
    # print event information and datastore contents
    main.add_module("EventInfoPrinter")
    main.add_module("PrintCollections", printForEvent=0)

    # Process events
    assert safe_process(main) == 0

    # Test restricting the number of events per file
    # This issues a warning because a part of the events is out of range
    input_module.param('entrySequences', ["0,3:10,23"])
    assert safe_process(main) == 0

    # Test starting directly with a given event. There will be a fatal error if the
    # event is not found in the file so let's call process() in a child process to
    # not be aborted
    for evtNo in range(1, 6):
        main = basf2.Path()
        main.add_module("RootInput", inputFileName='root_input.root',
                        branchNames=["EventMetaData"], skipToEvent=[0, 1, evtNo], logLevel=basf2.LogLevel.WARNING)
        main.add_module("EventInfoPrinter")
        safe_process(main)

    # Test eventSequences in detail
    main = basf2.Path()
    input_module = main.add_module('RootInput', inputFileNames=['chaintest_1.root', 'chaintest_2.root'],
                                   logLevel=basf2.LogLevel.WARNING)
    # The first file contains the following event numbers (in this order)
    # 2, 6, 5, 9, 10, 11, 8, 12, 0, 13, 15, 16
    # We select more event than the file contains, to check if it works anyway
    # The second file contains the following event numbers (in this order)
    # 7, 6, 3, 8, 9, 12, 4, 11, 10, 16, 13, 17, 18, 14, 15
    input_module.param('entrySequences', ['0,3:4,10:100', '1:2,4,12:13'])

    class TestingModule(basf2.Module):
        """
        Test module which writes out the processed event numbers
        into the global processed_event_numbers list
        """

        def __init__(self, expected):
            """
            Initialize this nice class with the list of expected event
            numbers to see in the correct order
            """
            super().__init__()
            #: event metadata
            self.emd = Belle2.PyStoreObj('EventMetaData')
            #: list of expected event numbers
            self._expected_event_numbers = expected
            #: list of seen event numbers
            self._processed_event_numbers = []

        def event(self):
            """
            Called for each event
            """
            self._processed_event_numbers.append(self.emd.getEvent())

        def terminate(self):
            """
            perform check
            """
            # We only want to do the check if we actually execute the process() but
            # since this script is called from basf2_args with --dry-run this is not
            # always the case
            assert self._expected_event_numbers == self._processed_event_numbers

    main.add_module(TestingModule([2, 9, 10, 15, 16, 6, 3, 9, 18, 14]))
    assert safe_process(main) == 0

    basf2.set_random_seed("something important")
    # make sure FATAL messages don't have the function signature as this makes
    # problems with clang printing namespaces differently
    basf2.logging.set_info(basf2.LogLevel.FATAL, basf2.logging.get_info(basf2.LogLevel.ERROR))

    # Test eventSequences parameter ':' in detail
    main = basf2.Path()
    input_module = main.add_module('RootInput', inputFileNames=['chaintest_1.root', 'chaintest_2.root'],
                                   logLevel=basf2.LogLevel.WARNING)
    # The first file contains the following event numbers (in this order)
    # 2, 6, 5, 9, 10, 11, 8, 12, 0, 13, 15, 16
    # We select the complete first file.
    # The second file contains the following event numbers (in this order)
    # 7, 6, 3, 8, 9, 12, 4, 11, 10, 16, 13, 17, 18, 14, 15
    input_module.param('entrySequences', [':', '1:2,4,12:13'])

    expected_event_numbers = [2, 6, 5, 9, 10, 11, 8, 12, 0, 13, 15, 16, 6, 3, 9, 18, 14]
    main.add_module(TestingModule(expected_event_numbers))

    assert safe_process(main) == 0

    full_list = ["chaintest_empty.root", "chaintest_1.root", "chaintest_2.root", "chaintest_1.root"]
    for i in range(len(full_list)):
        main = basf2.Path()
        main.add_module('RootInput', inputFileNames=full_list[:i + 1])
        main.add_module("EventInfoPrinter")
        safe_process(main)

    class BrokenEventsModule(basf2.Module):
        """
        Small module to mark all events except for the ones in the given list as
        discarded by the HLT
        """

        def __init__(self, accepted):
            """
            Initialize this nice class with the list/set of accepted event
            numbers to see. Order not important. All other events will be flagged
            as discarded.
            """
            super().__init__()
            #: event metadata
            self.emd = Belle2.PyStoreObj('EventMetaData')
            #: list/set of accepted events
            self._accepted = accepted

        def event(self):
            """Set error flag if required"""
            if self.emd.getEvent() not in self._accepted:
                error = Belle2.EventMetaData.c_HLTDiscard if self.emd.getEvent() % 2 == 0 else \
                    Belle2.EventMetaData.c_HLTCrash
                self.emd.addErrorFlag(error)

    main = basf2.Path()
    main.add_module("EventInfoSetter", evtNumList=10)
    main.add_module(BrokenEventsModule({3, 5, 6, 8}))
    main.add_module("RootOutput", outputFileName="brokenevents.root")
    assert safe_process(main) == 0

    main = basf2.Path()
    main.add_module("RootInput", inputFileName="brokenevents.root")
    main.add_module(TestingModule([3, 5, 6, 8]))
    assert safe_process(main) == 0
