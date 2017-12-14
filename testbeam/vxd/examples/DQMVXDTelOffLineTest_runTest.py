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
parser.add_argument('--svd-only', dest='svd_only', action='store_const', const=True,
                    default=False, help='Use only SVD sector maps in VXDTF track finder')
parser.add_argument('--raw-input', dest='raw_input', action='store_const', const=True, default=False,
                    help='Use SeqRootInput to load directly raw data files in .sroot format')
parser.add_argument('--unpacking', dest='unpacking', action='store_const', const=True,
                    default=False, help='Add PXD and SVD unpacking modules to the path')
parser.add_argument('--display', dest='display', action='store_const', const=True, default=False, help='Show Event Display window')
parser.add_argument(
    '--tel-input',
    dest='tel_input',
    action='store',
    default=None,
    type=str,
    help='Location of telescope .raw file to merge')
parser.add_argument(
    '--debug',
    dest='debug',
    action='store_const',
    const=True,
    default=False,
    help='Set log level to INFO to see all messages')
args = parser.parse_args()

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
if args.debug:
    set_log_level(LogLevel.INFO)


# Set up DB chain
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True)
if args.local_db is not None:
    use_local_database(Belle2.FileSystem.findFile(args.local_db), "", True)

main = create_path()

if args.raw_input:
    main.add_module('SeqRootInput')
else:
    branches = ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs']
    if not args.unpacking:
        branches = branches + ['PXDDigits', 'SVDDigits']
    main.add_module('RootInput', branchNames=branches)


histo = register_module('HistoManager')
histo.param('histoFileName', 'TB2016-OffLineDQM-VXDTel-histo.root')  # File to save histograms
main.add_module(histo)

main.add_module('Progress')

add_geometry(main, not args.magnet_off)

if args.tel_input:
    telmerger = register_module('TelDataMergerTB2016')
    # telmerger.logging.log_level = LogLevel.INFO
    param_telmerger = {
        'MergeSwitch': 1,
        'storeOutputPXDDigitsName': 'PXDDigits_2',
        'storeTELDigitsName': 'TelDigits',
        'bufferSize': 10000,
        'inputFileName': args.tel_input,
    }
    telmerger.param(param_telmerger)
    main.add_module(telmerger)

if args.unpacking:
    if not args.svd_only:
        triggerfix = main.add_module(register_module('PXDTriggerFixer'))
        triggerfix.if_false(create_path())
        main.add_module(triggerfix)
        """
        triggerfix = register_module(register_module('PXDTriggerFixer'))
        triggerfix.if_false(create_path())
        main.add_module(triggerfix)
        """
        main.add_module('PXDTriggerShifter')
        main.add_module('PXDUnpacker',
                        RemapFlag=True,
                        RemapLUT_IF_OB=Belle2.FileSystem.findFile('data/testbeam/vxd/LUT_IF_OB.csv'),
                        RemapLUT_IB_OF=Belle2.FileSystem.findFile('data/testbeam/vxd/LUT_IB_OF.csv'))

    SVDUNPACK = register_module('SVDUnpacker')
    SVDUNPACK.param('xmlMapFileName', 'testbeam/vxd/data/TB_svd_mapping.xml')
    SVDUNPACK.param('FADCTriggerNumberOffset', 1)
    SVDUNPACK.param('shutUpFTBError',  10)
    main.add_module(SVDUNPACK)

if not args.svd_only:
    if args.unpacking:
        main.add_module("PXDRawHitSorter")
    main.add_module('PXDDigitSorter')
    main.add_module('PXDClusterizer')

main.add_module('SVDDigitSorter')  # , ignoredStripsListName='data/testbeam/vxd/SVD_Masking.xml')
main.add_module('SVDClusterizer')

if args.tel_input:
    main.add_module('TelDigitSorter')
    main.add_module('TelClusterizer')

# main.add_module("VXDTelDQMOffLine", SaveOtherHistos=1)
# VXD+Tel off line DQM module
vxdTel_dqm = register_module('VXDTelDQMOffLine')
vxdTel_dqm.param('SaveOtherHistos', 1)
vxdTel_dqm.param('SwapPXD', 0)
vxdTel_dqm.param('SwapTel', 0)
# granularity of correlation plots, min = 0.002 mm, max = 1 mm, default = 1 mm
# vxdTel_dqm.param('CorrelationGranulation', 1)
main.add_module(vxdTel_dqm)

# main.add_module('RootOutput')

if args.display:
    main.add_module('Display', fullGeometry=True)

process(main)
print(statistics)
