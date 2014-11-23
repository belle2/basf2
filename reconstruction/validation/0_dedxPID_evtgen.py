#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec_dedx.root</output>
  <description>Generates dE/dx debug data (DedxTracks) for testing</description>
  <contact>christian.pulvermacher@kit.edu</contact>
</header>
"""

import os
from basf2 import *
from simulation import *
from reconstruction import *

set_random_seed(56423)

print os.getcwd()

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

pdgs = [11, 13, 211, 321, 2212]
pdgs += [-p for p in pdgs]

generator = register_module('ParticleGun')
generator.param('momentumParams', [0.05, 4.5])
generator.param('nTracks', 8.0)
generator.param('pdgCodes', pdgs)
main.add_module(generator)

#only up to CDC
components = [
    'MagneticFieldConstant4LimitedRCDC',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
]
add_simulation(main, components)

add_reconstruction(main, components)

#enable debug output for the module added by add_reconstruction()
for m in main.modules():
    if m.name() == 'DedxPID':
        m.param('enableDebugOutput', True)
        # m.param('usePXD', True)


output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenSimRec_dedx.root')
# let's keep this small
output.param('branchNames', ['DedxLikelihoods', 'DedxTracks', 'EventMetaData'])
main.add_module(output)

process(main)
print statistics
