#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>../mdst-jpsiks.root</output>
  <contact>Francesco Tenchini; francesco.tenchini@unimelb.edu.au</contact>
</header>
"""
######################################################
#
# Generate B0 -> J/psi K_S0; J/psi -> l+l- to get
# lepton samples for performance validation.
#
# Contributor: Francesco Tenchini
# October 2016
#
######################################################


# analysis_main is the default path created in the modularAnalysis.py
from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from ROOT import Belle2
import glob

set_random_seed(123)

# ---------------------------------------------------
# GENERATION
# ---------------------------------------------------

# set the BeamParameters for running at the Y(4S)
beamparameters = add_beamparameters(analysis_main, "Y4S")

# generateY4S function is defined in analysis/scripts/modularAnalysis.py
generateY4S(2000, Belle2.FileSystem.findFile('/analysis/validation/Bd_JpsiKS.dec'))

# Load geometry
loadGearbox()

# ---------------------------------------------------
# SIMULATION AND RECONSTRUCTION
# ---------------------------------------------------

# BKG files for running at KEKCC
bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')

# Simulation with BKG and reconstruction
add_simulation(analysis_main, bkgfiles=bg)
add_reconstruction(analysis_main)

# ---------------------------------------------------
# SAVE TO OUTPUT
# ---------------------------------------------------

# Dump in MDST format
add_mdst_output(analysis_main, True, '../mdst-jpsiks.root')

# Process all modules added to the analysis_main path
process(analysis_main)

# Print out the summary
print(statistics)
