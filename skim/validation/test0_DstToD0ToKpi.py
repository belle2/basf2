#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [D*+ -> [D0 -> K- pi+] pi+ ]

"""
<header>
  <output>./WG6_DstToD0pi_D0ToKpi.dst.root</output>
  <contact>lilongke@ihep.ac.cn</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
import glob

set_random_seed(12345)

# background (collision) files
# bg = glob.glob('./BG/[A-Z]*.root')

# create path
myMain = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
myMain.add_module(eventinfosetter)

# Generator
# to run the framework the used modules need to be registered
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'vpho')
evtgeninput.param('userDECFile', './WG6_DstToD0pi_D0ToKpi.dec')
evtgeninput.param('InclusiveParticle', 'D*+')
evtgeninput.param('InclusiveType', 2)
myMain.add_module(evtgeninput)

# detector simulation
# add_simulation(myMain, bkgfiles=bg)
add_simulation(myMain)

# reconstruction
add_reconstruction(myMain)


# Finally add mdst output
output_filename = "./WG6_DstToD0pi_D0ToKpi.dst.root"
add_mdst_output(myMain, filename=output_filename)

# process events and print call statistics
process(myMain)
print(statistics)
