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

##############################################################################

# select if partial or full test
full = False

# select which subdetectors to test
pxd = True
svd = True
cdc = True

# select fitter: 0 = Kalman, 1 = Daf
FilterId = 0

##############################################################################

import ROOT
import sys
import shutil
from ROOT import std
from basf2 import *

ROOT.gROOT.ProcessLine('gROOT.SetBatch()')

# generate filename
print 'use the following detectors: '
rootFileName = '../genfit_'
if pxd:
    rootFileName += 'pxd'
    print 'pxd'
if svd:
    rootFileName += 'svd'
    print 'svd'
if cdc:
    rootFileName += 'cdc'
    print 'cdc'
if FilterId == 1:
    rootFileName += '_Daf'
    print 'fit with Daf'
else:
    print 'fit with Kalman'
if full:
    rootFileName += '_FULL'
    nOfEvents = 200000  # should take about 12 hrs
    print 'do FULL test'
else:
    nOfEvents = 1000
    print 'do partial test'

logFileName = rootFileName + '.log'

sys.stdout = open(logFileName, 'w')

# ============ begin of magnetic field change script part 1
magneticField = '0afterSVD'
if cdc:
    magneticField = '0afterCDC'

currentBasf2BaseDir = os.environ['BELLE2_LOCAL_DIR']
currentWorkingDir = os.getcwd()

os.chdir(currentBasf2BaseDir + '/data/geometry')
shutil.copyfile('Belle2.xml', 'Belle2.xml.backup')
belle2xmlFile = open('Belle2.xml', 'r')
belle2xml = belle2xmlFile.read()
belle2xmlFile.close()
# fist deactiave any active mangetic field
belle2xml = \
    belle2xml.replace(r'<xi:include href="../geometry/MagneticFieldConstant.xml"/>'
                      , '')
belle2xml = \
    belle2xml.replace(r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRSVD.xml"/>'
                      , '')
belle2xml = \
    belle2xml.replace(r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRCDC.xml"/>'
                      , '')
belle2xml = \
    belle2xml.replace(r'<xi:include href="../geometry/MagneticField2d.xml"/>',
                      '')
belle2xml = \
    belle2xml.replace(r'<xi:include href="../geometry/MagneticField3d.xml"/>',
                      '')
# then replace it with the one you want
if magneticField == 'normal':
    belle2xml = belle2xml.replace(r'</Detector>',
                                  r'<xi:include href="../geometry/MagneticFieldConstant.xml"/></Detector>'
                                  )
if magneticField == '0afterSVD':
    belle2xml = belle2xml.replace(r'</Detector>',
                                  r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRSVD.xml"/></Detector>'
                                  )
if magneticField == '0afterCDC':
    belle2xml = belle2xml.replace(r'</Detector>',
                                  r'<xi:include href="../geometry/MagneticFieldConstant4LimitedRCDC.xml"/></Detector>'
                                  )

belle2xmlFile = open('Belle2.xml', 'w')
belle2xmlFile.write(belle2xml)
belle2xmlFile.close()
os.chdir(currentWorkingDir)
# =============== end of magnetic field change script part 1

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# set fixed random seed to make results comparable between different revisions
set_random_seed(1)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'uniformPt')
if full:
    particlegun.param('momentumParams', [0.2, 3.0])
else:
    particlegun.param('momentumParams', [0.9, 0.9])
particlegun.param('thetaGeneration', 'uniformCosinus')
if full:
    particlegun.param('thetaParams', [17, 150])
else:
    particlegun.param('thetaParams', [100, 100])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
print_params(particlegun)

# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
componentList = ['MagneticField', 'BeamPipe']
if pxd:
    componentList.append('PXD')
if svd:
    componentList.append('SVD')
if cdc:
    componentList.append('CDC')

geometry.param('Components', componentList)

# Run simulation
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)

# ---------------------------------------------------------------
# digitizer
if cdc:
    cdcDigitizer = register_module('CDCDigitizer')
    # use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
    param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
    cdcDigitizer.param(param_cdcdigi)

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UsePXDHits', pxd)
mctrackfinder.param('UseSVDHits', svd)
mctrackfinder.param('UseCDCHits', cdc)

# ---------------------------------------------------------------
# fitting
fitter = register_module('GenFitter')
param_fitter = {
    'StoreFailedTracks': True,
    'FilterId': FilterId,
    'NIterations': 1,
    'ProbCut': 0.001,
    }

fitter.param(param_fitter)

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
if cdc:
    main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(fitter)
main.add_module(trackfitchecker)

# Process events
process(main)

# =============== begin of magnetic field change script part 2
# undo all changes to the Belle.xml
os.chdir(currentBasf2BaseDir + '/data/geometry')
shutil.move('Belle2.xml.backup', 'Belle2.xml')
os.chdir(currentWorkingDir)
# =============== end of magnetic field change script part 2

# Print call statistics
print statistics

