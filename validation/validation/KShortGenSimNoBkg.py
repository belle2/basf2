#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>KShortGenSimNoBkg.root</output>
  <contact>tkuhr</contact>
  <cacheable/>
  <description>This steering file produces 500 events with five KLong each
  and runs the detector simulation without mixing in background.</description>
</header>
"""

from basf2 import *
from simulation import add_simulation

set_random_seed(1337)

main = create_path()

main.add_module('EventInfoSetter',
                evtNumList=[500],
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

add_simulation(main)

main.add_module(register_module('Profile'))
main.add_module('RootOutput', outputFileName='../KShortGenSimNoBkg.root')

process(main)

# Print call statistics
print(statistics)

from validation import *
statistics_plots('KShortGenSimNoBkg_statistics.root', contact='tkuhr',
                 jobDesc='a standard simulation job with KShortGenSimNoBkg particleGun', prefix='KShortGenSimNoBkg')
event_timing_plot('../KShortGenSimNoBkg.root', 'KShortGenSimNoBkg_statistics.root',
                  contact='tkuhr',
                  jobDesc='a standard simulation job with Klong particleGun', prefix='KShortGenSimNoBkg')
