#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>EvtGenSimRec_pid.root</output>
  <description>Generates files with information for PID testing</description>
  <contact>jvbennett@cmu.edu</contact>
</header>
"""

from basf2 import *
from simulation import *
from reconstruction import *
from beamparameters import add_beamparameters
from ROOT import Belle2

set_random_seed(56423)


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

add_beamparameters(main, "Y4S")

evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', 'ccbar.dec')
evtgeninput.param('ParentParticle', 'vpho')
main.add_module(evtgeninput)

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
for m in main.modules():
    if m.name() == 'VXDDedxPID':
        m.param('enableDebugOutput', True)
        # m.param('usePXD', True)

output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenSimRec_pid.root')
output.param('branchNames', ['CDCDedxLikelihoods', 'VXDDedxLikelihoods', 'Tracks', 'TrackFitResults',
                             'MCParticles', 'CDCDedxTracks', 'VXDDedxTracks', 'PIDLikelihoods', 'EventMetaData'])
main.add_module(output)

main.add_module(register_module('ProgressBar'))

process(main)
# print statistics
