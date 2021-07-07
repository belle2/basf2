#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>VTX_KShortGenSimNoBkg.root</output>
  <contact>Software team b2soft@mail.desy.de</contact>
  <cacheable/>
  <description>This steering file produces 1500 events with five KShort each
  and runs the detector simulation without mixing in background
  for Belle II w/ VTX.</description>
</header>
"""

from basf2 import set_random_seed, create_path, process, statistics, \
    register_module
from simulation import add_simulation
from validation import statistics_plots, event_timing_plot
import basf2 as b2

set_random_seed(1337)

# Need to use default global tag prepended with upgrade GT
from vtx import get_upgrade_globaltag
b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())

main = create_path()

main.add_module('EventInfoSetter',
                evtNumList=[1500],
                runList=[1],
                expList=[0]
                )

main.add_module('ParticleGun',
                pdgCodes=[310],
                nTracks=5,
                momentumGeneration='uniform',
                momentumParams=[0.000, 1.000],
                thetaGeneration='uniform',
                thetaParams=[17, 150],
                phiGeneration='uniform',
                phiParams=[0, 360],
                vertexGeneration='uniform',
                xVertexParams=[0.0, 0.0],
                yVertexParams=[0.0, 0.0],
                zVertexParams=[0.0, 0.0]
                )

add_simulation(main, useVTX=True)

main.add_module(register_module('Profile'))
main.add_module('RootOutput', outputFileName='../VTX_KShortGenSimNoBkg.root')

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    'VTX_KShortGenSimNoBkg_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with KShortGenSimNoBkg particleGun for Belle II w/ VTX',
    prefix='VTX_KShortGenSimNoBkg'
)
event_timing_plot(
    '../VTX_KShortGenSimNoBkg.root',
    'VTX_KShortGenSimNoBkg_statistics.root',
    contact='Software team b2soft@mail.desy.de',
    job_desc='a standard simulation job with KShort particleGun',
    prefix='VTX_KShortGenSimNoBkg'
)
