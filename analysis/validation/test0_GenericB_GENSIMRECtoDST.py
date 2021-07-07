#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>../GenericB_GENSIMRECtoDST.dst.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters

basf2.set_random_seed(12345)

main = basf2.create_path()

# specify number of events to be generated
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# set the BeamParameters for running at Y(4S)
beamparameters = add_beamparameters(main, "Y4S")
basf2.print_params(beamparameters)

# generate BBbar events
evtgeninput = basf2.register_module('EvtGenInput')
main.add_module(evtgeninput)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction
add_reconstruction(main)

# dst output
output = basf2.register_module('RootOutput')
output.param('outputFileName', '../GenericB_GENSIMRECtoDST.dst.root')
main.add_module(output)

# Go!
basf2.process(main)

# Print call statistics
print(basf2.statistics)
