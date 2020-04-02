#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSim.root</output>
  <cacheable/>
  <contact>Software team b2soft@mail.desy.de</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen
  and runs the detector simulation with mixed in background.</description>
</header>
"""

from basf2 import set_random_seed, create_path, process, statistics
from simulation import add_simulation
from L1trigger import add_tsim
from validation import statistics_plots, event_timing_plot
from background import get_background_files
import glob as glob

set_random_seed(12345)

exp = '8'
run = '*'
bgfile = '/group/belle2/users/jbennett/BGOverlayFromData/e000' + exp + \
    '/release-04-01-00/data_reprocessing_proc10/BGOverlay.physics.e000' + exp + '.r0' + run + '.root'
bg = glob.glob(bgfile)


main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[5000], runList=[1], expList=[1003])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
# add_simulation(main, bkgfiles=get_background_files())
add_simulation(main, bkgfiles=bg)

# trigger simulation
# add_tsim(main)

# memory profile
main.add_module('Profile')

# output
main.add_module('RootOutput', outputFileName='../EvtGenSim_5k_exp1003_run1.root')

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    'EvtGenSim_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix='EvtGenSim'
)
event_timing_plot(
    '../EvtGenSim.root', 'EvtGenSim_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix='EvtGenSim'
)
