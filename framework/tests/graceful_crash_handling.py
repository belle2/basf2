#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import os
from signal import *
import tempfile
import shutil
from basf2 import *
from ROOT import Belle2

# we test for stray resources later, so let's clean up first
os.system('clear_basf2_ipc')

set_random_seed("something important")


def run_test():
    class CrashyModule(Module):
        """test"""

        def __init__(self):
            """init."""
            super().__init__()

        def initialize(self):
            displaydata = Belle2.PyStoreArray(Belle2.DisplayData.Class())
            displaydata.registerInDataStore()

        def event(self):
            """reimplementation of Module::event()."""
            eventmetadata = Belle2.PyStoreObj('EventMetaData')
            nevent = eventmetadata.obj().getEvent()

            # add some elements to test
            displaydata = Belle2.PyStoreArray(Belle2.DisplayData.Class())
            displaydata.appendNew()
            displaydata.appendNew()

            B2INFO("event({})".format(nevent))
            if nevent % 2 == 1:
                pid = os.getpid()
                B2INFO("Killing %s" % (pid))
                os.kill(pid, 11)
                # B2FATAL("diediedie")  # faster

    # Create paths
    main = create_path()
    main.add_module('EventInfoSetter', evtNumList=[5])

    wrappedpath = create_path()
    testmod = wrappedpath.add_module(CrashyModule())
    testmod.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    wrappedpath.add_module('EventInfoPrinter').set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    crashhandler = main.add_module('CrashHandler', path=wrappedpath)

    printcollections = main.add_module('PrintCollections')

    process(main)
    print(statistics)

    assert statistics.get(testmod).calls(statistics.EVENT) == 2
    assert statistics.get(crashhandler).calls(statistics.EVENT) == 5
    assert statistics.get(printcollections).calls(statistics.EVENT) == 5


for nproc in [0, 3]:
    B2WARNING(80*'=')
    B2WARNING("Starting test with {} processes...".format(nproc))
    set_nprocesses(nproc)
    run_test()

ret = os.system('clear_basf2_ipc')
if ret != 0:
    raise RuntimeError("Some IPC structures were not cleaned up")

B2INFO("All tests ok.")
