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


input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')
printcollections = register_module('PrintCollections')

input.param('inputFileName', Belle2.FileSystem.findFile('framework/tests/root_input.root'))
# load all branches, minus PXDClusters
input.param('branchNames', [
    'EventMetaData',
    'PXDClustersToPXDDigits',
    'PXDClustersToPXDTrueHits',
    'PXDDigits',
    'PXDTrueHits',
    'PXDClusters', ])
input.param('excludeBranchNames', ['PXDClusters'])
input.param('skipNEvents', 1)
input.logging.log_level = LogLevel.WARNING

main = create_path()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module(input)
main.add_module(eventinfo)
main.add_module(printcollections)

# Process events
process(main)

# Test starting directly with a given event. There will be a fatal error if the
# event is not found in the file so let's call process() in a child process to
# not be aborted
ctx = multiprocessing.get_context("fork")
for evtNo in range(1, 6):
    main = create_path()
    main.add_module("RootInput", inputFileName=Belle2.FileSystem.findFile('framework/tests/root_input.root'),
                    branchNames=["EventMetaData"], skipTillEvent=[0, 1, evtNo], logLevel=LogLevel.WARNING)
    main.add_module("EventInfoPrinter")
    child = ctx.Process(target=process, args=(main,))
    child.start()
    child.join()
