##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2
import glob
from reconstruction import add_reconstruction

from simulation import add_simulation
import os

b2.set_random_seed(1)
bkgdir = "/remote/neurobelle/data/bckg/OfficialBKG/15thCampaign/phase3/set0/"
thrange = [10, 170]
particlegun_params = {
        'pdgCodes': [-13, 13],              # muons
        'nTracks': 1,                       # single tracks
        'momentumGeneration': 'inversePt',  # uniform in the track curvature
        'momentumParams': [0.3, 10.],       # 0.3: minimum pt for standard track finder
        'thetaGeneration': 'uniformCos',    # uniform in solid angle
        'thetaParams': thrange,           # hit SL 6 from z in [-50, 50]
        'phiGeneration': 'uniform',         # uniform in solid angle
        'phiParams': [0, 360],              # full phi
        'vertexGeneration': 'uniform',      # uniform vertex distribution
        'xVertexParams': [0, 0.0],          # vertex on z-axis
        'yVertexParams': [0, 0.0],          # vertex on z-axis
        'zVertexParams': [0.0, 0.0]}     # target range for training

main = b2.create_path()
main.add_module('EventInfoSetter', evtNumList=5000)  # , expList=[7], runList=[3525])
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('BeamBkgMixer',
                backgroundFiles=glob.glob(os.path.join(bkgdir, '*usual*.root')),
                overallScaleFactor=1,
                components=['CDC'])
particlegun = b2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)

add_simulation(main)
# main.add_module('FullSim')
main.add_module('CDCDigitizer')
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("trg/cdc/data/innerLUT_v2.2.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("trg/cdc/data/outerLUT_v2.2.coe"),
                TSHitCollectionName='CDCTriggerNNInputSegmentHits')
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("trg/cdc/data/innerLUT_v2.2.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("trg/cdc/data/outerLUT_v2.2.coe"),
                TSHitCollectionName='CDCTriggerSegmentHits')
main.add_module('CDCTrigger2DFinder',
                minHits=4,
                minHitsShort=4,
                minPt=0.3,
                hitCollectionName='CDCTriggerSegmentHits',
                outputCollectionName='CDCTrigger2DFinderTracks')
main.add_module('CDCTrigger2DFinder',
                minHits=4,
                minHitsShort=4,
                minPt=0.3,
                hitCollectionName='CDCTriggerNNInputSegmentHits',
                outputCollectionName='CDCTriggerNNInput2DFinderTracks')
main.add_module('CDCTriggerETF',
                hitCollectionName='CDCTriggerNNInputSegmentHits')
main.add_module('CDCTriggerNeuro',
                hitCollectionName='CDCTriggerNNInputSegmentHits',
                outputCollectionName='CDCTriggerNeuroTracks',
                inputCollectionName='CDCTriggerNNInput2DFinderTracks',
                fixedPoint=True,
                writeMLPinput=True)
add_reconstruction(main)


main.add_module('RootOutput', outputFileName="phase3bckg-0-reco_sim.root")
main.add_module('Progress')
b2.process(main)
print(b2.statistics)
