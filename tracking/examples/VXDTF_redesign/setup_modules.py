#!/usr/bin/env python3
# -*- coding: utf-8 -*-


#####################################################################
# VXDTF2 Setup Functions
#
# This file contains several convenience functions to setup the
# modules needed for the module chains of the VXDTF2.
#
# Contributors: Jonas Wagner, Felix Metzner
#####################################################################


from basf2 import *
from ROOT import Belle2

import os

# sets up the geometry. The file for the default geometry for Belle2 is geometry/Belle2.xml


def setup_Geometry(path=None):
    """
    Sets the geometry. Should be used in all VXDTF2 related scripts to ensure to use the same geometry in all
    trainings / validation steps!
    param path: the path to append the geometry

    """

    # If the environmental variable USE_BEAST2_GEOMETRY is set the Beast2 geometry will be used, else the default Belle2 geometry
    fileName = 'geometry/Belle2.xml'
    if os.environ.get('USE_BEAST2_GEOMETRY'):
        print('WARNING: will use the Beast2 geometry: geometry/Beast2_phase2.xml')
        fileName = 'geometry/Beast2_phase2.xml'
    print('The used geometry is ' + fileName)

    # Gearbox
    gearbox = register_module('Gearbox')
    gearbox.param('fileName', fileName)
    path.add_module(gearbox)

    # Geometry
    geometry = register_module('Geometry')
    geometry.param('components', ['BeamPipe',
                                  'MagneticFieldConstant4LimitedRSVD',
                                  'PXD',
                                  'SVD',
                                  'CDC'])
    path.add_module(geometry)


def setup_VXDTF2(path=None,
                 use_pxd=False,
                 use_svd=True,
                 filter_overlapping=True,
                 use_segment_network_filters=True,
                 observerType=0,
                 quality_estimator='circleFit',
                 overlap_filter='greedy',
                 sec_map_file=None,
                 setup_name='SVDOnlyDefault',
                 log_level=LogLevel.INFO,
                 debug_level=1):
    """
    Convenience Method to setup the redesigned vxd track finding module chain.
    Reuslt is a store array containing reco tracks called 'RecoTracks'.
    :param path: basf2.Path
    :param sec_map_file: training data for segment network.
    :param setup_name: name of the setup within all the sectormaps in the sec_map_file which should be used,
    :param use_pxd: if true use pxd hits. Default False.
    :param use_svd: if true use svd hits. Default True.
    :param quality_estimator: which fit to use to determine track quality. Options 'circle', 'random'. Default 'circle'.
    :param filter_overlapping: if true overlapping tracks are reduced to a single track using the qualitiy indicator.
    :param use_segment_network_filters: if true use filters for segmentMap training. Default True.
    :param observe_network_filters: FOR DEBUG ONLY! If true results for FilterVariables are stored to a root file. Default False.
    :param overlap_filter: which filter network to use. Options 'hopfield', 'greedy'. Default 'hopfield'.
    :param log_level: LogLevel of all modules in the chain
    :param debug_level: debug level of all modules in the chain.
    :return:
    """

    # List containing all modules either to be added to the path or to be returned.
    modules = []

    #################
    # VXDTF2 Step 0
    # Preparation
    #################
    if use_pxd:
        spCreatorPXD = register_module('PXDSpacePointCreator')
        spCreatorPXD.logging.log_level = log_level
        spCreatorPXD.logging.debug_level = debug_level
        spCreatorPXD.param('NameOfInstance', 'PXDSpacePoints')
        spCreatorPXD.param('SpacePoints', 'SpacePoints')
        modules.append(spCreatorPXD)

    if use_svd:
        spCreatorSVD = register_module('SVDSpacePointCreator')
        spCreatorSVD.logging.log_level = log_level
        spCreatorSVD.logging.debug_level = debug_level
        spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
        spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
        spCreatorSVD.param('SpacePoints', 'SpacePoints')
        modules.append(spCreatorSVD)

    # SecMap Bootstrap
    secMapBootStrap = register_module('SectorMapBootstrap')
    secMapBootStrap.param('ReadSectorMap', True)
    if sec_map_file:
        secMapBootStrap.param('SectorMapsInputFile', sec_map_file)
    secMapBootStrap.param('WriteSectorMap', False)
    secMapBootStrap.param('SetupToRead', "")
    modules.append(secMapBootStrap)

    ##################
    # VXDTF2 Step 1
    # SegmentNet
    ##################

    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('CreateNeworks', 3)
    segNetProducer.param('NetworkOutputName', 'test2Hits')
    segNetProducer.param('allFiltersOff', not use_segment_network_filters)
    segNetProducer.param('SpacePointsArrayNames', ['SpacePoints'])
    segNetProducer.param('printNetworks', False)
    segNetProducer.param('sectorMapName', setup_name)
    segNetProducer.param('addVirtualIP', False)
    segNetProducer.param('observerType', observerType)
    segNetProducer.logging.log_level = log_level
    segNetProducer.logging.debug_level = debug_level
    modules.append(segNetProducer)

    #################
    # VXDTF2 Step 2
    # TrackFinder
    #################

    cellOmat = register_module('TrackFinderVXDCellOMat')
    cellOmat.param('NetworkName', 'test2Hits')
    cellOmat.param('SpacePointTrackCandArrayName', 'SPTCs')
    cellOmat.param('printNetworks', False)
    cellOmat.param('strictSeeding', True)
    cellOmat.logging.log_level = log_level
    cellOmat.logging.debug_level = debug_level
    modules.append(cellOmat)

    #################
    # VXDTF2 Step 3
    # Analyzer
    #################

    # Quality

    qualityEstimator = register_module('QualityEstimatorVXD')
    qualityEstimator.param('EstimationMethod', quality_estimator)
    qualityEstimator.param('SpacePointTrackCandsStoreArrayName', 'SPTCs')
    qualityEstimator.logging.log_level = log_level
    qualityEstimator.logging.debug_level = debug_level
    modules.append(qualityEstimator)

    maxCandidateSelection = register_module('BestVXDTrackCandidatesSelector')
    maxCandidateSelection.param('NameSpacePointTrackCands', 'SPTCs')
    maxCandidateSelection.param('SubsetCreation', True)
    maxCandidateSelection.param('NewNameSpacePointTrackCands', '')
    maxCandidateSelection.logging.log_level = log_level
    maxCandidateSelection.logging.debug_level = debug_level
    modules.append(maxCandidateSelection)

    # Properties
    vIPRemover = register_module('SPTCvirtualIPRemover')
    vIPRemover.param('tcArrayName', '')
    vIPRemover.param('maxTCLengthForVIPKeeping', 0)  # want to remove virtualIP for any track length
    vIPRemover.logging.log_level = log_level
    vIPRemover.logging.debug_level = debug_level
    modules.append(vIPRemover)

    #################
    # VXDTF2 Step 4
    # OverlapFilter
    #################

    if filter_overlapping:
        overlapResolver = register_module('SVDOverlapResolver')
        overlapResolver.logging.log_level = log_level
        overlapResolver.logging.debug_level = debug_level
        overlapResolver.param('NameSpacePointTrackCands', '')
        overlapResolver.param('ResolveMethod', overlap_filter.lower())
        overlapResolver.param('NameSVDClusters', '')
        modules.append(overlapResolver)

    #################
    # VXDTF2 Step 5
    # Converter
    #################
    momSeedRetriever = register_module('SPTCmomentumSeedRetriever')
    momSeedRetriever.param('tcArrayName', '')
    momSeedRetriever.logging.log_level = log_level
    momSeedRetriever.logging.debug_level = debug_level
    modules.append(momSeedRetriever)

    converter = register_module('SPTC2RTConverter')
    converter.param('spacePointsTCsStoreArrayName', '')
    converter.logging.log_level = log_level
    converter.logging.debug_level = debug_level
    modules.append(converter)

    if path:
        for module in modules:
            path.add_module(module)
    else:
        return modules


def setup_RTCtoSPTCConverters(
        path=0,
        SPscollection='SpacePoints',
        RTCinput='mcTracks',
        sptcOutput='checkedSPTCs',
        usePXD=True,
        logLevel=LogLevel.INFO,
        debugVal=1,
        useNoKick=False):
    """This function adds the modules needed to convert Reco-TCs to SpacePointTCs to given path.

    @param path if set to 0 (standard) the created modules will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param SPscollection the name of the storeArray containing SPs of both SVD and PXD.

    @param RTCinput defines the name of input-Reco-TCs.

    @param sptcOutput defines the name of output-SPTCs.

    @param usePXD If False pxdClusters are ignored.

    @param logLevel set to logLevel level of your choice.

    @param debugVal set to debugLevel of choice - will be ignored if logLevel is not set to LogLevel.DEBUG

    @param useNoKick enable the training sample selection based on track parameters (and produce a TFile of its effect)
    """
    print("setup RTCtoSPTCConverters...")
    spacePointNames = []
    detectorTypes = []
    trueHitNames = []
    clusterNames = []
    if usePXD:
        detectorTypes.append('PXD')
        # PXD SpacePoints and SVD SpacePoints are assumed to be in the same StoreArray
        spacePointNames.append(SPscollection)
        trueHitNames.append('')
        clusterNames.append('')
    # PXD SpacePoints and SVD SpacePoints are assumed to be in the same StoreArray
    spacePointNames.append(SPscollection)
    detectorTypes.append('SVD')
    trueHitNames.append('')
    clusterNames.append('')

    # module to create relations between SpacePoints and TrueHits -> some of
    # the following modules will be utilizing these relations!
    sp2thConnector = register_module('SpacePoint2TrueHitConnector')
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
    recoTrackCandConverter = register_module('RT2SPTCConverter')
    recoTrackCandConverter.logging.log_level = logLevel
    recoTrackCandConverter.param('RecoTracksName', RTCinput)
    recoTrackCandConverter.param('SpacePointTCName', 'SPTracks')
    recoTrackCandConverter.param('SVDandPXDSPName', SPscollection)
    recoTrackCandConverter.param('useTrueHits', True)
    recoTrackCandConverter.param('ignorePXDHits', not usePXD)  # if True PXD hits will be ignored
    recoTrackCandConverter.param('useSingleClusterSP', False)
    recoTrackCandConverter.param('minSP', 3)
    recoTrackCandConverter.param('skipProblematicCluster', False)

    if os.environ.get('USE_BEAST2_GEOMETRY'):
        NoKickCuts = Belle2.FileSystem.findFile("data/tracking/NoKickCutsPhase2.root")
    else:
        NoKickCuts = Belle2.FileSystem.findFile("data/tracking/NoKickCuts.root")

    if useNoKick:
        recoTrackCandConverter.param('noKickCutsFile', NoKickCuts)  # NoKickCuts applied
        recoTrackCandConverter.param('noKickOutput', True)  # produce output TFile of NoKickCuts
    else:
        recoTrackCandConverter.param('noKickCutsFile', "")  # NoKickCuts not applied

    # SpacePointTrackCand referee
    sptcReferee = register_module('SPTCReferee')
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

    if path is 0:
        return [sp2thConnector, recoTrackCandConverter, sptcReferee]
    else:
        path.add_module(sp2thConnector)
        path.add_module(recoTrackCandConverter)
        path.add_module(sptcReferee)
        return None


def setup_sim(path=0, useEDeposit=True, useMultipleScattering=True, allowDecay=True, verbose=False):
    """This function adds the g4simulation to given path.

    @param path if set to 0 (standard) g4sim will not be added, but returned.
    If a path is given, 'None' is returned but will be added to given path instead.

    @param useEDeposit if False, particles will not have eDeposit. WARMING: if you still want to get hits for that case:
    useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
    where you have to activate see neutrons = true.

    @param useMultipleScattering if False, multiple scattering in matter is deactivated.

    @param allowDecay if False, particles can not decay.

    @param verbose if True, some extra debug output is given.
    """
    print("setup FullSim...")
    g4sim = register_module('FullSim')
    g4sim.param('StoreAllSecondaries', True)
    uiCommandList = []
    if verbose:
        uiCommandList.append('/process/list')  # prints list of processes available
    if allowDecay:
        uiCommandList.append('/process/inactivate              Decay all')
    if not useEDeposit:
        uiCommandList.append('/process/inactivate        StepLimiter all')
        uiCommandList.append('/process/inactivate             muIoni all')
        uiCommandList.append('/process/inactivate              hIoni all')
        uiCommandList.append('/process/inactivate            ionIoni all')
        uiCommandList.append('/process/inactivate              eIoni all')
        uiCommandList.append('/process/inactivate              eBrem all')
        uiCommandList.append('/process/inactivate             hBrems all')
        uiCommandList.append('/process/inactivate            muBrems all')
        uiCommandList.append('/process/inactivate          hPairProd all')
        uiCommandList.append('/process/inactivate         muPairProd all')
        uiCommandList.append('/process/inactivate            annihil all')
        uiCommandList.append('/process/inactivate               phot all')
        uiCommandList.append('/process/inactivate              compt all')
        uiCommandList.append('/process/inactivate               conv all')
        # uiCommandList.append('/process/inactivate    PhotonInelastic')
        # uiCommandList.append('/process/inactivate    photonInelastic')
        uiCommandList.append('/process/inactivate    electronNuclear all')
        uiCommandList.append('/process/inactivate    positronNuclear all')
        uiCommandList.append('/process/inactivate        muonNuclear all')
        uiCommandList.append('/process/inactivate      photonNuclear all')
        uiCommandList.append('/process/inactivate              Decay all')
        uiCommandList.append('/process/inactivate         hadElastic all')
        uiCommandList.append('/process/inactivate   neutronInelastic all')
        uiCommandList.append('/process/inactivate           nCapture all')
        uiCommandList.append('/process/inactivate           nFission all')
        uiCommandList.append('/process/inactivate    protonInelastic all')
        uiCommandList.append('/process/inactivate    ionInelastic all')
        uiCommandList.append('/process/inactivate  pi+Inelastic all')
        uiCommandList.append('/process/inactivate pi-Inelastic all')
        uiCommandList.append('/process/inactivate  kaon+Inelastic all')
        uiCommandList.append('/process/inactivate kaon-Inelastic all')
        uiCommandList.append('/process/inactivate kaon0LInelastic all')
        uiCommandList.append('/process/inactivate kaon0SInelastic all')
        uiCommandList.append('/process/inactivate anti_protonInelastic all')
        uiCommandList.append('/process/inactivate anti_neutronInelastic all')
        uiCommandList.append('/process/inactivate    lambdaInelastic all')
        uiCommandList.append('/process/inactivate anti-lambdaInelastic all')
        uiCommandList.append('/process/inactivate sigma-Inelastic all')
        uiCommandList.append('/process/inactivate anti_sigma-Inelastic all')
        uiCommandList.append('/process/inactivate sigma+Inelastic all')
        uiCommandList.append('/process/inactivate anti_sigma+Inelastic all')
        uiCommandList.append('/process/inactivate   xi-Inelastic all')
        uiCommandList.append('/process/inactivate anti_xi-Inelastic all')
        uiCommandList.append('/process/inactivate    xi0Inelastic all')
        uiCommandList.append('/process/inactivate anti_xi0Inelastic all')
        uiCommandList.append('/process/inactivate omega-Inelastic all')
        uiCommandList.append('/process/inactivate anti_omega-Inelastic all')
        # uiCommandList.append('/process/inactivate CHIPSNuclearCaptureAtRest')
        uiCommandList.append('/process/inactivate hFritiofCaptureAtRest all')
        uiCommandList.append('/process/inactivate hBertiniCaptureAtRest all')
        uiCommandList.append('/process/inactivate muMinusCaptureAtRest all')
        uiCommandList.append('/process/inactivate  dInelastic all')
        uiCommandList.append('/process/inactivate  anti_deuteronInelastic all')
        uiCommandList.append('/process/inactivate    tInelastic all')
        uiCommandList.append('/process/inactivate    anti_tritonInelastic all')
        uiCommandList.append('/process/inactivate    He3Inelastic all')
        uiCommandList.append('/process/inactivate    anti_He3Inelastic all')
        uiCommandList.append('/process/inactivate    alphaInelastic all')
        uiCommandList.append('/process/inactivate    anti_alphaInelastic all')
        uiCommandList.append('/process/inactivate      ExtEnergyLoss all')
        uiCommandList.append('/process/inactivate       OpAbsorption all')
        uiCommandList.append('/process/inactivate         OpRayleigh all')
        uiCommandList.append('/process/inactivate            OpMieHG all')
        uiCommandList.append('/process/inactivate         OpBoundary all')
        uiCommandList.append('/process/inactivate              OpWLS all')
        uiCommandList.append('/process/inactivate           Cerenkov all')
        uiCommandList.append('/process/inactivate      Scintillation all')

    if not useMultipleScattering:
        uiCommandList.append('/process/inactivate                msc all')
        uiCommandList.append('/process/inactivate        CoulombScat all')
        # uiCommandList.append('/process/inactivate              muMsc')
    # uiCommandList.append('/process/list')
    # if (useMultipleScattering == False or useMultipleScattering == False):
    if verbose:
        uiCommandList.append('/particle/select pi+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select pi-')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select e+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select e-')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select mu+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select mu-')
        uiCommandList.append('/particle/select kaon+')
        uiCommandList.append('/particle/process/dump')
        uiCommandList.append('/particle/select kaon-')
        uiCommandList.append('/particle/process/dump')
    g4sim.param('UICommands', uiCommandList)

    # print(uiCommandList)
    if (path is 0):
        return g4sim
    else:
        path.add_module(g4sim)
        return None
