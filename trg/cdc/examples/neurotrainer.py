#!/user/bin/env python

import basf2
from ROOT import Belle2
import os
import glob

"""
Example script showing how to train neural networks
to be used with the CDCTriggerNeuroModule.

This script uses realistic values for the amount of training data
and the number of runs, so it will run a long time.
"""

# ------------ #
# user options #
# ------------ #

# set random seed
basf2.set_random_seed(1)

# paths for the trained networks, the training data and the log files
mlpdir = Belle2.FileSystem.findFile('trg/cdc/data')
traindir = Belle2.FileSystem.findFile('trg/cdc/data')
logdir = Belle2.FileSystem.findFile('trg/cdc/data')
# filenames for the trained networks, the training data and the log files
mlpname = 'NeuroTrigger.root'
trainname = 'NeuroTriggerTraindata.root'
logname = 'NeuroTriggerLog'  # file extensions are appended automatically

# number of threads to be used for parallel training
nthreads = 1

# We want to train on single tracks within the acceptance of the track finder.
particlegun_params = {
    'pdgCodes': [-13, 13],              # muons
    'nTracks': 1,                       # single tracks
    'momentumGeneration': 'inversePt',  # uniform in the track curvature
    'momentumParams': [0.3, 10.],       # 0.3: minimum pt for standard track finder
    'thetaGeneration': 'uniformCos',    # uniform in solid angle
    'thetaParams': [23, 144],           # hit SL 6 from z in [-50, 50]
    'phiGeneration': 'uniform',         # uniform in solid angle
    'phiParams': [0, 360],              # full phi
    'vertexGeneration': 'uniform',      # uniform vertex distribution
    'xVertexParams': [0, 0.0],          # vertex on z-axis
    'yVertexParams': [0, 0.0],          # vertex on z-axis
    'zVertexParams': [-50.0, 50.0]}     # target range for training

background = True
bkgdir = '/sw/belle2/bkg.mixing/'


# ------------------------- #
# create path up to trigger #
# ------------------------- #

main = basf2.create_path()

# The CDCTriggerNeuroTrainer module stops the event loop when there is enough data,
# so just put a very high number of events here.
main.add_module('EventInfoSetter', evtNumList=1000000000)
main.add_module('Progress')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe', 'Cryostat',
                                        'PXD', 'SVD', 'CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
particlegun = basf2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)
main.add_module('FullSim')
if background:
    main.add_module('BeamBkgMixer',
                    backgroundFiles=glob.glob(os.path.join(bkgdir, '*usual*.root')),
                    components=['CDC'])
main.add_module('CDCDigitizer')


# -------------------------------------- #
# add trigger modules up to neurotrigger #
# -------------------------------------- #

main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"))
main.add_module('CDCTrigger2DFinder')
# For single tracks the event time estimate is not very reliable,
# so use the true event time here and hope for the best...
main.add_module('CDCTriggerETF', trueEventTime=True)


# ---------------- #
# add the training #
# ---------------- #

# To get target values for the training, we need relations between 2D track and MCParticles.
# We only want matched tracks (no clones) with many true hits.
main.add_module('CDCTriggerMCMatcher', minAxial=4, axialOnly=True,
                relateClonesAndMerged=False,
                TrgTrackCollectionName='TRGCDC2DFinderTracks')

main.add_module('CDCTriggerNeuroTrainer',
                # input and target arrays
                inputCollectionName='TRGCDC2DFinderTracks',
                targetCollectionName='MCParticles',
                trainOnRecoTracks=False,  # train with MCParticles as targets
                # output files
                filename=os.path.join(mlpdir, mlpname),
                trainFilename=os.path.join(traindir, trainname),
                logFilename=os.path.join(logdir, logname),
                # sector definition
                nMLP=5,  # total number of sectors
                # hit pattern sectorization (5 sectors for different missing stereo hits)
                SLpatternMask=[int('010101010', 2)],  # ignore axial hits in sector selection
                SLpattern=[int('111111111', 2),   # full hits
                           int('101111111', 2),   # SL 7 missing
                           int('111011111', 2),   # SL 5 missing
                           int('111110111', 2),   # SL 3 missing
                           int('111111101', 2)],  # SL 1 missing
                # phase space sectorization (trivial here, only 1 sector)
                invptRange=[[-5., 5.]],  # sectorization in charge/pt
                phiRange=[[0., 360.]],  # sectorization in phi
                thetaRange=[[0., 180.]],  # sectorization in theta (requires 3D input tracks)
                selectSectorByMC=False,  # use 2D track parameters to select sector
                invptRangeTrain=[[-5., 5.]],   # sector ranges during train may overlap
                phiRangeTrain=[[0., 360.]],    # i.e. they can be larger than the final sectors
                thetaRangeTrain=[[0., 180.]],  # (will be shrunk after training)
                # network structure
                multiplyHidden=False,  # set the number of hidden nodes directly
                nHidden=[[81]],  # 1 hidden layer with 81 nodes for all sectors
                wMax=63.,  # limit weights to [-63, 63]
                # target definition
                targetZ=True,  # output z-vertex
                targetTheta=True,  # output also polar angle
                outputScale=[[-50., 50., 0., 180.]],  # output 1 (z) scaled to [-50, 50]cm,
                                                      # output 2 (theta) scaled to [0, 180]deg
                rescaleTarget=False,  # targets outside of output range are skipped
                # relevant ID ranges: region around 2D track from which hits are taken
                # (determined from a histogram that is generated from hits related to MCParticles)
                nTrainPrepare=1000,  # number of tracks used to prepare the ID histogram
                relevantCut=0.02,  # cut on the ID histogram
                cutSum=False,  # cut directly on the ID histogram bins
                # training parameters
                multiplyNTrain=True,  # set training data relative to degrees of freedom
                nTrainMax=10.,  # training data (10x degrees of freedom)
                nTrainMin=10.,  # don't train if there is less than 10x DoF training data
                nValid=1000,  # number of validation samples (to avoid overtraining)
                nTest=5000,  # number of test samples (to select best of several runs)
                repeatTrain=10,  # train each sector 10x with different initial weights
                checkInterval=500,  # stop training if validation error does not improve for 500 epochs
                maxEpochs=10000,  # stop training after 10000 epochs
                nThreads=nthreads,  # number of parallel threads
                stopLoop=True,  # stop event loop when there is enough training data
                # log level
                logLevel=basf2.LogLevel.DEBUG,  # show some debug output
                debugLevel=50)
# show only the message of the debug output
basf2.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)


# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
