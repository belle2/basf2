#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
<output>TRGValidationGen.root</output>
<contact>Yun-Tsung Lai, ytlai@post.kek.jp</contact>
<description>This steering file generates 5000 e+/e- mu+/mu- particle guns to validate the trg package.</description>
</header>
"""

import basf2 as b2
from simulation import add_simulation
import reconstruction as re
import mdst


main = b2.create_path()
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [5000], 'runList': [1]})
main.add_module(eventinfosetter)

particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [11, -11, 13, -13])
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'uniformPt')
particlegun.param('momentumParams', [0.5, 3.0])
particlegun.param('thetaParams', [35, 127])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [0, 0])
particlegun.param('yVertexParams', [0, 0])
particlegun.param('zVertexParams', [-20.0, 20.0])
main.add_module(particlegun)


# trigger simulation is included in latest basf2
add_simulation(main)


re.add_reconstruction(main)

# output
mdst.add_mdst_output(
    path=main,
    mc=True,
    filename='../TRGValidationGen.root',
    additionalBranches=[
        "TRGKLMHits",
        "TRGKLMTracks",
        "KLMTrgSummary",
        "TRGCDC2DFinderTracks",
        "TRGCDC3DFitterTracks",
        "TRGCDCNeuroTracks",
        "TRGECLClusters",
        "TRGSummary",
        "MCInitialParticles"])


main.add_module('Progress')
# main
b2.process(main)
print(b2.statistics)
# ===<END>
