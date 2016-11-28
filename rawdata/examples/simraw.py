#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from rawdata import add_packers, add_raw_output, add_raw_seqoutput
from glob import glob
import ROOT

# create path
main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main)

# pack raw data
add_packers(main)

# raw data output
add_raw_output(main, 'raw.root')

# sroot raw data output
add_raw_seqoutput(main, 'raw.sroot')

# process events and print call statistics
process(main)
print(statistics)
