#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#
# 2012/10/11 : param_cdcdigi, Threshold added
# 2013/11/05 : Updated for release-00-03-00
# 2014/02/12 : Updated for build-2014-01-19 //JB
# 2015/01/28 : Updated for build-2015-01-03 //JB
# 2015/02/02 : Added KKGen, Background, RootInput/RootOutput //JB
# 2015/05/18 : Fixed background file issue. //JB
# 2015/06/09 : Updated for release-00-05-00 //JB
# 2015/07/12 : Updated for 2D trgcdc update //JB
# 2016/04/08 : Added NeuroTrigger. Updated for head //JB

import basf2 as b2
import glob
import os

# ...suppress messages and warnings during processing...
b2.set_log_level(b2.LogLevel.ERROR)
# 0 means using different seed every time.
b2.set_random_seed(1)
basf2datadir = os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None), 'data')


##########################################################
# Register modules
evtmetagen = b2.register_module('EventInfoSetter')
evtmetainfo = b2.register_module('Progress')
paramloader = b2.register_module('Gearbox')
geobuilder = b2.register_module('Geometry')
particlegun = b2.register_module('ParticleGun')
evtgeninput = b2.register_module('EvtGenInput')
kkgeninput = b2.register_module('KKGenInput')
mcparticleprinter = b2.register_module('PrintMCParticles')
g4sim = b2.register_module('FullSim')
bkgmixer = b2.register_module('BeamBkgMixer')
cdcdigitizer = b2.register_module('CDCDigitizer')
cdctrg = b2.register_module("TRGCDC")
rootOut = b2.register_module('RootOutput')
rootIn = b2.register_module('RootInput')
neuro = b2.register_module('NeuroTrigger')


##########################################################
# Module settings

# ...Global settings...
simulatedComponents = ['MagneticField', 'CDC', 'PXD', 'SVD', 'BeamPipe'
                       ]
# To speed things up but not precise.
# simulatedComponents = ['MagneticFieldConstant4LimitedRCDC', 'CDC',
# ]

# ...EventInfoSetter...
# Set number of events and runs
evtmetagen.param({'evtNumList': [1000], 'runList': [1]})

# ...Geometry...
# Set what dectectors to simulate.
geobuilder.param('components', simulatedComponents)
# Set verbose of geometry simulation
# geobuilder.log_level = LogLevel.INFO

# ...ParticleGun...
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 5)
particlegun.param('momentumGeneration', 'inversePt')
# particlegun.param('momentumGeneration', 'uniformPt')
# particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.2, 8.0])
# particlegun.param('momentumParams', [0.9, 0.9])
# particlegun.param('thetaGeneration', 'uniform')
# particlegun.param('thetaParams', [35, 127])
# particlegun.param('thetaParams', [90, 90])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
# particlegun.param('vertexGeneration', 'fixed')
# particlegun.param('vertexGeneration', 'normal')
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [0, 0])
particlegun.param('yVertexParams', [0, 0])
particlegun.param('zVertexParams', [-20.0, 20.0])
# particlegun.param('zVertexParams', [0, 0])

# ...EvtGenInput...
# evtgeninput.param('userDECFile', 'USER.DEC')

# ...KKGenInput...
# You need to copy mu.input.dat to the current directory, that is
# found in "data/generators/kkmc".
kkdir = os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None), 'generators')
kkgeninput.param('tauinputFile', kkdir + 'kkmc/data/mu.input.dat')
kkgeninput.param('KKdefaultFile', kkdir + 'kkmc/data/KK2f_defaults.dat')
kkgeninput.param('taudecaytableFile', '')

# ...PrintMCParticles...
mcparticleprinter.param('maxLevel', -1)

# ...FullSim...
# Turn off physics processes
# "physics.mac" is located at "trg/examples/".
# g4sim.param('UICommandsAtIdle',['/control/execute physics.mac'])
# or below line can be used when trgcdc.py is not in trg/examples directory //JB
# g4sim.param('UICommandsAtIdle', ['/control/execute ' +
#                            os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None), "trg/cdc/examples/physics.mac")])

# ...BeamBkgMixer...
# Mix background (From beamBkgMixer.py)
dir = '/sw/belle2/bkg/'  # change the directory name if you don't run on KEKCC
# bkg_files = [
#     dir + 'Coulomb_HER_100us.root',
#     dir + 'Coulomb_LER_100us.root',
#     dir + 'RBB_HER_100us.root',
#     dir + 'RBB_LER_100us.root',
#     dir + 'Touschek_HER_100us.root',
#     dir + 'Touschek_LER_100us.root',
# ]
bkg_files = glob.glob(dir + '/*.root')
bkgScaleFactor = 1
bkgmixer.param('backgroundFiles', bkg_files)
bkgmixer.param('components', simulatedComponents)
bkgmixer.param('scaleFactors', [
    ('Coulomb_LER', bkgScaleFactor),
    ('Coulomb_HER', bkgScaleFactor),
    ('RBB_LER', bkgScaleFactor),
    ('RBB_HER', bkgScaleFactor),
    ('Touschek_LER', bkgScaleFactor),
    ('Touschek_HER', bkgScaleFactor)
])

# ...CDCDigitizer...
# set digitizer to no smearing
# param_cdcdigi = {'Fraction': 1,
#                  'Resolution1': 0.,
#                  'Resolution2': 0.,
#                  'Threshold': -10.0}
# cdcdigitizer.param(param_cdcdigi)
# cdcdigitizer.param('AddInWirePropagationDelay', True)
# cdcdigitizer.param('AddTimeOfFlight', True)
# cdcdigitizer.param('UseSimpleDigitization', True)

# ...CDC Trigger...
# ---General settings---
cdctrg.param('ConfigFile', os.path.join(basf2datadir, "trg/cdc/TRGCDCConfig_0_20101111.dat"))
# cdctrg.param('DebugLevel', 1)
cdctrg.param('CurlBackStop', 1)
cdctrg.param('SimulationMode', 1)  # 1:fast, 2:firm, 3:fast and firm
cdctrg.param('FastSimulationMode', 0)
# cdctrg.param('SimulationMode',0x11)
# cdctrg.param('TRGCDCRootFile',1)
# cdctrg.param('RootTRGCDCFile', 'TRGCDC.root')
# ---TSF settings---
# TSLUT (latest version @ 2014.07)
cdctrg.param('InnerTSLUTFile', os.path.join(basf2datadir, "trg/cdc/innerLUT_v3.0.coe"))
cdctrg.param('OuterTSLUTFile', os.path.join(basf2datadir, "trg/cdc/outerLUT_v3.0.coe"))
# 0: Logic TSF, 1: (Default) LUT TSF
# cdctrg.param('TSFLogicLUT', 0)
# cdctrg.param('TSFRootFile',1)
# ---2D finder settings---
cdctrg.param('HoughFinderMappingFileMinus', os.path.join(basf2datadir, "trg/cdc/HoughMappingMinus20160223.dat"))
cdctrg.param('HoughFinderMappingFilePlus', os.path.join(basf2datadir, "trg/cdc/HoughMappingPlus20160223.dat"))
# cdctrg.param('2DFinderPerfect', 1)
# cdctrg.param('HoughFinderPeakMin',4)
# cdctrg.param('HoughFinderDoit', 2)
# ---3D finder settings---
# cdctrg.param('Hough3DRootFile',1)
# 0: perfect finder, 1: Hough3DFinder, 2: (Default) GeoFinder, 3: VHDL GeoFinder
# cdctrg.param('Finder3DMode',2)
# ---3D fitter settings---
# cdctrg.param('Fitter3DRootFile',1)
# cdctrg.param('RootFitter3DFile', 'Fitter3D.root')
# cdctrg.param('Fitter3DsMcLR', 0)
# cdctrg.param('Fitter3Ds2DFit', 1)
# cdctrg.param('Fitter3Ds2DFitDrift', 0)
# cdctrg.param('Fitter3DsXtSimple', 0)
# cdctrg.param('Fitter3DsLRLUT', 0)

# ...NeuroTrigger...
# define parameters
neuro.param('filename', os.path.join(basf2datadir, "trg/cdc/Neuro20160309Nonlin.root"))
# output warnings, info and some debug output for neurotrigger module
# neuro.logging.log_level = b2.LogLevel.DEBUG
# neuro.logging.debug_level = 80
# b2.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)

# ...RootOutput...
rootOut.param('outputFileName', 'basf2.root')

# ...RootInput...
rootIn.param('inputFileName', 'basf2.root')
# rootIn.param('skipNEvents', 0);


##########################################################
# Path settings.

# For full simulation.
fullMain = b2.create_path()
# Add modules to paths
fullMain.add_module(evtmetagen)
fullMain.add_module(evtmetainfo)
fullMain.add_module(paramloader)
fullMain.add_module(geobuilder)
fullMain.add_module(particlegun)
# fullMain.add_module(evtgeninput)
# fullMain.add_module(kkgeninput)
fullMain.add_module(mcparticleprinter)
fullMain.add_module(g4sim)
fullMain.add_module(bkgmixer)
fullMain.add_module(cdcdigitizer)
fullMain.add_module(cdctrg)
fullMain.add_module(neuro)

# For only generator+G4Sim and save file. (To save time)
g4SimMain = b2.create_path()
# Add modules to paths
g4SimMain.add_module(evtmetagen)
g4SimMain.add_module(evtmetainfo)
g4SimMain.add_module(paramloader)
g4SimMain.add_module(geobuilder)
g4SimMain.add_module(particlegun)
# g4SimMain.add_module(evtgeninput)
# g4SimMain.add_module(kkgeninput)
g4SimMain.add_module(mcparticleprinter)
g4SimMain.add_module(g4sim)
g4SimMain.add_module(bkgmixer)
g4SimMain.add_module(cdcdigitizer)
g4SimMain.add_module(rootOut)

# For TSIM with generator+G4Sim saved file. (To save time)
savedG4SimMain = b2.create_path()
# Add modules to paths
savedG4SimMain.add_module(rootIn)
savedG4SimMain.add_module(evtmetainfo)
savedG4SimMain.add_module(paramloader)
savedG4SimMain.add_module(geobuilder)
savedG4SimMain.add_module(cdctrg)

# For only generator and save file.
generatorMain = b2.create_path()
# Add modules to paths
generatorMain.add_module(evtmetagen)
generatorMain.add_module(evtmetainfo)
generatorMain.add_module(paramloader)
generatorMain.add_module(geobuilder)
generatorMain.add_module(particlegun)
# generatorMain.add_module(evtgeninput)
# generatorMain.add_module(kkgeninput)
generatorMain.add_module(mcparticleprinter)
generatorMain.add_module(g4sim)
generatorMain.add_module(rootOut)

# For TSIM with generator saved file.
savedGeneratorMain = b2.create_path()
# Add modules to paths
savedGeneratorMain.add_module(rootIn)
savedGeneratorMain.add_module(evtmetainfo)
savedGeneratorMain.add_module(paramloader)
savedGeneratorMain.add_module(geobuilder)
savedGeneratorMain.add_module(bkgmixer)
savedGeneratorMain.add_module(cdcdigitizer)
savedGeneratorMain.add_module(cdctrg)

##########################################################
# Process events
# Full simulation.
b2.process(fullMain)
# Only generator+G4Sim and save file. (To save time)
# process(g4SimMain)
# For TSIM with generator+G4Sim saved file. (To save time)
# process(savedG4SimMain)
# For only generator and save file.
# process(generatorMain)
# For TSIM with generator saved file.
# process(savedGeneratorMain)

# Print call statistics
print(b2.statistics)
