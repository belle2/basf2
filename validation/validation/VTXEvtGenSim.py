#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>VTXEvtGenSim.root</output>
  <cacheable/>
  <contact>Software team b2soft@mail.desy.de</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation with mixed in background for Belle II w/ VTX.</description>
</header>
"""

from vtx import get_upgrade_globaltag, get_upgrade_background_files, get_upgrade_background_mixer_files
from basf2 import set_random_seed, create_path, process, statistics, conditions
from simulation import add_simulation
from validation import statistics_plots, event_timing_plot
from background import get_background_files

set_random_seed(12345)


# Need to use default global tag prepended with upgrade GT
conditions.disable_globaltag_replay()
conditions.prepend_globaltag(get_upgrade_globaltag())

main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[1000], runList=[1], expList=[0])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
# Overlay file location from environment variable  'BELLE2_VTX_BACKGROUND_DIR'
add_simulation(main, bkgfiles=get_upgrade_background_files(), useVTX=True)


# memory profile
main.add_module('Profile')

# output
main.add_module('RootOutput', outputFileName='../VTXEvtGenSim.root')

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    'VTXEvtGenSim_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix='VTXEvtGenSim'
)
event_timing_plot(
    '../VTXEvtGenSim.root', 'VTXEvtGenSim_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix='VTXEvtGenSim'
)
