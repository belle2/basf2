#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Geometry helpers for VXD testbeam 2017

from basf2 import *
from ROOT import Belle2


def add_geometry(
        path,
        magnet=True,
        field_override=None,
        target=None,
        geometry_xml='testbeam/vxd/2017_geometry.xml',
        excluded_components=[],
        geometry_version=1):

    additonal_components = []

    if geometry_version == 0:
        print('WARNING: overriding the setting of parameter geometry_xml! Old value: ' + geometry_xml)
        geometry_xml = 'testbeam/vxd/2017_geometry.xml'
        print('New value: ' + geometry_xml)

    if geometry_version == 1:
        print('WARNING: overriding the setting of parameter geometry_xml! Old value: ' + geometry_xml)
        geometry_xml = 'testbeam/vxd/2017_geometry_1.xml'
        print('New value: ' + geometry_xml)

    if target is not None:
        additonal_components += [target]

    if not magnet:
        excluded_components += ['MagneticField']

    if field_override is not None:
        excluded_components += ['MagneticField']
        additonal_components += ['MagneticFieldConstant']

    # Add gearbox, additonally override field value if set
    if field_override is not None:
        path.add_module('Gearbox',
                        fileName=geometry_xml,
                        override=[("/DetectorComponent[@name='MagneticFieldConstant']//Z",
                                   str(field_override),
                                   "")])
    else:
        path.add_module('Gearbox', fileName=geometry_xml)

    # Add geometry with additional/removed components
    path.add_module('Geometry', excludedComponents=excluded_components, additionalComponents=additonal_components)


def add_simulation(path, momentum=5., positrons=False, momentum_spread=0.05):
    # momentum = 6.0  # GeV/c
    # momentum_spread = 0.05  # %
    theta = 90.0  # degrees
    theta_spread = 0.005  # degrees (sigma of gaussian)
    phi = 0.0  # degrees
    phi_spread = 0.005  # degrees (sigma of gaussian)
    gun_x_position = -100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
    # gun_x_position = -40. # cm ... 40cm ... inside magnet
    beamspot_size_y = 0.3  # cm (sigma of gaussian)
    beamspot_size_z = 0.3  # cm (sigma of gaussian)
    # ParticleGun
    particlegun = register_module('ParticleGun')
    # number of primaries per event
    particlegun.param('nTracks', 1)
    # DESY electrons/positrons:
    if not positrons:
        particlegun.param('pdgCodes', [11])
    else:
        particlegun.param('pdgCodes', [-11])

    # momentum magnitude 2 GeV/c or something above or around.
    # At DESY we can have up to 6 GeV/c(+-5%) electron beam.
    # Beam divergence and spot size is adjusted similar to reality
    # See studies of Benjamin Schwenker
    particlegun.param('momentumGeneration', 'normal')
    particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
    # momentum direction must be around theta=90, phi=180
    particlegun.param('thetaGeneration', 'normal')
    particlegun.param('thetaParams', [theta, theta_spread])
    particlegun.param('phiGeneration', 'normal')
    particlegun.param('phiParams', [phi, phi_spread])
    particlegun.param('vertexGeneration', 'normal')
    particlegun.param('xVertexParams', [gun_x_position, 0.])
    particlegun.param('yVertexParams', [0., beamspot_size_y])
    particlegun.param('zVertexParams', [0., beamspot_size_z])
    particlegun.param('independentVertices', True)

    path.add_module(particlegun)

    path.add_module('FullSim', StoreAllSecondaries=True)
    path.add_module('PXDDigitizer')
    path.add_module('SVDDigitizer')


def add_reconstruction(
        path,
        magnet=True,
        momentum=5.,
        vxdtf2=False,
        mc=False,
        geometry_version=1
):

    useThisGeometry = 'TB2017newGeo'
    if geometry_version == 0:
        useThisGeometry = 'TB2017'

    path.add_module('SetupGenfitExtrapolation')
    if mc:
        path.add_module('TrackFinderMCTruthRecoTracks')
    else:
        if(vxdtf2):
            add_vxdtf_v2(path,
                         magnet_on=magnet,
                         filter_overlapping=True,
                         use_segment_network_filters=True,
                         observerType=0,
                         quality_estimator='circleFit',
                         overlap_filter='greedy',  # 'hopfield' or 'greedy'
                         log_level=LogLevel.ERROR,
                         debug_level=1,
                         usedGeometry=useThisGeometry
                         )
        else:
            add_vxdtf(
                path,
                magnet=magnet,
                momentum=momentum,
                filterOverlaps='hopfield',
                usedGeometry=useThisGeometry
            )
            path.add_module('RecoTrackCreator', trackCandidatesStoreArrayName='__TrackCands')

    daf = register_module('DAFRecoFitter')
    daf.param('initializeCDCTranslators', False)
    daf.logging.log_level = LogLevel.ERROR
    path.add_module(daf)
    track_creator = register_module('TrackCreator')
    track_creator.param('beamSpot', [0., 0., 0.])
    track_creator.param('pdgCodes', [11])
    track_creator.logging.log_level = LogLevel.ERROR
    path.add_module(track_creator)


# adds the vxdtf to the path
def add_vxdtf(path, magnet=True, momentum=5., filterOverlaps='hopfield', usedGeometry='TB2017'):
    vxdtf_module = get_vxdtf(magnet, momentum, filterOverlaps, usedGeometry=usedGeometry)
    path.add_module(vxdtf_module)

# function which returns a vxdtf module with the default settings, so that one can change parameters afterwards


def get_vxdtf(magnet=True, svd_only=True, momentum=5., filterOverlaps='hopfield', usedGeometry='TB2017newGeo'):
    if magnet:
        if not svd_only:
            # SVD and PXD sec map
            if usedGeometry == 'TB2016':
                secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
            elif usedGeometry == 'TB2017':
                secSetup = ['SecMapTB2017MagnetOnPXDSVD-moreThan1000MeV_PXDSVD']
            elif usedGeometry == 'TB2017newGeo':
                secSetup = ['SecMapTB2017MagnetOnPXDSVD_afterMarch1st-moreThan1000MeV_PXDSVD']
            else:
                print('not supported option for usedGeometry!')
                exit()
        else:
            # only SVD:
            if usedGeometry == 'TB2016':
                secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
            elif usedGeometry == 'TB2017':
                secSetup = ['SecMapTB2017MagnetOnSVD-moreThan1000MeV_SVD']
            elif usedGeometry == 'TB2017newGeo':
                secSetup = ['SecMapTB2017MagnetOnSVD_afterMarch1st-moreThan1000MeV_SVD']
            else:
                print('not supported option for usedGeometry!')
                exit()
        qiType = 'circleFit'  # circleFit
    else:
        if not svd_only:
            # To turn off magnetic field:
            # SVD and PXD sec map:
            if usedGeometry == 'TB2016':
                secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
            elif usedGeometry == 'TB2017':
                secSetup = ['SecMapTB2017MagnetOffPXDSVD-moreThan1000MeV_PXDSVD']
            elif usedGeometry == 'TB2017newGeo':
                secSetup = ['SecMapTB2017MagnetOffPXDSVD_afterMarch1st-moreThan1000MeV_PXDSVD']
            else:
                print('not supported option for usedGeometry!')
                exit()
        else:
            # only SVD
            if usedGeometry == 'TB2016':
                secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
            elif usedGeometry == 'TB2017':
                secSetup = ['SecMapTB2017MagnetOffSVD-moreThan1000MeV_SVD']
            elif usedGeometry == 'TB2017newGeo':
                secSetup = ['SecMapTB2017MagnetOffSVD_afterMarch1st-moreThan1000MeV_SVD']
            else:
                print('not supported option for usedGeometry!')
                exit()
        qiType = 'straightLine'  # straightLine

    vxdtf = register_module('VXDTF')
    vxdtf.logging.debug_level = 2
    # calcQIType:
    # Supports 'kalman', 'circleFit' or 'trackLength.
    # 'circleFit' has best performance at the moment

    # filterOverlappingTCs:
    # Supports 'hopfield', 'greedy' or 'none'.
    # 'hopfield' has best performance at the moment
    param_vxdtf = {  # normally we don't know the particleID, but in the case of the testbeam,
        # we can expect (anti-?)electrons...
        # True
        # 'artificialMomentum': 5., ## uncomment if there is no magnetic field!
        # 7
        # 'activateDistance3D': [False],
        # 'activateDistanceZ': [True],
        # 'activateAngles3D': [False],
        # 'activateAnglesXY': [True],  #### noMagnet
        # ### withMagnet
        # 'activateAnglesRZHioC': [True], #### noMagnet
        # ### withMagnet r51x
        # True

        # activateBaselineTF was 1: but this TF caused lots of problems last TB so I turned it off,
        # it should work now but better play it safe
        'activateBaselineTF': 0,
        'debugMode': 0,
        'tccMinState': [2],
        'tccMinLayer': [3],
        'reserveHitsThreshold': [0.],
        'highestAllowedLayer': [6],
        'standardPdgCode': -11,
        'artificialMomentum': 3,
        'sectorSetup': secSetup,
        'calcQIType': qiType,
        'killEventForHighOccupancyThreshold': 500,
        'highOccupancyThreshold': 111,
        'cleanOverlappingSet': False,
        'filterOverlappingTCs': filterOverlaps,
        'TESTERexpandedTestingRoutines': True,
        'qiSmear': False,
        'smearSigma': 0.000001,
        'GFTrackCandidatesColName': '__TrackCands',
        'tuneCutoffs': 0.51,
        'activateDistanceXY': [False],
        'activateDistance3D': [True],
        'activateDistanceZ': [False],
        'activateSlopeRZ': [False],
        'activateNormedDistance3D': [False],
        'activateAngles3D': [True],
        'activateAnglesXY': [False],
        'activateAnglesRZ': [False],
        'activateDeltaSlopeRZ': [False],
        'activateDistance2IP': [False],
        'activatePT': [False],
        'activateHelixParameterFit': [False],
        'activateAngles3DHioC': [True],
        'activateAnglesXYHioC': [True],
        'activateAnglesRZHioC': [False],
        'activateDeltaSlopeRZHioC': [False],
        'activateDistance2IPHioC': [False],
        'activatePTHioC': [False],
        'activateHelixParameterFitHioC': [False],
        'activateDeltaPtHioC': [False],
        'activateDeltaDistance2IPHioC': [False],
        'activateZigZagXY': [False],
        'activateZigZagRZ': [False],
        'activateDeltaPt': [False],
        'activateCircleFit': [False],
    }

    if not magnet:
        param_vxdtf['artificialMomentum'] = 5.
        param_vxdtf['activateAnglesXY'] = [True]
        param_vxdtf['activateAnglesRZHioC'] = [True]

    vxdtf.param(param_vxdtf)
    return vxdtf


def add_vxdtf_v2(path=None,
                 magnet_on=True,
                 filter_overlapping=True,
                 use_segment_network_filters=True,
                 observerType=0,
                 quality_estimator='circleFit',
                 overlap_filter='greedy',
                 log_level=LogLevel.ERROR,
                 usedGeometry='TB2017newGeo',
                 debug_level=1):
    """
    Convenience Method to setup the redesigned vxd track finding module chain.
    Reuslt is a store array containing reco tracks called 'RecoTracks'.
    :param sec_map_file: training data for segment network.
    :param path: basf2.Path
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

    spCreatorSVD = register_module('SVDSpacePointCreator')
    spCreatorSVD.logging.log_level = log_level
    spCreatorSVD.logging.debug_level = debug_level
    spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
    spCreatorSVD.param('SpacePoints', 'SpacePoints')
    modules.append(spCreatorSVD)

    # SecMap Bootstrap
    secmap_name = ""
    if magnet_on:
        if usedGeometry == 'TB2017':
            secmap_name = 'SecMap_testbeamTEST_MagnetOnSVD.root'
        elif usedGeometry == 'TB2017newGeo':
            secmap_name = 'SecMap_testbeamTEST_MagnetOnSVD_afterMarch1st.root'
        else:
            print('ERROR: no sectormap for that setting')
            exit()
    else:  # magnet off
        if usedGeometry == 'TB2017':
            secmap_name = 'SecMap_testbeamTEST_MagnetOffSVD.root'
        elif usedGeometry == 'TB2017newGeo':
            secmap_name = 'SecMap_testbeamTEST_MagnetOffSVD_afterMarch1st.root'
        else:
            print('ERROR: no sectormap for that setting')
            exit()
    secMapBootStrap = register_module('SectorMapBootstrap')
    secMapBootStrap.param('ReadSectorMap', True)
    if secmap_name:
        secMapBootStrap.param('SectorMapsInputFile', Belle2.FileSystem.findFile("data/testbeam/vxd/" + secmap_name))
    secMapBootStrap.param('WriteSectorMap', False)
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
    segNetProducer.param('addVirtualIP', False)
    segNetProducer.param('virtualIPCoorindates', [-40, 0, 0])
    segNetProducer.param('sectorMapName', 'testbeamTEST')  # lowTestRedesign')
    segNetProducer.param('observerType', observerType)
    segNetProducer.logging.log_level = log_level  # LogLevel.DEBUG
    segNetProducer.logging.debug_level = debug_level
    modules.append(segNetProducer)

    #################
    # VXDTF2 Step 2
    # TrackFinder
    #################

    # this currently prevents from being able to use PXD as it only takes one array for space points (or merge pxd and svd SP)
    cellOmat = register_module('TrackFinderVXDCellOMat')
    cellOmat.param('NetworkName', 'test2Hits')
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
    qualityEstimator.logging.log_level = log_level
    qualityEstimator.logging.debug_level = debug_level
    modules.append(qualityEstimator)

    # Properties
    vIPRemover = register_module('SPTCvirtualIPRemover')
    vIPRemover.param('maxTCLengthForVIPKeeping', 0)  # want to remove virtualIP for any track length
    vIPRemover.logging.log_level = log_level
    vIPRemover.logging.debug_level = debug_level
    modules.append(vIPRemover)

    #################
    # VXDTF2 Step 4
    # OverlapFilter
    #################

    if filter_overlapping:
        overlapNetworkProducer = register_module('SVDOverlapResolver')
        overlapNetworkProducer.logging.log_level = log_level
        overlapNetworkProducer.logging.debug_level = debug_level
        overlapNetworkProducer.param('ResolveMethod', overlap_filter.lower())
        modules.append(overlapNetworkProducer)

    #################
    # VXDTF2 Step 5
    # Converter
    #################
    momSeedRetriever = register_module('SPTCmomentumSeedRetriever')
    momSeedRetriever.logging.log_level = log_level
    momSeedRetriever.logging.debug_level = debug_level
    modules.append(momSeedRetriever)

    converter = register_module('SPTC2RTConverter')
    converter.logging.log_level = log_level
    converter.logging.debug_level = debug_level
    modules.append(converter)

    if path:
        for module in modules:
            path.add_module(module)
    else:
        return modules
