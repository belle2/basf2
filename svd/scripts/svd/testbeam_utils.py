#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Geometry helpers for VXD testbeam 2017

from basf2 import *
from ROOT import Belle2
from testbeam.utils import add_vxdtf_v2


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
                         log_level=LogLevel.ERROR,
                         debug_level=1,
                         usedGeometry=useThisGeometry
                         )
        else:
            print("VXDTF1 not supported")
    daf = register_module('DAFRecoFitter')
    daf.param('initializeCDCTranslators', False)
    daf.logging.log_level = LogLevel.ERROR
    path.add_module(daf)
    track_creator = register_module('TrackCreator')
    track_creator.param('beamSpot', [0., 0., 0.])
    track_creator.param('pdgCodes', [11])
    track_creator.logging.log_level = LogLevel.ERROR
    path.add_module(track_creator)
