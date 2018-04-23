from basf2 import *
from ROOT import Belle2
import ROOT
from tracking import add_cdc_cr_track_finding
from time_extraction_helper_modules import *

# Propagation velocity of the light in the scinti.
lightPropSpeed = 12.9925

# Run range.
run_range = {'201607': [787, 833],
             '201608a': [885, 896],
             '201608b': [917, 924],
             '201609': [966, 973],
             '201702': [1601, 9999],
             'gcr2017': [3058, 100000],
             'phase2': [0, 100000],
             'phase3': [0, 100000],
             'normal': [-1, -1]
             }
# Size of trigger counter.
triggerSize = {'201607': [20.0, 6.0, 10.0],
               '201608a': [100.0, 8.0, 10.0],
               '201608b': [100.0, 8.0, 10.0],
               '201609': [100.0, 8.0, 10.0],
               '201702': [100.0, 8.0, 10.0],
               'gcr2017': [100, 0, 8.0, 10.0],
               'phase2': [100, 0, 8.0, 10.0],
               'phase3': [100, 0, 8.0, 10.0],
               'normal': [100.0, 8.0, 10.0]
               }
# Center position of trigger counter.
triggerPosition = {'201607': [0.3744, 0.0, -1.284],
                   '201608a': [-1.87, -1.25, 18.7],
                   '201608b': [-1.87, -1.25, 11.0],
                   '201609': [0, 0, 11.0],
                   '201702': [0., -1.5, 21.0],
                   'gcr2017': [0.0, 0.0, 0.0],
                   'phase2': [0.0, 0.0, 0.0],
                   'phase3': [0.0, 0.0, 0.0],
                   'normal': [0.0, 0.0, 0.0]
                   }

# Normal direction of the trigger plane.
triggerPlaneDirection = {'201607': [1, -1, 0],
                         '201608a': [0, 1, 0],
                         '201608b': [0, 1, 0],
                         '201609': [0, 1, 0],
                         '201702': [0, 1, 0],
                         'gcr2017': [0, 1, 0],
                         'phase2': [0, 1, 0],
                         'phase3': [0, 1, 0],
                         'normal': [0, 1, 0]
                         }

# PMT position.
pmtPosition = {'201607': [0, 0, 0],
               '201608a': [-1.87, 0, -25.0],
               '201608b': [-1.87, 0, -42.0],
               '201609': [0, 0, -42.0],
               '201702': [0., -1.5, -31.0],
               'gcr2017': [0.0, 0.0, -50.0],
               'phase2': [0.0, 0.0, -50.0],
               'phase3': [0.0, 0.0, -50.0],
               'normal': [0, 0, -50.0]
               }

# Global phi rotation.
globalPhiRotation = {'201607': 1.875,
                     '201608a': 1.875,
                     '201608b': 1.875,
                     '201609': 1.875,
                     '201702': 0.0,
                     'gcr2017': 0.0,
                     'phase2': 0.0,
                     'phase3': 0.0,
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


def add_cdc_cr_simulation(path,
                          components=None,
                          bkgfiles=None,
                          bkgcomponents=None,
                          bkgscale=1.0,
                          bkgOverlay=False,
                          generate_2nd_cdc_hits=False,
                          topInCounter=True):
    """
    Add CDC CR simulation.

    """
    empty_path = create_path()

    # background mixing or overlay input before process forking
    if bkgfiles:
        if bkgOverlay:
            bkginput = register_module('BGOverlayInput')
            bkginput.param('inputFileNames', bkgfiles)
            path.add_module(bkginput)
        else:
            bkgmixer = register_module('BeamBkgMixer')
            bkgmixer.param('backgroundFiles', bkgfiles)
            if bkgcomponents:
                bkgmixer.param('components', bkgcomponents)
            else:
                if components:
                    bkgmixer.param('components', components)
            bkgmixer.param('overallScaleFactor', bkgscale)
            path.add_module(bkgmixer)

    # geometry parameter database
    if 'Gearbox' not in path:
        gearbox = register_module('Gearbox')
        path.add_module(gearbox)

    # detector geometry
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # detector simulation
    if 'FullSim' not in path:
        g4sim = register_module('FullSim',
                                ProductionCut=1000000.)
        path.add_module(g4sim)

    #    path.add_module(RandomizeTrackTimeModule(8.0))

    # CDC digitization
    if components is None or 'CDC' in components:
        cdc_digitizer = register_module('CDCDigitizer')
        cdc_digitizer.param("Output2ndHit", generate_2nd_cdc_hits)
        path.add_module(cdc_digitizer)

    # ECL digitization
    if components is None or 'ECL' in components:
        ecl_digitizer = register_module('ECLDigitizer')
        if bkgfiles is not None:
            ecl_digitizer.param('Background', 1)
        path.add_module(ecl_digitizer)


def add_cdc_cr_reconstruction(path, eventTimingExtraction=True,
                              topInCounter=False,
                              pval2ndTrial=0.001):
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

    # Correct time seed with TOP in counter.
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=triggerPos,
                    triggerPlaneDirection=normTriggerPlaneDirection,
                    useFittedInformation=True,
                    useReadoutPosition=topInCounter,
                    readoutPosition=readOutPos,
                    readoutPositionPropagationSpeed=lightPropSpeed
                    )

    # Track fitting
    path.add_module("DAFRecoFitter",
                    probCut=pval2ndTrial,
                    pdgCodesToUseForFitting=13
                    )

    if eventTimingExtraction is True:
        # Extract the time
        path.add_module("FullGridTrackTimeExtraction",
                        RecoTracksStoreArrayName="RecoTracks",
                        maximalT0Shift=40,
                        minimalT0Shift=-40,
                        numberOfGrids=6
                        )

        # Track fitting
        path.add_module("DAFRecoFitter",
                        probCut=pval2ndTrial,
                        pdgCodesToUseForFitting=13
                        )

    # Create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackCreator',
                    pdgCodes=[13],
                    useClosestHitToIP=True,
                    useBFieldAtHit=True
                    )


def getExpRunNumber(fname):
    """
    Get expperimental number and run number from file name.
    """
    f = ROOT.TFile(fname)
    t = f.Get('tree')
    t.GetEntry(0)
    e = t.EventMetaData
    exp = e.getExperiment()
    run = e.getRun()
    f.Close()
    return [exp, run]


def getRunNumber(fname):
    """
    Get run number from file name.
    """
    f = ROOT.TFile(fname)
    t = f.Get('tree')
    t.GetEntry(0)
    e = t.EventMetaData
    run = e.getRun()
    f.Close()
    return run


def getDataPeriod(exp=0, run=0):
    """
    Get data period from run number
    It should be replaced the argument from run to (exp, run)!
    """
    period = None

    if exp is 1:  # GCR2017
        return 'gcr2017'

    if exp is 2:  # GCR2
        return 'phase2'

    if exp > 2:  # Phase3
        return 'phase3'

    # Pre global cosmics until March 2017
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


def getMapperAngle(exp=1, run=3118):
    '''
    Get B field mapper angle from exp and run number.
    '''
    if exp == 1:
        if run < 3883:
            return 16.7
        else:
            return 43.3
    else:
        return None


def getTriggerType(exp=1, run=3118):
    '''
    Get trigger type from exp and run number.
    '''
    if exp == 1:
        if run >= 3100 and run <= 3600:
            return 'b2b'
        elif run >= 3642 and run <= 4018:
            return 'single'
        else:
            return None
    else:
        return None


def add_GCR_Trigger_simulation(path, backToBack=False, skipEcl=True):
    """
    function to simulate trigger for GCR cosmics 2017. use CDC+ECL trigger
    :param path: path want to add trigger simulation
    :param backToBack: if true back to back TSF2; if false single TSF2
    :param skipEcl: ignore ECL in trigger, just use CDC TSF2
    """
    empty_path = create_path()
    path.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"))
    if not skipEcl:
        path.add_module('TRGECLFAM',
                        TCWaveform=0,
                        FAMFitMethod=1,
                        TCThreshold=100,
                        BeamBkgTag=0,
                        ShapingFunction=1)
        path.add_module('TRGECL',
                        Clustering=0,
                        EventTiming=1,
                        Bhabha=0,
                        EventSelect=0,
                        TimeWindow=375)

    TSF = path.add_module('TRGGDLCosmicRun',
                          BackToBack=backToBack,
                          skipECL=skipEcl)
    TSF.if_false(empty_path)
