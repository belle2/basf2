#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Charmless hadronic skim of double generic events
#
# This is a script that generates double generic BBbar
# events and performs a charmless hadronic skim.
# Surviving events proceed to simulation and reconstruction.
#
# The survival rate of the events of interest is about
# ~5.6%. One needs to generate 18k events in order to
# get an output sample of ~1k events of interest
#
# Survival rate: ~5%
# Generated no. of events: ~20 000 (18k)
# No. of events of interest: ~1 000 (1k)
#
# Contributors: Matic Lubej (May 2015)
#
######################################################

# analysis_main is the default path created in the modularAnalysis.py
from basf2 import *
from modularAnalysis import *
from generators import add_evtgen_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from ROOT import Belle2
import glob

import os
import sys

# Set parameters
if len(sys.argv) != 5:
    sys.exit('Must provide arguments: [# of events] [output path] [output name] [sim & rec ?: 1/0]')

nEvents = int(sys.argv[1])
outputDir = sys.argv[2]
outputName = sys.argv[3]
simrec = int(sys.argv[4])

# Add "/" to the end of the output path
if not outputDir.endswith('/'):
    outputDir = outputDir + '/'

# ---------------------------------------------------
# GENERATION
# ---------------------------------------------------

# Generate Y(4S) events.
setupEventInfo(nEvents)
add_evtgen_generator(analysis_main, 'signal')

# Load geometry
loadGearbox()

# Create directory if it doesn't exist
if not os.path.exists(outputDir):
    os.makedirs(outputDir)

# Create particle maps
bcharged = ('B+:all', '')
bneutral = ('B0:all', '')

# Fill particle lists, create daughters of particles and set mother-daughter relations (argument "True")
fillParticleListsFromMC([bcharged, bneutral], True)

# Check mother-daughter relations
matchMCTruth('B+:all')
matchMCTruth('B0:all')

# ---------------------------------------------------
# SKIM
# ---------------------------------------------------

# Select charmless (discard b -> c and b-> c anti-c transitions, upper vertex b -> anti-c allowed)
cutAndCopyList('B+:charmless', 'B+:all', 'hasCharmedDaughter(-1) == 0 and hasCharmoniumDaughter == 0')
cutAndCopyList('B0:charmless', 'B0:all', 'hasCharmedDaughter(-1) == 0 and hasCharmoniumDaughter == 0')

# Select non-leptonic events
cutAndCopyList(
    'B+:charmlessH',
    'B+:charmless',
    'countDaughters(abs(mcPDG) == 11) == 0 and countDaughters(abs(mcPDG) == 13) == 0 and countDaughters(abs(mcPDG) == 15) == 0')
cutAndCopyList(
    'B0:charmlessH',
    'B0:charmless',
    'countDaughters(abs(mcPDG) == 11) == 0 and countDaughters(abs(mcPDG) == 13) == 0 and countDaughters(abs(mcPDG) == 15) == 0')

# Create empty path
empty_path = create_path()

# Register skim module, passed events stay on original path, other events jump to the empty path and are discarded
skim = register_module('SkimFilter')
skim.param('particleLists', ['B+:charmlessH', 'B0:charmlessH'])
analysis_main.add_module(skim)

# Jump to empty path if event of interest doesn't exist
skim.if_false(empty_path)

# ---------------------------------------------------
# SIMULATION AND RECONSTRUCTION
# ---------------------------------------------------

# BKG files for running at KEKCC
bkgFiles = glob.glob('/sw/belle2/bkg/*.root')

# Simulation with BKG and reconstruction
if(simrec == 1):
    add_simulation(analysis_main, None, bkgFiles)
    add_reconstruction(analysis_main)
elif(simrec != 0):
    sys.exit('Last argument should be 0 or 1!')

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
print(statistics)
