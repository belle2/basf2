#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>BabagagaNLOBhabhaSimRec.root</output>
  <cacheable/>
  <contact>ferber</contact>
  <description>This steering file produces 100 radiative Bhabha events with Babayaga.NLO,
  runs the detector simulation with mixed in background, and performs the standard reconstruction.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
import glob

set_random_seed(12345)

main = create_path()
emptypath = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# geometry parameter database
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")

# generate Bhabha events
babayaganlo = register_module('BabayagaNLOInput')
main.add_module(babayaganlo)

# register the preselection module
generatorpreselection = register_module('GeneratorPreselection')
generatorpreselection.param({
    'nChargedMin': 1,
    'MinChargedP': 0.5,
    'MinChargedPt': 0.1,
    'MinChargedTheta': 16.5,
    'MaxChargedTheta': 150.5,
    'nPhotonMin': 1,
    'MinPhotonEnergy': 0.5,
    'MinPhotonTheta': 12.,
    'MaxPhotonTheta': 156.,
})

main.add_module(generatorpreselection)
generatorpreselection.if_value('<1', emptypath)

# detector simulation
bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../BabayagaNLOBhabhaSimRec.root')
main.add_module(output)

process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots(
    'BabayagaNLOBhabhaSimRec_statistics.root',
    contact='ferber',
    jobDesc='a standard simulation and reconstruction job with radiative Bhabha events using Babayaga.NLO',
    prefix='BabayagaNLOBhabhaSimRec')
event_timing_plot(
    '../BabayagaNLOBhabhaSimRec.root',
    'BabayagaNLOBhabhaSimRec_statistics.root',
    contact='ferber',
    jobDesc='a standard simulation and reconstruction job with radiative Bhabha events using Babayaga.NLO',
    prefix='BabayagaNLOBhabhaSimRec')
