#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from simulation import add_simulation
from rawdata import add_packers, add_raw_output, add_raw_seqoutput

# create path
main = b2.create_path()

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

# sroot raw data output with computing group preferred naming
add_raw_seqoutput(main, "raw.f%05d.sroot", fileNameIsPattern=True)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
