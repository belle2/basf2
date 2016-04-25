import os
from basf2 import *
from subprocess import call
import datetime
from ROOT import Belle2

# ## setup of the most important parts for the VXDTF ###
fieldOn = False  # Turn field on or off (changes geometry components and digi/clust params)
filterOverlaps = 'hopfield'

# Random number seed
seed = 1  # 1, 5, 6
set_random_seed(seed)

# Log level
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# Local database access
reset_database()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
databasefile = Belle2.FileSystem.findFile("data/framework/database.txt")
use_local_database(databasefile, os.path.dirname(databasefile), True)

# Gearbox
gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName',
              'testbeam/vxd/FullVXDTB2016.xml')

# Geometry
geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes the example much faster
if fieldOn:
    # SVD and PXD sec map
    # secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD:
    secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
    # To turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])
    # SVD and PXD sec map:
    # secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD
    secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

# Unpacker
SVDUNPACK = register_module('SVDUnpacker')
SVDUNPACK.param('xmlMapFileName', 'testbeam/vxd/data/TB_svd_mapping.xml')
SVDUNPACK.param('FADCTriggerNumberOffset', 1)
SVDUNPACK.param('shutUpFTBError', 10)

PXDUNPACK = register_module('PXDUnpacker')
PXDUNPACK.param('RemapFlag', True)
PXDUNPACK.param('RemapLUT_IB_OF', os.environ.get('BELLE2_LOCAL_DIR') + '/testbeam/vxd/data/LUT_IB_OF.csv')
PXDUNPACK.param('RemapLUT_IF_OB', os.environ.get('BELLE2_LOCAL_DIR') + '/testbeam/vxd/data/LUT_IF_OB.csv')

# Digitizer
SVDDIGISORTER = register_module('SVDDigitSorter')

PXDDIGISORTER = register_module('PXDRawHitSorter')

# Clusterizer
SVDCLUST = register_module('SVDClusterizer')
if fieldOn:
    SVDCLUST.param('TanLorentz_holes', 0.)  # 0.052
    SVDCLUST.param('TanLorentz_electrons', 0.)
else:
    SVDCLUST.param('TanLorentz_holes', 0.)  # value scaled from 0.08 for 1.5T to 0.975T
    SVDCLUST.param('TanLorentz_electrons', 0.)

PXDCLUST = register_module('PXDClusterizer')
# PXDCLUST.param('NoiseSN', 0.)
# PXDCLUST.param('SeedSN', 0.)
# PXDCLUST.param('ClusterSN', 0.)

# VXDTF
vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.ERROR
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
    'GFTrackCandidatesColName': 'offline_caTracks',
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
vxdtf.param(param_vxdtf)

# Track Fitter
trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.FATAL
trackfitter.param('GFTrackCandidatesColName', 'offline_caTracks')
trackfitter.param('GFTracksColName', 'offline_gfTracks')
trackfitter.param('FilterId', 'Kalman')
# trackfitter.param('StoreFailedTracks', True)
trackfitter.param('StoreFailedTracks', False)
# trackfitter.param('FilterId', 'simpleKalman')
trackfitter.param('UseClusters', True)

# Track Builder
trackbuilder = register_module('TrackBuilder')
trackbuilder.logging.log_level = LogLevel.FATAL
trackbuilder.param('GFTrackCandidatesColName', 'offline_caTracks')
trackbuilder.param('GFTracksColName', 'offline_gfTracks')


# Genfit Extrapolator
setupGenfit = register_module('SetupGenfitExtrapolation')


##
# ROI FINDING
##

roiprod = register_module('PXDDataReduction')
roiprod.logging.log_level = LogLevel.ERROR
param_roiprod = {
    'gfTrackListName': 'offline_gfTracks',
    'PXDInterceptListName': 'offline_PXDIntercepts',
    'ROIListName': 'offline_ROIs',
    'sigmaSystU': 0.02,
    'sigmaSystV': 0.02,
    'numSigmaTotU': 10,
    'numSigmaTotV': 10,
    'maxWidthU': 0.5,
    'maxWidthV': 0.5,
}
roiprod.param(param_roiprod)

# Dummy RoIs Generator
# roiprod_debug = register_module('ROIGenerator')
# roiprod_debug.param('ROIListName','debugROIs')
# roiprod_debug.param('MinU',0)
# roiprod_debug.param('MaxU',479)
# roiprod_debug.param('MinV',0)
# roiprod_debug.param('MaxV',191)
# roiprod_debug.param('TrigDivider',5)
# roiprod_debug.param('Sensor',2)


# Input and Output

# Input module
input = register_module('SeqRootInput')
# input = register_module ('RootInput' )

# Output module
output = register_module('RootOutput')

# Progress
progress = register_module('Progress')


# Create paths
main = create_path()
emptypath = create_path()

# Add modules to paths

main.add_module(input)

# Geometry etc.
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)

# Unpack SVD
main.add_module(SVDUNPACK)
main.add_module(SVDDIGISORTER)
main.add_module(SVDCLUST)

# Unpack PXD
main.add_module(register_module('PXDTriggerShifter'))
main.add_module(PXDUNPACK)
main.add_module(PXDDIGISORTER)
main.add_module(PXDCLUST)

# Track finding and fitting
main.add_module(vxdtf)
main.add_module(trackfitter)
main.add_module(trackbuilder)

# ROI finding
main.add_module(roiprod)  # standard ROI production

main.add_module(output)
main.add_module(progress)


# Run it

# set_nprocesses(2)
process(main)

# Statistics
