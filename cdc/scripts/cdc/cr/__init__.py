from basf2 import *
from ROOT import Belle2
from tracking import add_cdc_cr_track_finding
from tracking import add_cdc_track_finding
from tracking import add_cdc_cr_track_fit_and_track_creator
from time_extraction_helper_modules import *

# Propagation velocity of the light in the scinti.
lightPropSpeed = 12.9925

# Run range.
run_range = {'201607': [787, 833],
             '201608a': [885, 896],
             '201608b': [917, 924],
             '201609': [966, 973],
             '201702': [1601, 9999],
             'noamal': [-1, -1]
             }
# Size of trigger counter.
triggerSize = {'201607': [20.0, 6.0, 10.0],
               '201608a': [100.0, 8.0, 10.0],
               '201608b': [100.0, 8.0, 10.0],
               '201609': [100.0, 8.0, 10.0],
               '201702': [100.0, 8.0, 10.0],
               'normal': [100.0, 8.0, 10.0]
               }
# Center position of trigger counter.
triggerPosition = {'201607': [0.3744, 0.0, -1.284],
                   '201608a': [-1.87, -1.25, 18.7],
                   '201608b': [-1.87, -1.25, 11.0],
                   '201609': [0, 0, 11.0],
                   '201702': [0., -1.5, 21.0],
                   'normal': [0.0, 0.0, 0.0]
                   }

# Normal direction of the trigger plane.
triggerPlaneDirection = {'201607': [1, -1, 0],
                         '201608a': [0, 1, 0],
                         '201608b': [0, 1, 0],
                         '201609': [0, 1, 0],
                         '201702': [0, 1, 0],
                         'normal': [0, 1, 0]
                         }

# PMT position.
pmtPosition = {'201607': [0, 0, 0],
               '201608a': [-1.87, 0, -25.0],
               '201608b': [-1.87, 0, -42.0],
               '201609': [0, 0, -42.0],
               '201702': [0., -1.5, -31.0],
               'normal': [0, 0, -50.0]
               }

# Global phi rotation.
globalPhiRotation = {'201607': 1.875,
                     '201608a': 1.875,
                     '201608b': 1.875,
                     '201609': 1.875,
                     '201702': 0.0,
                     'normal': 0.0
                     }

lengthOfCounter = 100.0
widthOfCounter = 8.0
triggerPos = []
normTriggerPlaneDirection = []
readOutPos = []
globalPhi = 0.0
cosmics_period = None


def set_cdc_cr_parameters(period):

    global lengthOfCounter
    global widthOfCounter
    global triggerPos
    global normTriggerPlaneDirection
    global readOutPos
    global globalPhi
    global cosmics_period

    lengthOfCounter = triggerSize[period][0]
    widthOfCounter = triggerSize[period][1]
    triggerPos = triggerPosition[period]
    normTriggerPlaneDirection = triggerPlaneDirection[period]
    readOutPos = pmtPosition[period]
    globalPhi = globalPhiRotation[period]
    cosmics_period = period


def add_cdc_cr_reconstruction(path, eventTimingExtraction=False, topInCounter=True):
    """
    Add CDC CR reconstruction
    """

    # Add cdc track finder
    add_cdc_cr_track_finding(path, merge_tracks=False)

    # Setup Genfit extrapolation
    path.add_module("SetupGenfitExtrapolation")

    # Time seed
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=triggerPos,
                    triggerPlaneDirection=normTriggerPlaneDirection,
                    useFittedInformation=False)

    # Initial track fitting
    path.add_module("DAFRecoFitter",
                    probCut=0.00001,
                    pdgCodesToUseForFitting=13,
                    )

    if topInCounter is True:
        # Correct time seed with TOP in counter.
        path.add_module("PlaneTriggerTrackTimeEstimator",
                        pdgCodeToUseForEstimation=13,
                        triggerPlanePosition=triggerPos,
                        triggerPlaneDirection=normTriggerPlaneDirection,
                        useFittedInformation=True,
                        useReadoutPosition=True,
                        readoutPosition=readOutPos,
                        readoutPositionPropagationSpeed=lightPropSpeed
                        )

        # Track fitting
        path.add_module("DAFRecoFitter",
                        # probCut=0.00001,
                        pdgCodesToUseForFitting=13,
                        )

    if eventTimingExtraction is True:
        # Extract the time
        path.add_module("FullGridTrackTimeExtraction",
                        recoTracksStoreArrayName="RecoTracks",
                        maximalT0Shift=70,
                        minimalT0Shift=-70,
                        numberOfGrids=6
                        )

        # Track fitting
        path.add_module("DAFRecoFitter",
                        # probCut=0.00001,
                        pdgCodesToUseForFitting=13,
                        )

    # Create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator',
                    defaultPDGCode=13,
                    useClosestHitToIP=True
                    )


def getExpNumber(fname):
    """
    Get expperimental number from file name.
    """
    exp = int((fname.split('/')[-1]).split('.')[2])
    return exp


def getRunNumber(fname):
    """
    Get run number from file name.
    """
    run = int(((fname.split('/')[-1]).split('.')[3]).split('_')[0])
    return run


def getDataPeriod(run):
    """
    Get data period from run number
    It should be replaced the argument from run to (exp, run)!
    """
    period = None
    global run_range

    for key in run_range:
        if run_range[key][0] <= run <= run_range[key][1]:
            period = key
            print("Data period : " + key)
            break

    if period is None:
        B2WARNING("No valid data period is specified.")
        B2WARNING("Default configuration is loaded.")
        period = 'normal'
    return period


def getPhiRotation():
    global globalPhi
    return(globalPhi)
