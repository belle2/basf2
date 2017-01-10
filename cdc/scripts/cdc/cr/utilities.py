from basf2 import *
from ROOT import Belle2
from tracking import add_cdc_cr_track_finding


# Propagation velocity of the light in the scinti.
lightPropSpeed = 12.9925

# Run range.
run_range = {'201607': [787, 833],
             '201608a': [885, 896],
             '201608b': [917, 924],
             '201609': [966, 973]
             }
# Size of trigger counter.
triggerSize = {'201607': [20.0, 6.0, 10.0],
               '201608a': [100.0, 8.0, 10.0],
               '201608b': [100.0, 8.0, 10.0],
               '201609': [100.0, 8.0, 10.0]
               }
# Center position of trigger counter.
triggerPosition = {'201607': [0.3744, 0.0, -1.284],
                   '201608a': [-1.87, -1.25, 18.7],
                   '201608b': [-1.87, -1.25, 11.0],
                   '201609': [0, 0, 11.0]}

# Normal direction of the trigger plane.
triggerPlaneDirection = {'201607': [1, -1, 0],
                         '201608a': [0, 1, 0],
                         '201608b': [0, 1, 0],
                         '201609': [0, 1, 0]
                         }

# PMT position.
pmtPosition = {'201607': [0, 0, 0],
               '201608a': [-1.87, 0, -25.0],
               '201608b': [-1.87, 0, -42.0],
               '201609': [0, 0, -42.0]
               }

# Global phi rotation.
globalPhiRotation = {'201607': 1.875,
                     '201608a': 1.875,
                     '201608b': 1.875,
                     '201609': 1.875
                     }

lengthOfCounter = 100.0
widthOfCounter = 8.0
triggerPos = []
normTriggerPlanDirection = []
readOutPos = []


def set_cdc_cr_parameters(period):

    if period is '201607':
        pass
    elif period is '201608a':
        pass
    elif period is '201608b':
        pass
    elif period is '201609':
        pass
    else:
        pass
    global lengthOfCounter
    global widthOfCounter
    global triggerPos
    global normTriggerPlanDirection
    global readOutPos

    lengthOfCounter = triggerSize[period][0]
    widthOfCounter = triggerSize[period][1]
    triggerPos = triggerPosition[period]
    normTriggerPlanDirection = triggerPlaneDirection[period]
    readOutPos = pmtPosition[period]


def add_cdc_cr_simulation(path, empty_path):
    """
    Add cdc cr simulation.

    """
    # Register the CRY module
    cry = register_module('CRYInput')
    # cosmic data input
    cry.param('CosmicDataDir', Belle2.FileSystem.findFile('data/generators/modules/cryinput/'))
    # user input file
    cry.param('SetupFile', 'cry.setup')
    # acceptance half-lengths - at least one particle has to enter that box to use that event
    cry.param('acceptLength', 0.6)
    cry.param('acceptWidth', 0.2)
    cry.param('acceptHeight', 0.2)
    cry.param('maxTrials', 10000)
    # keep half-lengths - all particles that do not enter the box are removed
    # (keep box >= accept box)
    cry.param('keepLength', 0.6)
    cry.param('keepWidth', 0.2)
    cry.param('keepHeight', 0.2)
    # minimal kinetic energy - all particles below that energy are ignored
    cry.param('kineticEnergyThreshold', 0.01)
    path.add_module(cry)

    # Selector module.
    sel = register_module('CDCCosmicSelector',
                          lOfCounter=lengthOfCounter,
                          wOfCounter=widthOfCounter,
                          xOfCounter=triggerPos[0],
                          yOfCounter=triggerPos[1],
                          zOfCounter=triggerPos[2],
                          phiOfCounter=0.,
                          TOP=True,
                          propSpeed=lightPropSpeed,
                          TOF=1,
                          cryGenerator=True
                          )

    path.add_module(sel)
    sel.if_false(empty_path)
    path.add_module('FullSim',
                    # Uncomment if you want to disable secondaries.
                    ProductionCut=1000000.)
    path.add_module('CDCDigitizer')


def add_cdc_cr_reconstruction(path):

    # Add cdc track finder
    add_cdc_cr_track_finding(path)

    path.add_module("SetupGenfitExtrapolation")
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=triggerPos,
                    triggerPlaneDirection=normTriggerPlanDirection,
                    useFittedInformation=False)

    path.add_module("DAFRecoFitter",
                    probCut=0.00001,
                    pdgCodesToUseForFitting=13,
                    )

    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=triggerPos,
                    triggerPlaneDirection=normTriggerPlanDirection,
                    useFittedInformation=True,
                    useReadoutPosition=True,
                    readoutPosition=readOutPos,
                    readoutPositionPropagationSpeed=lightPropSpeed
                    )

    path.add_module("DAFRecoFitter",
                    # probCut=0.00001,
                    pdgCodesToUseForFitting=13,
                    )
    path.add_module('TrackCreator',
                    defaultPDGCode=13,
                    useClosestHitToIP=True
                    )


def getRunNumber(fname):
    run = int((fname.split('/')[-1]).split('.')[3])
    return run


def getDataPeriod(run):
    period = None
    global run_range
    for key in run_range:
        if run_range[key][0] <= run <= run_range[key][1]:
            period = key
            print("Data period : " + key)
            break

    if period is None:
        B2ERROR("No valid data period is specified.")
    return period
