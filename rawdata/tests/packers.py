#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import multiprocessing as mp
import sys
from ROOT import Belle2
import rawdata as raw
import simulation as sim
import reconstruction as rec
import basf2 as b2


# Deactivate test for the moment
sys.exit(0)

path_to_output = 'rawdata/tests/digits.root'
b2.set_random_seed("L1V0RN0")
b2.set_log_level(b2.LogLevel.WARNING)
# Disable tag replay, we want to test current packers
# independent of when the digits were created
b2.conditions.disable_globaltag_replay()


def create_digits():
    """Create the file 'digits.root' needed for testing the packers if it does not exist"""
    child_path = b2.create_path()
    child_path.add_module('EventInfoSetter',
                          evtNumList=[10])
    child_path.add_module('EvtGenInput')
    sim.add_simulation(path=child_path)
    child_path.add_module(
        'RootOutput',
        outputFileName='${BELLE2_LOCAL_DIR}/' +
        path_to_output,
        branchNames=list(rec.DIGITS_OBJECTS))
    child_path.add_module('Progress')
    b2.process(child_path)
    print(b2.statistics)


if Belle2.FileSystem.findFile(path_to_output, True) == '':
    # Execute create_digits() in a child process to avoid side effects
    child = mp.Process(target=create_digits)
    child.start()
    # Wait for simulation to finish
    child.join()
    # And exit if it had an error
    if child.exitcode != 0:
        sys.exit(child.exitcode)

# Here starts the main path
main_path = b2.create_path()
main_path.add_module('RootInput',
                     inputFileNames=Belle2.FileSystem.findFile(path_to_output))
raw.add_packers(path=main_path)
main_path.add_module('Progress')
b2.process(main_path)
print(b2.statistics)
