#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------------------------------------
#           TSim-ecl example code.
# ------------------------------------------------------------------------------------------------------
#       In order to test Tsim-ecl code, you need a root file which has ECLHit table.(after Gsim)
#       ex)
#       commend > basf2 TrgEcl_pgun.py [Name of output root file]
# ------------------------------------------------------------------------------------------------------
import os
import basf2 as b2
import random

################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 2:

    f_out_root = argvs[1]
print()
# print 'f_in_root  = %s' % f_in_root
print('f_out_root = %s\n' % f_out_root)

################
# f_in_root1 = f_in_root+ "1.root";
# f_in_root2 = f_in_root+ "2.root";

# suppress messages and warnings during processing:
# level: LogLevel.DEBUG/INFO/WARNING/ERROR/FATALls
b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

b2.set_random_seed(0)
basf2datadir = os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None), 'data')

# one event
# eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

# Register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfoprinter = b2.register_module('EventInfoPrinter')

gearbox = b2.register_module('Gearbox')

# input
# rootinput1 = register_module('RootInput')
# rootinput1.param('inputFileName', f_in_root)

# Geometry builder
geometry = b2.register_module('Geometry')

# Simulation
g4sim = b2.register_module('FullSim')

# register module for TRGCDC
# evtmetagen        = register_module('EventInfoSetter')
evtmetainfo = b2.register_module('Progress')
# paramloader       = register_module('Gearbox')
# geobuilder        = register_module('Geometry')
# particlegun       = register_module('ParticleGun')
# evtgeninput       = register_module('EvtGenInput')
# kkgeninput        = register_module('KKGenInput')
mcparticleprinter = b2.register_module('PrintMCParticles')
# g4sim             = register_module('FullSim')
# bkgmixer          = register_module('BeamBkgMixer')
cdcdigitizer = b2.register_module('CDCDigitizer')
cdctrg = b2.register_module("TRGCDC")
# rootOut           = register_module('RootOutput')
# rootIn            = register_module('RootInput')


# one event
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

intseed = random.randint(1, 10000000)

pGun = b2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [11],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [1.0, 2.0],
    'thetaGeneration': 'uniform',
    'thetaParams': [50., 130.],
    'phiGeneration': 'uniform',
    #    'phiParams': [0, 360],
    'phiParams': [-45, 45],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}

pGun.param(param_pGun)

mcparticleprinter.param('maxLevel', -1)
# need to check
# g4sim.param('UICommandsAtIdle',['/control/execute ' +
#                           os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None),"trg/cdc/examples/physics.mac")])

# ...CDCDigitizer...
# set digitizer to no smearing
param_cdcdigi = {'Fraction': 1,
                 'Resolution1': 0.,
                 'Resolution2': 0.,
                 'Threshold': -10.0}
cdcdigitizer.param(param_cdcdigi)
cdcdigitizer.param('AddInWirePropagationDelay', True)
cdcdigitizer.param('AddTimeOfFlight', True)

# ...CDC Trigger...
# ---General settings---
# cdctrg.param('ConfigFile', os.path.join(basf2datadir,"trg/cdc/TRGCDCConfig_0_20101111_1051.dat"))
cdctrg.param('ConfigFile', os.path.join(basf2datadir, "trg/cdc/TRGCDCConfig_0_20101111_1051_2013beamtest.dat"))
# cdctrg.param('DebugLevel', 1)
cdctrg.param('CurlBackStop', 0)
cdctrg.param('SimulationMode', 1)        # 1:fast, 2:firm, 3:fast and firm
cdctrg.param('FastSimulationMode', 0)
# cdctrg.param('SimulationMode',0x11)
# cdctrg.param('TRGCDCRootFile',1)
# cdctrg.param('RootTRGCDCFile', 'TRGCDC.root')
# ---TSF settings---
# TSLUT (latest version @ 2014.07)
cdctrg.param('InnerTSLUTFile', os.path.join(basf2datadir, "trg/cdc/innerLUT_v2.2.coe"))
cdctrg.param('OuterTSLUTFile', os.path.join(basf2datadir, "trg/cdc/outerLUT_v2.2.coe"))
# cdctrg.param('TSFLogicLUT', 1)
# cdctrg.param('TSFRootFile',1)
# ---2D finder settings---
cdctrg.param('2DFinderPerfect', 0)
cdctrg.param('HoughFinderMeshX', 160)
cdctrg.param('HoughFinderMeshY', 26)
# ---3D finder settings---
cdctrg.param('Hough3DRootFile', 1)
# 0: perfect finder, 1: Hough3DFinder, 2: (Default) GeoFinder, 3: VHDL GeoFinder
# 0: (Default) Logic TSF, 1: LUT TSF
# cdctrg.param('Finder3DMode',0)
# ---3D fitter settings---
cdctrg.param('Fitter3DRootFile', 1)
cdctrg.param('RootFitter3DFile', 'Fitter3D.root')
cdctrg.param('Fitter3DLRLUT', 0)


# TRGECL
trgeclfam = b2.register_module("TRGECLFAM")
trgecl = b2.register_module("TRGECL")
trgeclMC = b2.register_module("TRGECLMCMatching")
# addParam("FAMFitMethod", _famMethod, "TRGECLFAM fit method", _famMethod);
trgeclfam.param('FAMFitMethod', 1)

grltrg = b2.register_module('TRGGRL')
grltrg.param('DebugLevel', 0)
grltrg.param('ConfigFile',
             os.path.join(os.environ['BELLE2_LOCAL_DIR'],
                          "trg/gdl/data/ftd/0.01/ftd_0.01"))

# output
rootoutput = b2.register_module('RootOutput')
rootoutput.param('outputFileName', f_out_root)

# import random
# progress = register_module('Progress')


# Set parameters

# Create paths
main = b2.create_path()

# main.add_module(rootinput1)
# main.add_module(rootinput2)
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(evtmetainfo)
# main.add_module(paramloader)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(mcparticleprinter)
main.add_module(g4sim)

main.add_module(cdcdigitizer)
main.add_module(cdctrg)
main.add_module(trgeclfam)
main.add_module(trgecl)
main.add_module(grltrg)
# main.add_module(trgeclMC)

main.add_module(rootoutput)


# main
b2.process(main)
###
###
###
print(b2.statistics)
# ===<END>
