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

import basf2
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
# exp = '12'
# run = '5736'
# bgfile = '/group/belle2/dataprod/BGOverlay/e' + exp.zfill(4) + \
#    '/release-04-02-02/online/BGOverlay.physics.e' + exp.zfill(4) + '.r0' + run + '.root'
bgfile = '/group/belle2/dataprod/BGOverlay/e' + exp.zfill(4) + \
    '/release-04-01-04/data_reprocessing_proc10/BGOverlay.physics.e' + exp.zfill(4) + '.r0' + run + '.root'
bg = glob.glob(bgfile)

# define the settings for the generator (?!)
usedExp = 1003  # 1003 is the incomplete detector due to some missing parts in the PXD. 0 would be the complete detector
runNo = 1
events = 1000
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

# basf2.conditions.prepend_globaltag("online_proc10")
# basf2.conditions.prepend_globaltag("data_reprocessing_proc10")
# basf2.conditions.prepend_globaltag("mc_production_MC13b")
# basf2.conditions.prepend_globaltag("release-04-02-00_rev2")
# basf2.conditions.prepend_globaltag("online_proc11")
# basf2.conditions.prepend_globaltag("data_reprocessing_proc11")
# basf2.conditions.prepend_globaltag("mc_production_MC13b_proc11")
# basf2.conditions.prepend_globaltag("klm_alignment_testing")
# basf2.conditions.prepend_globaltag("mc_production_MC13a_rev1")

# basf2.conditions.prepend_globaltag("online")
# basf2.conditions.prepend_globaltag("Reco_master_patch_rel5")
# basf2.conditions.prepend_globaltag("online")
# basf2.conditions.prepend_globaltag("data_reprocessing_prompt")
# basf2.conditions.prepend_globaltag("bgoverlay_production_rel5patch")
# basf2.conditions.prepend_globaltag("prerel5_rundep_mc_test")


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
