#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2016 @ DESY Simulation
# This is the default simulation scenario for VXD beam test

from basf2 import *
from ROOT import Belle2


def add_geometry(path, magnet=True):
    path.add_module('Gearbox', fileName='testbeam/vxd/FullVXDTB2016.xml')

    if magnet:
        path.add_module('Geometry')
    else:
        path.add_module('Geometry', excludedComponents=['MagneticField'])


def add_simulation(path, momentum=6., positrons=False):
    # momentum = 6.0  # GeV/c
    momentum_spread = 0.05  # %
    theta = 90.0  # degrees
    theta_spread = 0.005  # degrees (sigma of gaussian)
    phi = 0.0  # degrees
    phi_spread = 0.005  # degrees (sigma of gaussian)
    gun_x_position = -100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
    # gun_x_position = 40. # cm ... 40cm ... inside magnet
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
    # Beam divergence divergence and spot size is adjusted similar to reality
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
    path.add_module('TelDigitizer')


def add_unpackers(path, svd_only=False):
    if not svd_only:
        path.add_module('PXDUnpacker')
    path.add_module('SVDUnpacker', xmlMapFileName='testbeam/vxd/data/TB_svd_mapping.xml')


def add_vxdtf(path, magnet=True, svd_only=False, momentum=6., filterOverlaps='hopfield'):
    if magnet:
        if not svd_only:
            # SVD and PXD sec map
            secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
        else:
            # only SVD:
            secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
        qiType = 'circleFit'  # circleFit
    else:
        if not svd_only:
            # To turn off magnetic field:
            # SVD and PXD sec map:
            secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
        else:
            # only SVD
            secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
        qiType = 'straightLine'  # straightLine

    vxdtf = register_module('VXDTF')
    vxdtf.logging.log_level = LogLevel.INFO
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
        'activateBaselineTF': 1,
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
        'GFTrackCandidatesColName': 'TrackCands',
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
        param_vxdtf['artificialMomentum'] = momentum
        param_vxdtf['activateAnglesXY'] = [True]
        param_vxdtf['activateAnglesRZHioC'] = [True]

    vxdtf.param(param_vxdtf)
    path.add_module(vxdtf)


import argparse
parser = argparse.ArgumentParser(description="Reconstruction for DESY VXD Testbeam 2016")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--magnet-off',
    dest='magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='Turn off magnetic field')
parser.add_argument('--momentum', dest='momentum', action='store', default=6., type=float,
                    help='Nominal momentum of particles (if magnet is off). Default = 6 GeV/c')
parser.add_argument('--svd-only', dest='svd_only', action='store_const', const=True,
                    default=False, help='Use only SVD sector maps in VXDTF track finder')
parser.add_argument('--raw-input', dest='raw_input', action='store_const', const=True, default=False,
                    help='Use SeqRootInput to load directly raw data files in .sroot format')
parser.add_argument('--unpacking', dest='unpacking', action='store_const', const=True,
                    default=False, help='Add PXD and SVD unpacking modules to the path')
parser.add_argument('--dqm', dest='dqm', action='store_const', const=True, default=False, help='Produce DQM plots')
parser.add_argument('--display', dest='display', action='store_const', const=True, default=False, help='Show Event Display window')
parser.add_argument('--gbl-collect', dest='gbl_collect', action='store_const', const=True,
                    default=False, help='Use GBLfit for track fitting and collect calibration data')
args = parser.parse_args()


# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)

# Set up DB chain
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
if args.local_db is not None:
    use_local_database(Belle2.FileSystem.findFile(args.local_db))

main = create_path()

if args.raw_input:
    main.add_module('SeqRootInput')
else:
    main.add_module('RootInput', branchNames=['EventMetaData', 'RawFTSWs', 'RawSVDs'])


if args.dqm:
    main.add_module('HistoManager')

main.add_module('Progress')

add_geometry(main, not args.magnet_off)

if args.unpacking:
    add_unpackers(main, args.svd_only)

if not args.svd_only:
    main.add_module('PXDClusterizer')

main.add_module('SVDDigitSorter')  # , ignoredStripsListName='data/testbeam/vxd/SVD_Masking.xml')
main.add_module('SVDClusterizer')
# main.add_module('TelClusterizer')

if args.gbl_collect:
    main.add_module('SetupGenfitExtrapolation', whichGeometry='TGeo')
else:
    main.add_module('SetupGenfitExtrapolation')

add_vxdtf(main, not args.magnet_off, args.svd_only, args.momentum)

if args.gbl_collect:
    main.add_module('GBLfit', BuildBelle2Tracks=True, beamSpot=[-30, 0, 0])
    main.add_module('MillepedeCollector', minPValue=0.001)
else:
    main.add_module('GenFitter', FilterId='Kalman')

if args.dqm:
    main.add_module('PXDDQM')
    main.add_module('SVDDQM3')

    if not args.gbl_collect:
        main.add_module('TrackfitDQM')

main.add_module('RootOutput')

if args.display:
    main.add_module('Display', fullGeometry=True)

process(main)
print(statistics)
