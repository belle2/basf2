#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import multiprocessing
from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
from rawdata import add_packers, add_raw_output


def create_digits():
    """Create digits.root needed for testing the packers if it does not exist"""
    # Digits file does not exist, create it on the fly in a child process
    os.chdir(os.path.expandvars("${BELLE2_LOCAL_DIR}/rawdata/tests"))
    sim = create_path()
    sim.add_module("EventInfoSetter", evtNumList=[10])
    sim.add_module("EvtGenInput")
    add_simulation(sim)
    sim.add_module("RootOutput", outputFileName="digits.root", branchNames=[
        "PXDDigits", "SVDShaperDigits", "CDCDigits", "TOPRawDigits", "ARICHDigits",
        "BKLMDigits", "EKLMDigits", "ECLDigits", "ECLDsp",
    ])
    process(sim)


set_random_seed("something important")
set_log_level(LogLevel.WARNING)

if Belle2.FileSystem.findFile("rawdata/tests/digits.root", False) == "":
    # execute the create_digits in a child process to avoid side effects
    child = multiprocessing.Process(target=create_digits)
    child.start()
    # wait for simulation to finish
    child.join()
    # and exit if it had an error
    if child.exitcode != 0:
        sys.exit(child.exitcode)

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileNames', Belle2.FileSystem.findFile('rawdata/tests/digits.root'))
main.add_module(input)

# gearbox
main.add_module('Gearbox')

# geometry, needed by arich and top
main.add_module('Geometry', components=['ARICH', 'TOP', 'SVD'])

# conversion from digits to raw data
add_packers(main)

# output
# add_raw_output(main)

# process events
process(main)

# Print call statistics
print(statistics)
