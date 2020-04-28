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
import os

set_random_seed(12345)

exp = '8'
run = '*'
bgfile = '/group/belle2/dataprod/BGOverlay/e000' + exp + \
    '/release-04-01-04/data_reprocessing_proc10/BGOverlay.physics.e000' + exp + '.r0' + run + '.root'
bg = glob.glob(bgfile)

# define the settings for the generator (?!)
usedExp = 1003  # 1003 is the incomplete detector due to some missing parts in the PXD. 0 would be the complete detector
runNo = 1
events = 200
postfix = '_local'
OUTPUT_FILE_NAME = 'EvtGenSim_' + str(int(round(events / 1000, 0))) + 'k_exp' + \
                                      str(usedExp) + '_run' + str(runNo) + postfix + '.root'


main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[events], runList=[runNo], expList=[usedExp])

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
main.add_module('RootOutput', outputFileName=OUTPUT_FILE_NAME)

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    os.path.splitext(OUTPUT_FILE_NAME)[0] + '_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix=os.path.splitext(OUTPUT_FILE_NAME)[0]
)
event_timing_plot(
    OUTPUT_FILE_NAME, os.path.splitext(OUTPUT_FILE_NAME)[0] + '_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with generic EvtGen events',
    prefix=os.path.splitext(OUTPUT_FILE_NAME)[0]
)
