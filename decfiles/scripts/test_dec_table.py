#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Descriptor: script to test decay tables

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output
import glob
import sys
from ROOT import Belle2


event_type = str(sys.argv[1])
b2.set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
main = b2.create_path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/{0}.dec'.format(event_type)))
main.add_module(evtgeninput)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# reconstruction
add_reconstruction(main)

# Finally add mdst output
output_filename = "test.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
