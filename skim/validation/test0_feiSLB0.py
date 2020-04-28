#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [B0 -> D(*) l nu]cc and [B0 -> D(*) pi l nu]cc

"""
<header>
  <output>../feiSLB0.dst.root</output>
  <contact>philip.grace@adelaide.edu.au</contact>
</header>
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output

b2.set_random_seed(12345)


# create path
path = b2.Path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
path.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', b2.find_file('/skim/validation/feiSLB0.dec'))
path.add_module(evtgeninput)

# detector simulation
add_simulation(path)

# reconstruction
add_reconstruction(path)


# Finally add mdst output
output_filename = "../feiSLB0.dst.root"
add_mdst_output(path, filename=output_filename)

# process events and print call statistics
b2.process(path)
print(b2.statistics)
