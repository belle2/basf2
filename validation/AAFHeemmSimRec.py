#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>AAFHeemmSimRec.root</output>
  <contact>ferber</contact>
  <description>This steering file produces 100 non radiative ee->eemm events with AAFH, runs the detector simulation with mixed in background, and performs the standard reconstruction.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
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

# generate ee->eemm events
aafh = register_module('AafhInput')
aafh.param({
    'mode': 3,
    'minMass': 0.50,
    'rejection': 2,
    'maxSubgeneratorWeight': 1.000e+00,
    'maxFinalWeight': 3.0,
    'subgeneratorWeights': [
        1.000e+00,
        1.540e+01,
        5.205e+03,
        4.088e+04,
        1.000e+00,
        1.216e+00,
        5.583e+00,
        6.155e+00,
        ],
    'suppressionLimits': [1e100] * 4,
    })
main.add_module(aafh)

## register the preselection module
generatorpreselection = register_module('GeneratorPreselection')
generatorpreselection.param({
    'nChargedMin': 1,
    'MinChargedP': 0.50,
    'MinChargedPt': 0.1,
    'MinChargedTheta': 16.5,
    'MaxChargedTheta': 150.5,
    'nPhotonMin': 1,
    'MinPhotonEnergy': 0.50,
    'MinPhotonTheta': 12.,
    'MaxPhotonTheta': 156.,
    })

main.add_module(generatorpreselection)
generatorpreselection.if_value('<1', emptypath)

# detector simulation
bg = None
if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../AAFHeemmSimRec.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics

from validation import *
statistics_plots('AAFHeemmSimRec_statistics.root', contact='ferber',
                 jobDesc='a standard simulation and reconstruction job with non radiative ee->eemm events'
                 )
event_timing_plot('../AAFHeemmSimRec.root', 'AAFHeemmSimRec_statistics.root',
                  contact='ferber',
                  jobDesc='a standard simulation and reconstruction job with non radiative ee->eemm events'
                  )
