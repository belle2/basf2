#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Geometry helpers for VXD testbeam 2017

from basf2 import *
from ROOT import Belle2
from tracking.__init__ import *


def setup_database(local_db=None, global_tag=None, daq_db='testbeam/daq/data/database_v1/database.txt'):
    # Set up DB chain
    reset_database()
    use_database_chain()
    use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True)
    if daq_db is not None:
        use_local_database(Belle2.FileSystem.findFile(daq_db), "", True)
    if local_db is not None:
        use_local_database(Belle2.FileSystem.findFile(local_db), "", True)
    if global_tag is not None:
        use_central_database(global_tag, LogLevel.DEBUG)


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


def add_unpacking(path, svd_only=False):
    path.add_module('SVDUnpacker', xmlMapFileName='testbeam/vxd/data/2017_svd_mapping.xml')
    path.add_module('SVDDigitSorter')

    if not svd_only:
        add_pxd_unpacking(path)


def add_pxd_unpacking(path):
    pxd_unpacker = register_module('PXDUnpacker')
    pxd_unpacker.logging.log_level = LogLevel.RESULT
    pxd_unpacker.param('IgnoreDATCON', True)
    pxd_unpacker.param('IgnoreFrameCount', True)
    pxd_unpacker.param('IgnoreSorFlag', True)
    pxd_unpacker.param('RemapFlag', True)
    path.add_module(pxd_unpacker)

    path.add_module("PXDRawHitSorter")


# function for the case one only needs the clusterizers
def add_clusterizer(path, svd_only=False):
    if not svd_only:
        path.add_module('PXDClusterizer')

    path.add_module('SVDClusterizer')


def add_reconstruction(
        path,
        magnet=True,
        svd_only=False,
        mc=False,
        geometry_version=1
):
    add_clusterizer(path, svd_only)

    useThisGeometry = 'TB2017newGeo'
    if geometry_version == 0:
        useThisGeometry = 'TB2017'

    path.add_module('SetupGenfitExtrapolation')
    if mc:
        path.add_module('TrackFinderMCTruthRecoTracks')
    else:
        add_vxdtf_v2(path,
                     use_pxd=False,
                     magnet_on=magnet,
                     filter_overlapping=True,
                     use_segment_network_filters=True,
                     quality_estimator='circleFit',
                     overlap_filter='greedy',  # 'hopfield' or 'greedy'
                     usedGeometry=useThisGeometry
                     )

    # path.add_module('GenFitterVXDTB')
    daf = register_module('DAFRecoFitter')
    daf.param('initializeCDCTranslators', False)
    daf.logging.log_level = LogLevel.ERROR
    path.add_module(daf)
    track_creator = register_module('TrackCreator')
    track_creator.param('beamSpot', [0., 0., 0.])
    track_creator.param('pdgCodes', [11])
    track_creator.logging.log_level = LogLevel.ERROR
    path.add_module(track_creator)


def add_vxdtf_v2(path=None,
                 use_pxd=False,
                 magnet_on=True,
                 filter_overlapping=True,
                 use_segment_network_filters=True,
                 quality_estimator='circleFit',
                 overlap_filter='greedy',
                 usedGeometry='TB2017newGeo'):
    """
    Convenience Method to setup the redesigned vxd track finding module chain.
    Result is a store array containing reco tracks called 'RecoTracks'.
    :param path: basf2.Path
    :param magnet_on: whether magnet was turned on or not.
    :param use_pxd: if true use pxd hits. Default False.
    :param quality_estimator: which fit to use to determine track quality. Options 'circle', 'random'. Default 'circle'.
    :param filter_overlapping: if true overlapping tracks are reduced to a single track using the qualitiy indicator.
    :param use_segment_network_filters: if true use filters for segmentMap training. Default True.
    :param overlap_filter: which filter network to use. Options 'hopfield', 'greedy'. Default 'hopfield'.
    :param usedGeometry: Which geometry is to be used.
    :return:
    """

    tf2_components = []
    tf2_components += ['SVD']
    if use_pxd:
        tf2_components += ['PXD']

    # SecMap Bootstrap
    secmap_name = ""
    if magnet_on:
        if use_pxd:
            if usedGeometry == 'TB2017':
                secmap_name = 'SecMap_testbeamTEST_MagnetOnVXD.root'
            elif usedGeometry == 'TB2017newGeo':
                secmap_name = 'SecMap_testbeamTEST_MagnetOnVXD_afterMarch1st.root'
            else:
                print('ERROR: no sectormap for that setting')
                exit()
        else:
            if usedGeometry == 'TB2017':
                secmap_name = 'SecMap_testbeamTEST_MagnetOnSVD.root'
            elif usedGeometry == 'TB2017newGeo':
                secmap_name = 'SecMap_testbeamTEST_MagnetOnSVD_afterMarch1st.root'
            else:
                print('ERROR: no sectormap for that setting')
                exit()
    else:  # magnet off
        if use_pxd:
            if usedGeometry == 'TB2017':
                secmap_name = 'SecMap_testbeamTEST_MagnetOffVXD.root'
            elif usedGeometry == 'TB2017newGeo':
                secmap_name = 'SecMap_testbeamTEST_MagnetOffVXD_afterMarch1st.root'
            else:
                print('ERROR: no sectormap for that setting')
                exit()
        else:
            if usedGeometry == 'TB2017':
                secmap_name = 'SecMap_testbeamTEST_MagnetOffSVD.root'
            elif usedGeometry == 'TB2017newGeo':
                secmap_name = 'SecMap_testbeamTEST_MagnetOffSVD_afterMarch1st.root'
            else:
                print('ERROR: no sectormap for that setting')
                exit()

    add_vxd_track_finding_vxdtf2(path=path,
                                 svd_clusters="",
                                 reco_tracks="RecoTracks",
                                 components=tf2_components,
                                 suffix="",
                                 useTwoStepSelection=False,
                                 sectormap_file=Belle2.FileSystem.findFile("data/testbeam/vxd/" + secmap_name),
                                 PXDminSVDSPs=0)  # This module was not in the old function, so reproduce no action for now

    set_module_parameters(path, 'SectorMapBootstrap', SetupToRead='testbeamTEST')
    set_module_parameters(path, 'SegmentNetworkProducer', allFiltersOff=not use_segment_network_filters)
    set_module_parameters(path, 'SegmentNetworkProducer', sectorMapName='testbeamTEST')
    set_module_parameters(path, 'QualityEstimatorVXD', EstimationMethod=quality_estimator)
    if filter_overlapping:
        set_module_parameters(path, 'SVDOverlapResolver', ResolveMethod=overlap_filter.lower())
    else:
        # Would originally not add the module to the path
        # Instead use crude workaround to create new path without the module
        new_path = create_path()
        for m in path.modules():
            if m.name() != 'SVDOverlapResolver':
                new_path.add_module(m)
        # Need to use this way to propagate changes to the outside of the function
        path.__init__()
        path.add_path(new_path)
