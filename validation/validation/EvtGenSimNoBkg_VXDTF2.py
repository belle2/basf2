#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimNoBkg.root</output>
  <contact>tkuhr</contact>
  <cacheable/>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation without mixing in background.
  This file uses the new VXDTF2 (instead of VXDTF1) for data reduction in the simulation. As
  soon as VXDTF2 becomes the default, this file can be removed.
  </description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from beamparameters import add_beamparameters
import glob

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# detector simulation, no background files
add_simulation(main, use_vxdtf2=True)

# trigger simulation
add_tsim(main)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimNoBkg_VXDTF2.root')
main.add_module(output)

process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots('EvtGenSimNoBkg_statistics_VXDTF2.root', contact='tkuhr',
                 jobDesc='a standard simulation job with generic EvtGen events', prefix='EvtGenSimNoBkg')
event_timing_plot('../EvtGenSimNoBkg_VXDTF2.root', 'EvtGenSimNoBkg_statistics_VXDTF2.root',
                  contact='tkuhr',
                  jobDesc='a standard simulation job with generic EvtGen events', prefix='EvtGenSimNoBkg')
