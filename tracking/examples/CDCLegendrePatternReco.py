#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
############################################################################################################################

import sys
import os
from optparse import OptionParser

parser = OptionParser()
parser.add_option('--input', dest='input', default='-999',
                  help='location of input file')

(options, args) = parser.parse_args()

print options

from basf2 import *

set_log_level(LogLevel.INFO)

# necessary modumes
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
# eventinfosetter.param('evtNumList', [options.nevents])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')
# evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
#                  + '/tracking/acceptance/BtoDpi.dec')
evtgeninput.param('boost2LAB', True)
# DECFile and pdlFile have sane defaults
#
# simulation
g4sim = register_module('FullSim')
# this is need for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', True)
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

mixbkg = register_module('MixBkg')
bkg_file = [
    '/storage/5/mziegler/Belle2/MCprod_2013Summer/Coulomb_HER/CDCROFnoMC_Coulomb_HER_1ms_2x.root'
        ,
    '/storage/5/mziegler/Belle2/MCprod_2013Summer/Coulomb_LER/CDCROFnoMC_Coulomb_LER_1ms_2x.root'
        ,
    '/storage/5/mziegler/Belle2/MCprod_2013Summer/RBB_HER/CDCROFnoMC_RBB_HER_1ms_2x.root'
        ,
    '/storage/5/mziegler/Belle2/MCprod_2013Summer/RBB_LER/CDCROFnoMC_RBB_LER_1ms_2x.root'
        ,
    '/storage/5/mziegler/Belle2/MCprod_2013Summer/Touschek_HER/CDCROFnoMC_Touschek_HER_1ms_2x.root'
        ,
    '/storage/5/mziegler/Belle2/MCprod_2013Summer/Touschek_LER/CDCROFnoMC_Touschek_LER_1ms_2x.root'
        ,
    ]

# '/storage/5/mziegler/Belle2/MCprod_2013Summer/Coulomb_HER/CDCROF_Coulomb_HER_1ms_0x.root',
# '/storage/5/mziegler/Belle2/MCprod_2013Summer/Coulomb_LER/CDCROF_Coulomb_LER_1ms_0x.root',
# '/storage/5/mziegler/Belle2/MCprod_2013Summer/RBB_HER/CDCROF_RBB_HER_1ms_0x.root',
# '/storage/5/mziegler/Belle2/MCprod_2013Summer/RBB_LER/CDCROF_RBB_LER_1ms_0x.root',
# '/storage/5/mziegler/Belle2/MCprod_2013Summer/Touschek_HER/CDCROF_Touschek_HER_1ms_0x.root',
# '/storage/5/mziegler/Belle2/MCprod_2013Summer/Touschek_LER/CDCROF_Touschek_LER_1ms_0x.root'
mixbkg.param('BackgroundFiles', bkg_file)

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# ---------------------------------------------------------------
cdctracking = register_module('CDCLegendreTracking')
param_cdctracking = {  #    'Resolution StereoHits': 2.,
    'GFTrackCandidatesColName': 'TrackCands',
    'Threshold': 10,
    'InitialAxialHits': 30,
    'MaxLevel': 10,
    'FitTracks': True,
    'EarlyTrackFitting': False,
    'EarlyTrackMerge': False,
    'AppendHits': True,
    'DrawCandidates': False,
    'EnableDrawing': False,
    'StepScale': 0.75,
    'Reconstruct Curler': False,
    }
cdctracking.param(param_cdctracking)

# ---------------------------------------------------------------
# match the found track candidates with MCParticles
mcmatching = register_module('CDCMCMatching')

# select the correct collection for the matching
param_mcmatching = {'GFTrackCandidatesColName': 'TrackCands',
                    'CDCHitsColName': 'CDCHits',
                    'MCParticlesColName': 'MCParticles'}
mcmatching.param(param_mcmatching)

# ----------------------------------------------------------------
CDCNiceDrawing = register_module('CDCNiceDrawing')
CDCNiceDrawing.param('StoreDirectory', 'Legendre')
CDCNiceDrawing.param('DrawMCSignal', True)
CDCNiceDrawing.param('DrawCands', True)

# ---------------------------------------------------------------
# input
input = register_module('RootInput')
input.param('inputFileName', options.input)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'tmp/MCEvtGenData_output.root')

# ---------------------------------------------------------------
# Add all modules to the main path
main = create_path()

# add modules to paths
if options.input == '-999':
    main.add_module(eventinfosetter)
    main.add_module(eventinfoprinter)
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(evtgeninput)
    main.add_module(g4sim)
    main.add_module(mixbkg)
    main.add_module(cdcDigitizer)
else:
    main.add_module(input)
    main.add_module(gearbox)
    main.add_module(geometry)

main.add_module(cdctracking)
main.add_module(mcmatching)
main.add_module(CDCNiceDrawing)

main.add_module(output)

process(main)

# Print call statistics
print statistics

