#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>CombinedPIDPerformance.root</output>
  <description>Generates particle gun events for PID performance validation</description>
  <contact>jvbennett@cmu.edu</contact>
  <interval>release</interval>
</header>
"""

from basf2 import *
from simulation import *
from reconstruction import *
from ROOT import Belle2

set_random_seed(56424)


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
main.add_module(eventinfosetter)

particles = Belle2.Const.chargedStableSet
pdgs = []
for i in range(particles.size()):
    pdgs.append(particles.at(i).getPDGCode())

pdgs += [-p for p in pdgs]
print(pdgs)

generator = register_module('ParticleGun')
generator.param('momentumParams', [0.05, 5.0])
generator.param('nTracks', 8.0)
generator.param('pdgCodes', pdgs)
main.add_module(generator)

add_simulation(main)
add_reconstruction(main)


# run a module to generate histograms to test pid performance
pidperformance = register_module('CombinedPIDPerformance')
pidperformance.param('outputFileName', 'CombinedPIDPerformance.root')
pidperformance.param('numberOfBins', 50)
main.add_module(pidperformance)


main.add_module(register_module('ProgressBar'))

process(main)
print(statistics)
