#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>KLongGenSimNoBkg.root</output>
  <contact>tkuhr</contact>
  <cacheable/>
  <description>This steering file produces 1000 events with one KLong each
  and runs the detector simulation without mixing in background.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters
import glob

set_random_seed(12345)

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

# generate Klong events
# Particle gun
particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [130])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 1])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [30, 30])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
main.add_module(particlegun)

# detector simulation, no background files
add_simulation(main)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../KLongGenSimNoBkg.root')
main.add_module(output)

process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots('KLongGenSimNoBkg_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation job with generic KLongGenSimNoBkg events', prefix='KLongGenSimNoBkg')
event_timing_plot('../KLongGenSimNoBkg.root', 'KLongGenSimNoBkg_statistics.root',
                  contact='tkuhr',
                  jobDesc='a standard simulation job with Klong events', prefix='KLongGenSimNoBkg')
