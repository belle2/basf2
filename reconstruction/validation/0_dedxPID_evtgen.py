#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec_dedx.root</output>
  <description>Generates dE/dx debug data (DedxTracks) for testing</description>
  <contact>jvbennett@cmu.edu</contact>
</header>
"""

from basf2 import *
from simulation import *
from reconstruction import *
from ROOT import Belle2

set_random_seed(56423)


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

particles = Belle2.Const.chargedStableSet
pdgs = []
for i in range(particles.size()):
    pdgs.append(particles.at(i).getPDGCode())

pdgs += [-p for p in pdgs]
print(pdgs)

generator = register_module('ParticleGun')
generator.param('momentumParams', [0.05, 4.5])
generator.param('nTracks', 8.0)
generator.param('pdgCodes', pdgs)
main.add_module(generator)

# only up to CDC
components = [
    'MagneticFieldConstant4LimitedRCDC',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
]
add_simulation(main, components)

add_reconstruction(main, components)

# enable debug output for the module added by add_reconstruction()
for m in main.modules():
    if m.name() == 'CDCDedxPID':
        m.param('enableDebugOutput', True)
    if m.name() == 'VXDDedxPID':
        m.param('enableDebugOutput', True)
        # m.param('usePXD', True)


output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimRec_dedx.root')
# let's keep this small
output.param('branchNames', ['CDCDedxLikelihoods', 'CDCDedxTracks', 'VXDDedxLikelihoods', 'VXDDedxTracks', 'EventMetaData'])
main.add_module(output)

main.add_module(register_module('ProgressBar'))

process(main)
print(statistics)
