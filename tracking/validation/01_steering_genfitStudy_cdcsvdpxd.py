#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file which shows all usage options for the particle gun module
# in the generators package. The generated particles from the particle gun are
# then fed through a full Geant4 simulation and the output is stored in a root
# file.
#
# The different options for the particle gun are explained below.
# Uncomment/comment different lines to get the wanted settings
#
# Example steering file - 2012 Belle II Collaboration
##############################################################################

import ROOT
import sys
import shutil
from ROOT import std
from basf2 import *

full = False
# full = True

ROOT.gROOT.ProcessLine('gROOT.SetBatch()')

# argument parsing
options = ['-full']
opts = set(options)


def getArgsToNextOpt(stringlist, index):
    id = index + 1
    args = []
    while id < len(stringlist):
        if stringlist[id] in options:
            return args[0]
        args.append(stringlist[id])
        id += 1
    return args[0]


for iarg in range(len(sys.argv)):
    arg = sys.argv[iarg]
    if arg == '-full':  # do full test
        full = True

if full:
    nOfEvents = 200000  # should take about 12 hrs
    print 'doing FULL test'
else:
    nOfEvents = 1000
    print 'doing partial test'

rootFileName = '../genfit_cdcsvdpxd'
if full:
    rootFileName += 'FULL'
logFileName = rootFileName + '.log'

sys.stdout = open(logFileName, 'w')

##============ begin of magnetic field change script part 1
##magneticField = "normal"
##magneticField = "0afterSVD"
# magneticField = "0afterCDC"
#
#
# currentBasf2BaseDir = os.environ["BELLE2_LOCAL_DIR"]
# currentWorkingDir = os.getcwd()
#
# os.chdir(currentBasf2BaseDir + "/data/geometry")
# shutil.copyfile("Belle2.xml", "Belle2.xml.backup")
# belle2xmlFile = open("Belle2.xml", "r");
# belle2xml = belle2xmlFile.read()
# belle2xmlFile.close()
##fist deactiave any active mangetic field
# belle2xml = belle2xml.replace(r'<xi:include href="../geometry/MagneticFieldConstant.xml"/>', "")
# belle2xml = belle2xml.replace(r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRSVD.xml"/>', "")
# belle2xml = belle2xml.replace(r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRCDC.xml"/>', "")
# belle2xml = belle2xml.replace(r'<xi:include href="../geometry/MagneticField2d.xml"/>', "")
# belle2xml = belle2xml.replace(r'<xi:include href="../geometry/MagneticField3d.xml"/>', "")
##then replace it with the one you want
# if magneticField == "normal":
# ....belle2xml = belle2xml.replace( r'</Detector>' , r'<xi:include href="../geometry/MagneticFieldConstant.xml"/></Detector>')
# if magneticField == "0afterSVD":
# ....belle2xml = belle2xml.replace( r'</Detector>' , r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRSVD.xml"/></Detector>')
# if magneticField == "0afterCDC":
# ....belle2xml = belle2xml.replace( r'</Detector>' , r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRCDC.xml"/></Detector>')
#
# belle2xmlFile = open("Belle2.xml", "w");
# belle2xmlFile.write(belle2xml)
# belle2xmlFile.close()
# os.chdir(currentWorkingDir)
##=============== end of magnetic field change script part 1

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# set fixed random seed to make results comparable between different revisions
set_random_seed(1)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')

# ============================================================================
# Setting the list of particle codes (PDG codes) for the generated particles
# the codes are given in an array, if only one code is used, the brackets
# should be kept: particlegun.param('pdgCodes',[11])
# there is no limit on how many codes can be given the particle gun will select
# randomly amongst the PDGcodes using a uniform distribution if nTracks>0,
# otherwise there will be one particle for each code in the list default is
# [-11, 11]
particlegun.param('pdgCodes', [211, -211])

# ============================================================================
# Setting the number of tracks to be generated per event:
# this number can be any int>=0
# default is 1
particlegun.param('nTracks', 1)

# a value o 0 means that a track should be created for each entry in the
# pdgCodes list, e.g. the following two lines would create two electrons and
# one pion per event.
# particlegun.param('pdgCodes', [11,11,211])
# particlegun.param('nTracks', 0)

# ============================================================================
# Varying the number of tracks per event can be achieved by setting varyNTacks
# to True. If so, the number of tracks will be randomized using possion
# distribution around the value of nTracks. Only valid if nTracks>0
# default is False
particlegun.param('varyNTracks', False)

# ============================================================================
# Setting the parameters for the random generation
# of particles momenta:
# Five different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - uniformPt: uniform distribution of transverse momentum between min and max
# - normal:    normal distribution around mean with width of sigma
# - normalPt:  normal distribution of transverse momentum around mean with
#              width of sigma

# The default is a uniform momentum distribution between 0.05 and 3 GeV
# particlegun.param('momentumGeneration', 'fixed')
# particlegun.param('momentumParams', [3])

# we could also generate a fixed momentum of 1 GeV
# particlegun.param('momentumGeneration', "fixed")
# particlegun.param('momentumParams', [1.0])

# or we could generate a normal distributed transverse momentum around 1 GeV
# with a width of 0.5 GeV
particlegun.param('momentumGeneration', 'uniformPt')
if full:
    particlegun.param('momentumParams', [0.2, 3.0])
else:
    particlegun.param('momentumParams', [0.9, 0.9])

# ============================================================================
# Setting the parameters for the random generation
# of the particle polar angle:
# Four different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - uniformCosinus: uniform distribution of cos(theta) between min and max
# - normal:    normal distribution around mean with width of sigma
#
# The default is a uniform theta distribution between 17 and 150 degree
particlegun.param('thetaGeneration', 'uniformCosinus')
if full:
    particlegun.param('thetaParams', [17, 150])
else:
    particlegun.param('thetaParams', [100, 100])

# or we could create a normal distributed theta angle around 90 degrees with a
# width of 5 degrees
# particlegun.param('thetaGeneration', "normal")
# particlegun.param('thetaParams', [90,5])

# or we could create a theta angle between 17 and 150 degree where the
# cos(theta) distribution is flat
# particlegun.param('thetaGeneration', "normal")
# particlegun.param('thetaParams', [90,5])

# ============================================================================
# Setting the parameters for the random generation
# of the particle azimuth angle:
# Three different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - normal:    normal distribution around mean with width of sigma
#
# The default is a uniform theta distribution between 0 and 360 degree
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# or we could create a normal distributed phi angle around 90 degrees with a
# width of 5 degrees
# particlegun.param('phiGeneration', "normal")
# particlegun.param('phiParams', [90,5])

# ============================================================================
# Setting the parameters for random generation of the event vertex
# Three different distributions can be used:
# - fixed:     always use the exact same value
# - uniform:   uniform distribution between min and max
# - normal:    normal distribution around mean with width of sigma
#
# The default is a normal distribution of the vertex
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])

# ============================================================================
# Setting independent vertices for each particle
# The default is to create one event vertex for all particles per event. By
# setting independentVertices to True, a new vertex will be created for each
# particle
# default is False
particlegun.param('independentVertices', False)

# ============================================================================
# Print the parameters of the particle gun
print_params(particlegun)

# ============================================================================
# Now lets create the necessary modules to perform a simulation

# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])
# Run simulation
simulation = register_module('FullSim')

# ---------------------------------------------------------------
# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# ---------------------------------------------------------------
# digitizer
# cdcBackground = register_module('CDCSimpleBackground')
# cdcBackground.param('BGLevelHits', 0.0)
# cdcBackground.param('BGLevelClusters', 0.0)

# ---------------------------------------------------------------
# Trasan
# trasan = register_module('Trasan')
# trasan.param('DebugLevel', 0)
# trasan.param('GFTrackCandidatesColName', 'GFTrackCands_Trasan')
#

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UseCDCHits', True)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UsePXDHits', True)
# mctrackfinder.param('GFTrackCandidatesColName', 'GFTrackCands_MCTrack')
# mctrackfinder.param('WhichParticles', 0)

# ---------------------------------------------------------------
# match the found track candidates with MCParticles
mcmatching = register_module('CDCMCMatching')

# select the correct collection for the matching
# param_mcmatching = {'GFTrackCandidatesColName': 'GFTrackCands_MCTrack'}
# mcmatching.param(param_mcmatching)

# ---------------------------------------------------------------
# fitting
cdcfitting = register_module('GenFitter')
# set correct collection name as input and custom collection names as output
# select DAF instead of Kalman as Filter
# set the pdg hypothesis to the simulated one, if you want to fit with different pdg hypothesises, set 'allPDG' to true
param_cdcfitting = {  # 'GFTrackCandidatesColName': 'GFTrackCands_MCTrack',
                      # 'GFTracksColName': 'GFTracks_MCTrack',
                      # 'GFTracksToMCParticlesColName': 'GFTracksToMCParticles_MCTrack',
    'TracksColName': 'Tracks_MCTrack',
    'StoreFailedTracks': True,
    'mcTracks': 1,
    'pdg': 211,
    'allPDG': 0,
    'FilterId': 0,
    'NIterations': 1,
    'ProbCut': 0.001,
    }

cdcfitting.param(param_cdcfitting)

# ---------------------------------------------------------------
# Setting the option for all non particle gun modules:
# want to process 100 MC events
evtmetagen.param({'EvtNumList': [nOfEvents], 'RunList': [1]})

trackfitchecker = register_module('TrackFitChecker')
# trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the statistical tests will only show up at info or debug level
# trackfitchecker.param('testSi', True)
trackfitchecker.param('robustTests', True)
trackfitchecker.param('writeToTextFile', True)
trackfitchecker.param('writeToRootFile', True)
trackfitchecker.param('inspectTracks', False)
trackfitchecker.param('truthAvailable', True)
trackfitchecker.param('outputFileName', rootFileName)

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(mcmatching)
main.add_module(cdcfitting)
main.add_module(trackfitchecker)

# Process events
process(main)

##=============== begin of magnetic field change script part 2
##undo all changes to the Belle.xml
# os.chdir(currentBasf2BaseDir + "/data/geometry")
# shutil.move("Belle2.xml.backup", "Belle2.xml")
# os.chdir(currentWorkingDir)
##=============== end of magnetic field change script part 2

# Print call statistics
print statistics
