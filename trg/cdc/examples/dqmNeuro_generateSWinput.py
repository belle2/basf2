from basf2 import *
from ROOT import Belle2
import glob
from reconstruction import add_reconstruction

from simulation import add_simulation
import os

# use_central_database('data_reprocessing_prompt')
set_random_seed(1)
bkgdir = "/remote/pcbelle11/sebastian/OfficialBKG/15thCampaign/phase3/set0/"
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

main = create_path()
main.add_module('EventInfoSetter', evtNumList=5000)  # , expList=[7], runList=[3525])
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('BeamBkgMixer',
                backgroundFiles=glob.glob(os.path.join(bkgdir, '*usual*.root')),
                overallScaleFactor=1,
                components=['CDC'])
particlegun = register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)

add_simulation(main)
# main.add_module('FullSim')
main.add_module('CDCDigitizer')
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("trg/cdc/data/innerLUT_v2.2.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("trg/cdc/data/outerLUT_v2.2.coe"),
                TSHitCollectionName='CDCTriggerSegmentHits')
main.add_module('CDCTrigger2DFinder',
                minHits=4,
                minHitsShort=4,
                minPt=0.3,
                outputCollectionName='CDCTrigger2DFinderTracks')
main.add_module('CDCTriggerETF',
                hitCollectionName='CDCTriggerSegmentHits')
main.add_module('CDCTriggerNeuro',
                filename=Belle2.FileSystem.findFile("trg/cdc/data/Background2.0_20161207.root"),
                # et_option='fastestpriority',
                et_option='fastestpriority',
                hitCollectionName='CDCTriggerSegmentHits',
                outputCollectionName='CDCTriggerNeuroTracks',
                inputCollectionName='CDCTrigger2DFinderTracks',
                writeMLPinput=True)
add_reconstruction(main)


main.add_module('RootOutput', outputFileName="phase3bckg-0-reco_sim.root")
main.add_module('Progress')
process(main)
print(statistics)
