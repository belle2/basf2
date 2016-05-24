#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2016 @ DESY Simulation
# This is the default simulation scenario for VXD beam test

from basf2 import *
from ROOT import Belle2
# import glob
import ROOT
# import time
import datetime


UseTracks = 0

events = 10000  # Number of events to simulate
# For simulation:
RunNoVXD = '0'
TelRunNo = -1
StartTimee = '1461929666592085'
EndTimee = str(1461929666592085 + 300.5 * 1000000)

input_files = Belle2.Environment.Instance().getInputFilesOverride()
rootstr = '.root'
if len(input_files) > 0:
    SubNomberString = input_files[0][input_files[0].find(rootstr) + len(rootstr):]
    NomberString = int(input_files[0][input_files[0].find(rootstr) + len(rootstr) - 11:][0:6])
    RunNoVXD = str(NomberString) + SubNomberString
    print('VXD run number: {}'.format(RunNoVXD))

tOut = [0, 0]


def getTimeStamps(filenames):
    rootfiles = filenames
    chain = ROOT.TChain('tree')
    for f in sorted(rootfiles):
        print('Adding {}'.format(f))
        chain.Add(f)
    nEntries = chain.GetEntries()
    print('Entriess: {}'.format(nEntries))
    for ev_number in [0, nEntries - 1]:
        chain.GetEntry(ev_number)
        sec = chain.RawFTSWs[0].GetTTUtime(0)
        usec = int(chain.RawFTSWs[0].GetTTCtime(0) / 127.216)
        if (ev_number == 0):
            tOut[0] = sec * 1000000 + usec
            print('Time Start {}'.format(tOut[0]))
        if (ev_number == (nEntries - 1)):
            tOut[1] = sec * 1000000 + usec
            print('Time End   {}'.format(tOut[1]))
    return tOut

getTimeStamps(input_files)
StartTimee = str(tOut[0])
EndTimee = str(tOut[1])


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

# for real data parce from name of VXD:

# SummaryFileRunName = 'QualityCheckSummary_Run'+str(RunNoVXD)+'.txt'
# SummaryFileRunName = 'QualityCheckSummary.txt'
SummaryFileRunName = '/home/peter/QualityCheck2/QualityCheckSummary.txt'
# SummaryFileRunName = '/data2/depfet/TB_DESY_2016/Sources/QualityCheck2/QualityCheckSummary.txt'


# OutputHistoName = 'TB2016_QualityCheckHistos_Run'+str(RunNoVXD)+'.root'
OutputHistoName = '/home/peter/QualityCheck2/TB2016_QualityCheckHistos_Run' + str(RunNoVXD) + '.root'
# OutputHistoName = '/data2/depfet/TB_DESY_2016/Sources/QualityCheck2/TB2016_QualityCheckHistos_Run'+str(RunNoVXD)+'.root'

# PathToMask = 'testbeam/vxd/data/'
PathToMask = '/home/peter/Masking/'
# PathToMask = 'testbeam/vxd/data//'

pxd_maskBasic = PathToMask + 'PXD_MaskFiredBasic.xml'
svd_maskBasic = PathToMask + 'SVD_MaskFiredBasic.xml'
tel_maskBasic = PathToMask + 'Tel_MaskFiredBasic.xml'
pxd_mask = PathToMask + 'PXD_MaskFired_Run' + str(RunNoVXD) + '.xml'
svd_mask = PathToMask + 'SVD_MaskFired_Run' + str(RunNoVXD) + '.xml'
tel_mask = PathToMask + 'Tel_MaskFired_Run' + str(RunNoVXD) + '.xml'


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
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True)
use_local_database(Belle2.FileSystem.findFile("/home/peter/database/database.txt"), "", True)
if args.local_db is not None:
    use_local_database(Belle2.FileSystem.findFile(args.local_db), "", True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})


main = create_path()

# main.add_module(eventinfosetter)

if args.raw_input:
    main.add_module('SeqRootInput')
else:
    branches = ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs']
    if not args.unpacking:
        branches = branches + ['PXDDigits', 'SVDDigits']
    main.add_module('RootInput', branchNames=branches)

main.add_module('HistoManager', histoFileName=OutputHistoName)

main.add_module('Progress')

add_geometry(main, not args.magnet_off)

if args.tel_input:
    telmerger = register_module('TelDataMergerTB2016')
    print('Telescope Number:' + str(int(args.tel_input[-10:-4])))
    TelRunNo = int(args.tel_input[-10:-4])

    # telmerger.logging.log_level = LogLevel.INFO
    param_telmerger = {
        'MergeSwitch': 1,
        'storeOutputPXDDigitsName': 'PXDDigits_2',
        'storeTELDigitsName': 'TelDigits',
        'bufferSize': 10000,
        'inputFileName': args.tel_input,
        # 'EUDAQTriggerOffset': 1,
    }
    telmerger.param(param_telmerger)
    main.add_module(telmerger)

if args.unpacking:
    if not args.svd_only:
        # triggerfix = main.add_module(register_module('PXDTriggerFixer'))
        # triggerfix.if_false(create_path())
        # main.add_module(triggerfix)
        main.add_module('PXDTriggerShifter')
        main.add_module('PXDUnpacker',
                        RemapFlag=True,
                        RemapLUT_IF_OB=Belle2.FileSystem.findFile('data/testbeam/vxd/LUT_IF_OB.csv'),
                        RemapLUT_IB_OF=Belle2.FileSystem.findFile('data/testbeam/vxd/LUT_IB_OF.csv'))

    SVDUNPACK = register_module('SVDUnpacker')
    SVDUNPACK.param('xmlMapFileName', 'testbeam/vxd/data/TB_svd_mapping.xml')
    SVDUNPACK.param('FADCTriggerNumberOffset', 1)
    SVDUNPACK.param('shutUpFTBError', 10)
    main.add_module(SVDUNPACK)

if not args.svd_only:
    if args.unpacking:
        main.add_module("PXDRawHitSorter")
    PXDSortBasic = register_module('PXDDigitSorter')
    PXDSortBasic.param('ignoredPixelsListName', pxd_maskBasic)
    main.add_module(PXDSortBasic)
    PXDSort = register_module('PXDDigitSorter')
    PXDSort.param('ignoredPixelsListName', pxd_mask)
    main.add_module(PXDSort)
    main.add_module('PXDClusterizer')

# main.add_module('SVDDigitSorter')  # , ignoredStripsListName='data/testbeam/vxd/SVD_Masking.xml')
main.add_module('SVDDigitSorter', ignoredStripsListName=svd_maskBasic)
main.add_module('SVDDigitSorter', ignoredStripsListName=svd_mask)
main.add_module('SVDClusterizer')

if args.tel_input:
    # main.add_module('TelDigitSorter')
    TelSortBasic = register_module('TelDigitSorter')
    TelSortBasic.param('ignoredPixelsListName', tel_maskBasic)
    main.add_module(TelSortBasic)
    TelSort = register_module('TelDigitSorter')
    TelSort.param('ignoredPixelsListName', tel_mask)
    main.add_module(TelSort)
    main.add_module('TelClusterizer')

if args.gbl_collect:
    main.add_module('SetupGenfitExtrapolation', whichGeometry='TGeo')
else:
    main.add_module('SetupGenfitExtrapolation')

add_vxdtf(main, not args.magnet_off, args.svd_only, args.momentum)

if args.gbl_collect:
    # main.add_module('GBLfit', BuildBelle2Tracks=True, beamSpot=[-30, 0, 0])
    # main.add_module('MillepedeCollector', minPValue=0.0)
    main.add_module('GBLfit')
    main.add_module('MillepedeCollector', minPValue=0.0001)
else:
    main.add_module('GenFitter', FilterId='Kalman')

if args.dqm:
    # main.add_module('RawDQMCorr')
    if not args.svd_only:
        if args.unpacking:
            main.add_module("PXDRawDQM")
        main.add_module("PXDDQMCorr")
        main.add_module('PXDRawDQMCorr')
        main.add_module('PXDDQM', histgramDirectoryName='pxddqm')
    # CorrelationGranulation: granularity of correlation plots, min = 0.02 mm, max = 1 mm, default = 1 mm
    main.add_module("VXDTelDQMOffLine", SaveOtherHistos=1, SwapTel=0, CorrelationGranulation=0.5)
    if not args.gbl_collect:
        main.add_module('TrackfitDQM')

QualityCheck = register_module('QualityCheck')
QualityCheck.param('SummaryFileRunName', SummaryFileRunName)
QualityCheck.param('StartTime', StartTimee)
QualityCheck.param('EndTime', EndTimee)
QualityCheck.param('TelRunNo', TelRunNo)
QualityCheck.param('RunNo', RunNoVXD)
main.add_module(QualityCheck)

# main.add_module('RootOutput')

if args.display:
    if UseTracks:
        main.add_module('TrackBuilder')
    main.add_module('Display', fullGeometry=True)

process(main)
print(statistics)
