#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from alignment import MillepedeCalibration

# Default configurations
components = []
tracks = ['CosmicRecoTracks']
particles = ['mu+:bbmu']
vertices = []
primary_vertices = ['Z0:mumu']
gearbox_file = ''
geometry_components = []
do_reconstruction = False
do_cosmics_reconstruction = False
do_analysis = False
magnet_off = False

dirty_data = False


def get_path(
        gearboxFile=None,
        geometryComponents=None,
        doReconstruction=None,
        doAnalysis=None,
        magnetOff=None,
        doCosmicsReconstruction=None):
    if gearboxFile is None:
        gearboxFile = gearbox_file
    if geometryComponents is None:
        geometryComponents = geometry_components
    if doReconstruction is None:
        doReconstruction = do_reconstruction
    if doAnalysis is None:
        doAnalysis = do_analysis
    if magnetOff is None:
        magnetOff = magnet_off
    if do_cosmics_reconstruction is None:
        doCosmicsReconstruction = do_cosmics_reconstruction

    import modularAnalysis as ana
    import reconstruction as reco

    path = create_path()
    path.add_module('Progress')

    path.add_module('Gearbox', fileName=gearboxFile) if gearboxFile else path.add_module('Gearbox')

    if geometryComponents:
        path.add_module('Geometry', components=geometryComponents, excludedComponents=['MagneticField'] if magnetOff else [])
    else:
        path.add_module('Geometry', excludedComponents=['MagneticField'] if magnetOff else [])

    if doReconstruction:
        if geometryComponents:
            reco.add_reconstruction(path, pruneTracks=False, components=geometryComponents)
        else:
            reco.add_reconstruction(path, pruneTracks=False)

    if doCosmicsReconstruction:
        import pxd
        import svd

        if not geometryComponents or 'PXD' in geometryComponents:
            pxd.add_pxd_reconstruction(path)
        if not geometryComponents or 'SVD' in geometryComponents:
            svd.add_svd_reconstruction(path)

        if geometryComponents:
            reco.add_cosmics_reconstruction(
                path,
                pruneTracks=False,
                components=geometryComponents,
                merge_tracks=True)
        else:
            reco.add_cosmics_reconstruction(
                path,
                pruneTracks=False,
                merge_tracks=True)

    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)

    if doAnalysis:
        ana.fillParticleList('mu+:bbmu', 'muonID > 0.1 and useLabFrame(p) > 0.5', True, path)
        ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useLabFrame(p) > 2.', True, path)
        ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=path)
        ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=path, constraint='ipprofile')

    return path


def setup_none():
    millepede = MillepedeCalibration()
    return millepede


def setup_default():
    millepede = MillepedeCalibration(components,
                                     tracks=tracks,
                                     particles=particles,
                                     vertices=vertices,
                                     primary_vertices=primary_vertices,
                                     path=get_path())

    if dirty_data:
        millepede.set_command('chiscut', None)
        millepede.set_command('outlierdownweighting', None)
        millepede.set_command('dwfractioncut', None)

    millepede.algo.invertSign()

    return millepede


def setup_Global():
    millepede = setup_default()
    millepede.set_command('Fortranfiles')
    millepede.set_command('constraints.txt')
    return millepede


def setup_BeamVertex():
    millepede = setup_default()
    millepede.set_components(['BeamParameters'])
    return millepede


def setup_VXDHalfShells():
    millepede = setup_default()
    millepede.set_components(['VXDAlignment'])
    millepede.fixSVDPat()
    for layer in range(1, 7):
        for ladder in range(1, 17):
            # Fix also all ladders
            millepede.fixVXDid(layer, ladder, 0)
            for sensor in range(1, 6):
                # Fix all sensors
                millepede.fixVXDid(layer, ladder, sensor)
                pass
    return millepede


def setup_VXDFullHierarchy():
    millepede = setup_default()
    millepede.set_components(['VXDAlignment'])
    millepede.set_command('Fortranfiles')
    millepede.set_command('constraints.txt')
    return millepede


def setup_VXDSensors():
    millepede = setup_default()
    millepede.set_components(['VXDAlignment'])
    millepede.fixSVDPat()
    millepede.fixSVDMat()
    millepede.fixPXDYing()
    millepede.fixPXDYang()
    for layer in range(1, 7):
        for ladder in range(1, 17):
            # Fix also all ladders
            millepede.fixVXDid(layer, ladder, 0)
            for sensor in range(1, 6):
                # Fix all sensors
                # millepede.fixVXDid(layer, ladder, sensor)
                pass
    millepede.fixVXDid(1, 1, 1)
    return millepede


def setup_CDCLayers():
    millepede = setup_default()
    millepede.set_components(['CDCAlignment', 'CDCLayerAlignment'])
    millepede.fixCDCLayerX(49)
    millepede.fixCDCLayerY(49)
    millepede.fixCDCLayerRot(49)
    millepede.fixCDCLayerX(55)
    millepede.fixCDCLayerY(55)
    millepede.fixCDCLayerRot(55)
    return millepede


def setup_CDCTimeWalks():
    millepede = setup_default()
    millepede.set_components(['CDCTimeWalks'])
    millepede.fixCDCTimeWalk(1)
    return millepede


def setup_BKLMAlignment():
    millepede = setup_default()
    millepede.set_components(['BKLMAlignment'])
    millepede.fixGlobalParam(Belle2.BKLMAlignment.getGlobalUniqueID(), Belle2.BKLMElementID(0, 3, 15).getID(), 1)
    millepede.fixGlobalParam(Belle2.BKLMAlignment.getGlobalUniqueID(), Belle2.BKLMElementID(0, 3, 15).getID(), 2)

    for sector in range(1, 9):
        for layer in range(1, 16):
            for forward in [0, 1]:
                millepede.fixBKLMModule(forward, sector, layer, [1, 2, 3, 4, 5, 6])
    return millepede


def setup_EKLMAlignment():
    millepede = setup_default()
    millepede.set_components(['EKLMAlignment'])
    millepede.fixEKLMModule(1, 12, 2, 1, 1, [1, 2, 6])
    return millepede


def setup_CDCLayers_GCR_Karim():
    geometry = '/alignment/examples/GCR_Summer2017.xml'
    recotracks = ['RecoTracks']
    global do_cosmics_reconstruction
    do_cosmics_reconstruction = True

    # Inherit settings
    millepede = setup_CDCLayers()
    millepede.algo.invertSign(False)

    millepede.set_param(recotracks, 'tracks')
    millepede.set_param(geometry, 'fileName', 'Gearbox')
    millepede.set_param(1.e-20, 'minPValue')

    return millepede
