#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>TRGValidationGen.root</output>
<contact>Yun-Tsung Lai, ytlai@post.kek.jp</contact>
<description>This steering file generates 1000 e+/e- particle guns to validate the trg package.</description>
</header>
"""

import os
from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from modularAnalysis import *

import glob

main = create_path()
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [11, -11])
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'uniformPt')
particlegun.param('momentumParams', [0.2, 5.0])
particlegun.param('thetaParams', [35, 127])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [0, 0])
particlegun.param('yVertexParams', [0, 0])
particlegun.param('zVertexParams', [-20.0, 20.0])
main.add_module(particlegun)

add_simulation(main)

# add trigger
add_tsim(main, component=["CDC", "ECL", "KLM", "GRL", "GDL"])

# output
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', "../TRGValidationGen.root")
main.add_module(
    rootoutput,
    branchNames=[
        "TRGCDC2DFinderTracks",
        "TRGCDC3DFitterTracks",
        "TRGCDCNeuroTracks",
        "TRGECLClusters",
        "MCParticles"])

# main
process(main)
print(statistics)
# ===<END>
