#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This file contains several utility functions used during the SectorMapTraining.
'''

from simulation import add_simulation
from pxd import add_pxd_reconstruction
from svd import add_svd_reconstruction
from beamparameters import add_beamparameters
import basf2 as b2
import random
from ROOT import Belle2
from generators import add_babayaganlo_generator


def add_event_generation(path, randomSeed, eventType, expNumber):
    ''' Adds the event generator and needed modules to the path. The IP is randomized so a random seed has to be provided.
      @param path: the path the modules will be added to
      @param randomSeed: a random seed to set up the random generator which randomizes the IP position.
      @param eventType: the allowed event types are
          "BBbar" which will generate BBbar events plus extra ParticleGun tracks.
          "BhaBha" which will generate radiative BhaBha events
      @param expNumber: the experiment number 0 or 1004 for full geometry, 1003 for partial PXD geometry (early phase 3).
          For training SVD only SectorMaps this setting does not matter, as SVD does not change between exp 0, 1004 and 1003.
    '''
    if (expNumber not in (0, 1003, 1004)):
        b2.B2FATAL("Specified experiment experiment number " + str(expNumber) + " not valid! Allowed numbers are 0, 1003, 1004.")

    # set the exp number
    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param("expList", [expNumber])
    path.add_module(eventinfosetter)

    # get some debugging output
    eventinfoprinter = b2.register_module('EventInfoPrinter')
    path.add_module(eventinfoprinter)

    # randomize the vertex position (flatly distributed) to make the sectormap more robust wrt. changing beam position
    # minima and maxima of the beam position given in cm
    random.seed(randomSeed)
    vertex_x_min = -0.1
    vertex_x_max = 0.1
    vertex_y_min = -0.1
    vertex_y_max = 0.1
    vertex_z_min = -0.5
    vertex_z_max = 0.5

    vertex_x = random.uniform(vertex_x_min, vertex_x_max)
    vertex_y = random.uniform(vertex_y_min, vertex_y_max)
    vertex_z = random.uniform(vertex_z_min, vertex_z_max)

    b2.B2WARNING("Setting non-default beam vertex at x= " + str(vertex_x) + " y= " + str(vertex_y) + " z= " + str(vertex_z))

    # Beam parameters
    beamparameters = add_beamparameters(path, "Y4S")
    beamparameters.param("vertex", [vertex_x, vertex_y, vertex_z])

    # additional flatly smear the muon vertex between +/- this value
    vertex_delta = 0.005  # in cm

    # Particle Gun:
    # One can add more particle gun modules if wanted.
    # allways put the particle gun
    particlegun = b2.register_module('ParticleGun')
    particlegun.logging.log_level = b2.LogLevel.WARNING
    param_pGun = {
      'pdgCodes': [13, -13],   # 13 = muon --> negatively charged!
      'nTracks': 2,
      'momentumGeneration': 'uniform',
      'momentumParams': [0.05, 4],
      'vertexGeneration': 'uniform',
      'xVertexParams': [vertex_x - vertex_delta, vertex_x + vertex_delta],            # in cm...
      'yVertexParams': [vertex_y - vertex_delta, vertex_y + vertex_delta],
      'zVertexParams': [vertex_z - vertex_delta, vertex_z + vertex_delta]
    }
    particlegun.param(param_pGun)
    path.add_module(particlegun)

    # adding event generators according to settings
    if eventType == "BBbar":
        # EvtGen Simulation:
        evtgenInput = b2.register_module('EvtGenInput')
        evtgenInput.logging.log_level = b2.LogLevel.WARNING
        path.add_module(evtgenInput)
    elif eventType == "BhaBha":
        # generate some Bhabha
        add_babayaganlo_generator(path=path, finalstate='ee')
    else:
        b2.B2FATAL("eventType: " + eventType + " is not recognized")


def add_simulation_and_reconstruction_modules(path, usePXD=False):
    '''
    Adds the required modules to the path for simulate and reconstruct events (not event generation) needed for the
    SectorMap training.
    @param path: the path the modules should be added to
    @param usePXD: If True allows to collect PXD data for SectorMap training.
    '''
    # Detector Simulation:
    add_simulation(path=path,
                   forceSetPXDDataReduction=True,  # needs to be true to override the DB settings
                   usePXDDataReduction=False,  # for training one does not want the data reduction
                   components=None)  # dont specify components because else not the whole geometry will be loaded!

    # needed for fitting
    path.add_module('SetupGenfitExtrapolation')

    # this adds the clusters for PXD and SVD which are needed in the next steps
    if usePXD:
        add_pxd_reconstruction(path=path)
    add_svd_reconstruction(path=path)

    # Setting up the MC based track finder.
    mctrackfinder = b2.register_module('TrackFinderMCTruthRecoTracks')
    mctrackfinder.param('UseCDCHits', False)
    mctrackfinder.param('UseSVDHits', True)
    mctrackfinder.param('UsePXDHits', usePXD)
    mctrackfinder.param('Smearing', False)
    mctrackfinder.param('MinimalNDF', 6)
    mctrackfinder.param('WhichParticles', ['primary'])
    mctrackfinder.param('RecoTracksStoreArrayName', 'MCRecoTracks')
    # set up the track finder to only use the first half loop of the track and discard all other hits
    mctrackfinder.param('UseNLoops', 0.5)
    mctrackfinder.param('discardAuxiliaryHits', True)
    path.add_module(mctrackfinder)

    # include a track fit into the chain (sequence adopted from the tracking scripts)
    # Correct time seed: Do I need it for VXD only tracks ????
    path.add_module("IPTrackTimeEstimator", recoTracksStoreArrayName="MCRecoTracks", useFittedInformation=False)
    # track fitting
    daffitter = b2.register_module("DAFRecoFitter")
    daffitter.param('recoTracksStoreArrayName', "MCRecoTracks")
    path.add_module(daffitter)
    # also used in the tracking sequence (multi hypothesis)
    # may be overkill
    path.add_module('TrackCreator', recoTrackColName="MCRecoTracks", pdgCodes=[211, 321, 2212])


def add_rootoutput(path, outputFileName):
    ''' Adds the RootOutput module to the path. All branches not needed for the SectorMap training will be excluded.
      @param: path the path the modules should be added.
      @param: outputFileName the name of the file the root output will be stored.
    '''
    # Root output. Default filename can be overriden with '-o' basf2 option.
    rootOutput = b2.register_module('RootOutput')
    rootOutput.param('outputFileName', outputFileName)
    # to save some space exclude everything except stuff needed for tracking
    rootOutput.param('excludeBranchNames', ["ARICHAeroHits",
                                            "ARICHDigits",
                                            "ARICHSimHits",
                                            "KLMDigits",
                                            "KLMSimHits",
                                            "CDCHits",
                                            "CDCHits4Trg",
                                            "CDCSimHits",
                                            "CDCSimHitsToCDCHits4Trg",
                                            "ECLDigits",
                                            "ECLDsps",
                                            "ECLHits",
                                            "ECLSimHits",
                                            "ECLTrigs",
                                            "ECLDiodeHits",
                                            "EKLMDigits",
                                            "EKLMSimHits",
                                            "TOPBarHits",
                                            "TOPDigits",
                                            "TOPRawDigits",
                                            "TOPSimHits"
                                            ])
    path.add_module(rootOutput)


def setup_RTCtoSPTCConverters(
        path=0,
        SVDSPscollection='SVDSpacePoints',
        PXDSPscollection='PXDSpacePoints',
        RTCinput='mcRecoTracks',
        sptcOutput='checkedSPTCs',
        usePXD=False,
        logLevel=b2.LogLevel.INFO,
        debugVal=1,
        useNoKick=False,
        useOnlyFittedTracks=False):
    """This function adds the modules needed to convert Reco-TCs to SpacePointTCs to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned. Else
      modules will be added to this path.

    @param SVDSPscollection the name of the storeArray containing SPs of  SVD.

    @param PXDSPscollection the name of the storeArray containing SPs of PXD.

    @param RTCinput defines the name of input-Reco-TCs.

    @param sptcOutput defines the name of output-SPTCs.

    @param usePXD If False pxdClusters are ignored.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG

    @param useNoKick enable the training sample selection based on track parameters (and produce a TFile of its effect)

    @param useOnlyFittedTracks: if True only fitted RecoTracks will be transformed to SpacePointTrackCands
    """
    print("setup RTCtoSPTCConverters...")

    spacePointNames = []
    detectorTypes = []
    trueHitNames = []
    clusterNames = []
    if usePXD:
        detectorTypes.append('PXD')
        # PXD SpacePoints and SVD SpacePoints are assumed to be in the same StoreArray
        spacePointNames.append(PXDSPscollection)
        trueHitNames.append('')
        clusterNames.append('')
    # PXD SpacePoints and SVD SpacePoints are assumed to be in the same StoreArray
    spacePointNames.append(SVDSPscollection)
    detectorTypes.append('SVD')
    trueHitNames.append('')
    clusterNames.append('')

    # module to create relations between SpacePoints and TrueHits -> some of
    # the following modules will be utilizing these relations!
    sp2thConnector = b2.register_module('SpacePoint2TrueHitConnector')
    sp2thConnector.logging.log_level = logLevel
    sp2thConnector.param('DetectorTypes', detectorTypes)
    sp2thConnector.param('TrueHitNames', trueHitNames)
    sp2thConnector.param('ClusterNames', clusterNames)
    sp2thConnector.param('SpacePointNames', spacePointNames)
    sp2thConnector.param('outputSuffix', '_relTH')
    sp2thConnector.param('storeSeperate', False)
    sp2thConnector.param('registerAll', False)
    sp2thConnector.param('maxGlobalPosDiff', 0.05)
    sp2thConnector.param('maxPosSigma', 5)
    sp2thConnector.param('minWeight', 0)
    sp2thConnector.param('requirePrimary', True)
    sp2thConnector.param('positionAnalysis', False)

    # TCConverter, RecoTrack -> SPTC
    recoTrackCandConverter = b2.register_module('RT2SPTCConverter')
    recoTrackCandConverter.logging.log_level = logLevel
    recoTrackCandConverter.param('RecoTracksName', RTCinput)
    recoTrackCandConverter.param('SpacePointTCName', 'SPTracks')
    recoTrackCandConverter.param('SVDSpacePointStoreArrayName', SVDSPscollection)
    recoTrackCandConverter.param('PXDSpacePointStoreArrayName', None)
    if usePXD:
        recoTrackCandConverter.param('PXDSpacePointStoreArrayName', PXDSPscollection)
    recoTrackCandConverter.param('useTrueHits', True)
    recoTrackCandConverter.param('ignorePXDHits', not usePXD)  # if True PXD hits will be ignored
    recoTrackCandConverter.param('useSingleClusterSP', False)
    recoTrackCandConverter.param('minSP', 3)
    recoTrackCandConverter.param('skipProblematicCluster', False)
    recoTrackCandConverter.param('convertFittedOnly', useOnlyFittedTracks)

    NoKickCuts = Belle2.FileSystem.findFile("data/tracking/NoKickCuts.root")

    if useNoKick:
        recoTrackCandConverter.param('noKickCutsFile', NoKickCuts)  # NoKickCuts applied
        recoTrackCandConverter.param('noKickOutput', True)  # produce output TFile of NoKickCuts
    else:
        recoTrackCandConverter.param('noKickCutsFile', "")  # NoKickCuts not applied

    # SpacePointTrackCand referee
    sptcReferee = b2.register_module('SPTCReferee')
    sptcReferee.logging.log_level = logLevel
    sptcReferee.param('sptcName', 'SPTracks')
    sptcReferee.param('newArrayName', sptcOutput)
    sptcReferee.param('storeNewArray', True)
    sptcReferee.param('checkCurling', True)
    sptcReferee.param('splitCurlers', True)
    sptcReferee.param('keepOnlyFirstPart', True)
    sptcReferee.param('kickSpacePoint', True)
    sptcReferee.param('checkSameSensor', True)
    sptcReferee.param('useMCInfo', True)
    # sptcReferee.logging.log_level = LogLevel.DEBUG

    if path == 0:
        return [sp2thConnector, recoTrackCandConverter, sptcReferee]
    else:
        path.add_module(sp2thConnector)
        path.add_module(recoTrackCandConverter)
        path.add_module(sptcReferee)
        return None


def add_training_data_collector(path, usePXD=False, nameTag='', outputDir='./', use_noKick=False):
    ''' Adds the modules which extract and collect the actual training data from simulated events.
        @param path: The basf2 path to add the modules
        @param usePXD: If False PXD hits will be ignored
        @param nameTag: Output file names are generated automatically. With this name tag can be set to have a unique
          output file name. It will be added at the end of output file name.
        @param outputDir: The output file will be written to that directory.
        @param use_noKick: Activates alternative trainings method. Note: This trainings method is experimental and not maintained
          anymore. So if you dont know what you do, keep it set to False!'''

    # puts the geometry and gearbox in the path
    if 'Gearbox' not in path:
        gearbox = b2.register_module('Gearbox')
        path.add_module(gearbox)
    # the geometry is loaded from the DB by default now! The correct geometry
    # should be pickked according to exp number for the generated events.
    if 'Geometry' not in path:
        geometry = b2.register_module('Geometry')
        path.add_module(geometry)

    # Converts RecoTrack candidates and checks them, with respect to the SecMap settings
    # Produces SpacePoint track candidates which are used in the VXDTFTrainingDataCollector.
    setup_RTCtoSPTCConverters(path=path,
                              SVDSPscollection='SVDSpacePoints',
                              PXDSPscollection='PXDSpacePoints',
                              RTCinput='MCRecoTracks',
                              sptcOutput='checkedSPTCs',
                              usePXD=usePXD,
                              logLevel=b2.LogLevel.ERROR,
                              useNoKick=use_noKick,
                              useOnlyFittedTracks=True)  # train on fitted tracks only

    # SecMap BootStrap
    # Module to fetch SecMap Config and store or load SecMap Training.
    # Config is defined in /tracking/modules/vxdtfRedesing/src/SectorMapBootstrapModule.cc
    # and must be available for the training of the SecMap
    # Double False for "ReadSectorMap" and "WriteSectorMap" only fetches config.
    secMapBootStrap = b2.register_module('SectorMapBootstrap')
    secMapBootStrap.param('ReadSectorMap', False)
    secMapBootStrap.param('WriteSectorMap', False)
    path.add_module(secMapBootStrap)

    # Module for generation of train sample for SecMap Training
    if usePXD:
        nameTag += 'VXD'
    else:
        nameTag += 'SVDOnly'

    SecMapTrainerBase = b2.register_module('VXDTFTrainingDataCollector')
    SecMapTrainerBase.param('outputDir', outputDir)
    SecMapTrainerBase.param('NameTag', nameTag)
    SecMapTrainerBase.param('SpacePointTrackCandsName', 'checkedSPTCs')
    path.add_module(SecMapTrainerBase)


def create_unique_random_numbers(n=500, random_seed=12345, n_min=9999, n_max=9999999):
    ''' Creates randomly a unique set of random numbers of size n. WARNING: if n is close or equal to the
      specified range this will take very long!
      @param n number of random numbers
      @param random_seed random seed to initialize the random generator which generates the random numbers
      @patam n_min minimum value for the generated random numbers
      @param n_max maximum value for the generated random numbers
    '''

    # you cannot create more random numbers than numbers in range.
    if (n_max - n_min + 1) < n or n_min > n_max:
        b2.B2ERROR(
            "Range (" +
            str(n_min) +
            "," +
            str(n_max) +
            ") not well defined or not great enough for that many random numbers (n=" +
            str(n) +
            ")")
        return []

    random.seed(random_seed)

    # as there is no "do while" in python
    while True:
        my_list = [random.randint(n_min, n_max) for i in range(0, n)]
        if (len(my_list) == len(set(my_list))):
            return my_list


def remove_timing_cuts_from_SectorMap(sectorMapFile, setupToRead="SVDOnlyDefault"):
    '''
    Simple function to remove the timing cuts from a given SectorMap and stores the
    new SectorMap to a file
    @param sectorMapFile Name (including path) of the SectorMap to be altered.
    @param setupToRead SectorMap files can store different setups. This parameter specifies the name
        of the setup which should be read.
        Currently in use should only be the name given as default.
     NOTE: This script should only be used by experts!
    '''

    import basf2 as b2

    # Using the SectorMapBootstrap module to read the sectormap from file
    SMBSM1 = b2.register_module("SectorMapBootstrap")
    SMBSM1.param("ReadSecMapFromDB", False)
    SMBSM1.param("ReadSectorMap", True)
    SMBSM1.param("SectorMapsInputFile", sectorMapFile)
    SMBSM1.param("SetupToRead", setupToRead)

    if not sectorMapFile.endswith(".root"):
        b2.B2FATAL("SectorMaps are supposed to be root - files! Provided name does not end on \".root\"")

    # assumes it is a root file so replace the last 5 letters
    outputMapFile = sectorMapFile[:-5] + '_timingRemoved.root'
    # the following setting will make the module write the altered sectormap to a root file
    SMBSM1.param("SectorMapsOutputFile", outputMapFile)
    SMBSM1.param("WriteSectorMap", True)

    ##################################################################
    # now the actual alteration of the cut values
    ###############################################################

    # NOTE: the indizes in the example below may have changed if the code changed! So you have to cross check!

    # three hit filter
    # (#19 <= DistanceInTime <= #20)
    SMBSM1.param('threeHitFilterAdjustFunctions', [(19, "-TMath::Infinity()"), (20, "TMath::Infinity()")])
    # two hit filters:
    # (#12 <= DistanceInTimeUside <= #13)
    # (#10 <= DistanceInTimeVside <= #11)
    SMBSM1.param('twoHitFilterAdjustFunctions', [(12, "-TMath::Infinity()"), (13, "TMath::Infinity()"),
                                                 (10, "-TMath::Infinity()"), (11, "TMath::Infinity()")])

    # this will, in addition to other debbugging output, print the original filter ("BEFORE")
    # and the altered filter ("AFTER") to the screen.
    # NOTE: there are order of 10th of thousends of filters both for 2-hits and 3-hits. So expect lots of output.
    # SMBSM1.logging.log_level = LogLevel.DEBUG

    # create path
    main = b2.create_path()
    # EventInfoSetter needed else basf2 complains as it is afraid of having no EventMetaData
    main.add_module('EventInfoSetter')
    main.add_module(SMBSM1)
    b2.print_path(main)
    # run path
    b2.process(main)
    print(b2.statistics)
