#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>VTXEvtGenSimNoBkg.root</output>
  <contact>Software team b2soft@mail.desy.de</contact>
  <cacheable/>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation without mixing in background for Belle II w/ VTX.</description>
</header>
"""

from validation import *
from vtx import get_upgrade_globaltag
from basf2 import (
    set_random_seed,
    create_path,
    process,
    statistics,
    register_module,
    conditions
)
from simulation import add_simulation
from beamparameters import add_beamparameters
from validation import statistics_plots, event_timing_plot

set_random_seed(12345)

# Need to use default global tag prepended with upgrade GT
conditions.disable_globaltag_replay()
conditions.prepend_globaltag(get_upgrade_globaltag())

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("smearVertex", False)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# detector simulation, no background files
add_simulation(main, useVTX=True)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../VTXEvtGenSimNoBkg.root')
main.add_module(output)

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    'VTXEvtGenSimNoBkg_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix='VTXEvtGenSimNoBkg'
)
event_timing_plot(
    '../VTXEvtGenSimNoBkg.root', 'VTXEvtGenSimNoBkg_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix='VTXEvtGenSimNoBkg'
)