#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# Charmless semileptonic skim of double generic events
#
# This is a script that generates double generic BBbar
# events and performs a charmless semileptonic skim.
# Surviving events proceed to simulation and reconstruction.
#
# The survival rate of the events of interest is about
# ~1%. One needs to generate ~100k events in order to
# get an output sample of ~1k events of interest
#
# Survival rate: ~1%
# Generated no. of events: ~100 000 (100k)
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

# Set parameters
if len(sys.argv) != 4:
    sys.exit('Must provide enough arguments: [# of events] [output path] [output name]')

nEvents = int(sys.argv[1])
outputDir = sys.argv[2]
outputName = sys.argv[3]

# Add "/" to the end of the output path
if not outputDir.endswith('/'):
    outputDir = outputDir + '/'

# ---------------------------------------------------
# GENERATION
# ---------------------------------------------------

# Generate Y(4S) events. The Function is defined in analysis/scripts/modularAnalysis.py
generateY4S(nEvents)

# Load geometry
# loadGearbox()

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
cutAndCopyList('B+:charmless', 'B+:all', '-0.5 < hasCharmedDaughter(-1) < 0.5 and -0.5 < hasCharmoniumDaughter < 0.5')
cutAndCopyList('B0:charmless', 'B0:all', '-0.5 < hasCharmedDaughter(-1) < 0.5 and -0.5 < hasCharmoniumDaughter < 0.5')

# Select semileptonic (only one leptonic first daughter - hasNDaughtersWithPDG(PDG,N,Sign))
cutAndCopyList(
    'B+:charmlessSL',
    'B+:charmless',
    'hasNDaughtersWithPDG(11,1,1) > 0.5 or hasNDaughtersWithPDG(13,1,1) > 0.5 or hasNDaughtersWithPDG(15,1,1) > 0.5')
cutAndCopyList(
    'B0:charmlessSL',
    'B0:charmless',
    'hasNDaughtersWithPDG(11,1,1) > 0.5 or hasNDaughtersWithPDG(13,1,1) > 0.5 or hasNDaughtersWithPDG(15,1,1) > 0.5')

# Create empty path
empty_path = create_path()

# Register skim module, passed events stay on original path, other events jump to the empty path and are discarded
skim = register_module('SkimFilter')
skim.param('particleLists', ['B+:charmlessSL', 'B0:charmlessSL'])
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
