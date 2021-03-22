#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec_B2Kpi.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Generates signal MC of B0 -> K-pi+, 200 events</description>
</header>
"""

import basf2 as b2
import os
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2
import glob

b2.set_random_seed(123451)

main = b2.create_path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

# generate events (B0 -> K+pi- + cc, other B0 generic)
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('top/validation/B2Kpi.dec'))
main.add_module(evtgeninput)

# detector simulation
bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# output (to save space only branches needed in 1_makeNtuple.py)
output = b2.register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimRec_B2Kpi.root')
output.param('branchNames', ['MCParticles', 'Tracks', 'TrackFitResults',
                             'TOPLikelihoods', 'TOPBarHits', 'ExtHits', 'TOPRecBunch'])
main.add_module(output)

b2.process(main)

# Print call statistics
print(b2.statistics)
