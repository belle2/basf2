#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec.root</output>
  <cacheable/>
  <contact>Software team b2soft@mail.desy.de</contact>
  <description>This steering file produces 1000 generic BBbar events with EvtGen,
  runs the detector simulation with mixed in background, and performs the standard reconstruction.</description>
</header>
"""

from basf2 import set_random_seed, create_path, process, statistics
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from svd import add_svd_create_recodigits
from validation import statistics_plots, event_timing_plot
from background import get_background_files

set_random_seed(12345)

# set one parallel process to excercise the basf2 parallel code
# set_nprocesses(1)

main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[1000], runList=[1], expList=[0])

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main, bkgfiles=get_background_files())

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

# reconstruct SVDRecoDigits not used in reconstruction
# but interesting for validation purposes
add_svd_create_recodigits(main)

# memory profile
main.add_module('Profile')

# output
main.add_module(
    'RootOutput',
    additionalBranchNames=['SpacePoints', 'SVDSpacePoints'],
    outputFileName='../EvtGenSimRec.root'
)

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    'EvtGenSimRec_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation and reconstruction job with generic EvtGen events',
    prefix='EvtGenSimRec'
)
event_timing_plot(
    '../EvtGenSimRec.root', 'EvtGenSimRec_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation and reconstruction job with generic EvtGen events',
    prefix='EvtGenSimRec'
)
