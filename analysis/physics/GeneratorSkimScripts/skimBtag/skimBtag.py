#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# B-tag skim of double generic events
#
# This is a script that generates double generic BBbar
# events and performs a B-tag skim. Surviving events
# proceed to simulation and reconstruction.
#
# The survival rate of the events of interest is about
# ~20%. One needs to generate ~5k events in order to
# get an output sample of ~1k events of interest
#
# Survival rate: ~20%
# Generated no. of events: ~5 000 (10k)
# No. of events of interest: ~1 000 (1k)
#
# Contributors: Matic Lubej (May 2015)
#
######################################################

# analysis_main is the default path created in the modularAnalysis.py
from basf2 import *
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from ROOT import Belle2
import glob

import os
import sys

# Import B-tag decay modes from analysis/physics/BtagGenSkim/
sys.path.insert(0, os.environ["BELLE2_LOCAL_DIR"] + "/analysis/physics/BtagGenSkim")
from imp_D_decayRec import *
from imp_cc_decayRec import *
from imp_BC_decayRec import *
from imp_BN_decayRec import *

# Set parameters: 0 - include only most common modes, 1 - include all written modes
if len(sys.argv) != 5:
    sys.exit('Must provide enough arguments: [# of events] [output path] [output name] [0/1]')

nEvents = int(sys.argv[1])
outputDir = sys.argv[2]
outputName = sys.argv[3]
recLong = sys.argv[4]

# Add "/" to the end of the output path
if not outputDir.endswith('/'):
    outputDir = outputDir + '/'

# ---------------------------------------------------
# GENERATION
# ---------------------------------------------------

# Generate Y(4S) events. The Function is defined in analysis/scripts/modularAnalysis.py
generateY4S(nEvents)

# Load geometry
loadGearbox()

# Create directory if it doesn't exist
if not os.path.exists(outputDir):
    os.makedirs(outputDir)

# ---------------------------------------------------
# BTAG MC DECAY CHAIN RECONSTRUCTION ON GENERATOR LEVEL
# ---------------------------------------------------

# Create particle maps for FSP particles
pions = ('pi+:all', '')
kaons = ('K+:all', '')
gammas = ('gamma:all', '')
electrons = ('e-:all', '')
muons = ('mu+:all', '')

# Fill particle lists
fillParticleListsFromMC([pions, kaons, gammas, electrons, muons])

# Reconstruct pi0 and K_S0
reconstructDecay('pi0:all -> gamma:all gamma:all', '0.10 < M < 0.15')
reconstructDecay('K_S0:all -> pi+:all pi-:all', '0.45 < M < 0.55')

# Reconstruct charm
reconstructD0()
reconstructDP()
reconstructDSP()
reconstructDST0()
reconstructDSTP()
reconstructDSSTP()

# Reconstruct ccbar
reconstructJpsi()
reconstructPsi2S()
reconstructChi_cy()

# Reconstruct B mesons
q = int(recLong)
if q != 0 and q != 1:
    sys.exit('Must provide argument 0 or 1')

reconstructBPD0YPS0(q)
reconstructBPD0YPS1(q)
reconstructBPDST0YPS0(q)
reconstructBPDST0YPS1(q)
reconstructBPccbarYPS1(q)
reconstructBPmisc(q)

reconstructB0DMYPS0(q)
reconstructB0DMYPS1(q)
reconstructB0DSTMYPS0(q)
reconstructB0DSTMYPS1(q)
reconstructB0ccbarY0S1(q)
reconstructB0misc(q)

# merge individual B+ lists
copyLists('B+:antiD0Y+', ['B+:antiD0Y+S0', 'B+:antiD0Y+S1'])
copyLists('B+:antiD*0Y+', ['B+:antiD*0Y+S0', 'B+:antiD*0Y+S1'])
copyLists('B+:rest', ['B+:ccbarYPS1', 'B+:misc'])

copyLists('B+:all', ['B+:antiD0Y+', 'B+:antiD*0Y+', 'B+:rest'])

# merge individual B0 lists
copyLists('B0:D-Y+', ['B0:D-Y+S0', 'B0:D-Y+S1'])
copyLists('B0:D*-Y+', ['B0:D*-Y+S0', 'B0:D*-Y+S1'])
copyLists('B0:rest', ['B0:ccbarY0S1', 'B0:misc'])

copyLists('B0:all', ['B0:D-Y+', 'B0:D*-Y+', 'B0:rest'])

# ---------------------------------------------------
# SKIM
# ---------------------------------------------------

# Create empty path
empty_path = create_path()

# Register skim module, passed events stay on original path, other events jump to the empty path and are discarded
skim = register_module('SkimFilter')
skim.param('particleLists', ['B+:all', 'B0:all'])
analysis_main.add_module(skim)

# Jump to empty path if event of interest doesn't exist
skim.if_false(empty_path)

# ---------------------------------------------------
# SIMULATION AND RECONSTRUCTION
# ---------------------------------------------------

# BKG files for running at KEKCC
bkgFiles = glob.glob('/sw/belle2/bkg/*.root')

# Simulation
add_simulation(analysis_main, None, bkgFiles)

# Reconstruction
add_reconstruction(analysis_main)

# ---------------------------------------------------
# SAVE TO OUTPUT
# ---------------------------------------------------

# Dump in MDST format
add_mdst_output(analysis_main, True, outputDir + outputName)

# Show progress of processing
progress = register_module('ProgressBar')
analysis_main.add_module(progress)

# Process all modules added to the analysis_main path
process(analysis_main)

# Print out the summary
print statistics
