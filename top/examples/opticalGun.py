#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

# ---------------------------------------------------------------
# Example of usage of the OpticalGun module.
# It simulates two sources at the left and right side of prism,
# outside quartz, pointed towards the PMTs
#
# Contibutors: Marko Staric
#              Umberto Tamponi
# ---------------------------------------------------------------


# Create path
main = b2.create_path()


# Set number of events to generate
main.add_module('EventInfoSetter',
                expList=[1003],  # 0 for nominal phase 3, 1002 for phase II, 1003 for early phase III
                evtNumList=[10])

# Gearbox
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Optical sources
main.add_module('OpticalGun',
                maxAlpha=45.0,
                na=0.5,
                startTime=0,
                pulseWidth=10.0e-3,  # laser time Jitter, in ns
                numPhotons=10,
                diameter=10.0e-3,  # source diameter in cm
                slotID=5,  # if nonzero, local (slot) referenc frame is used, otherwise Belle II
                x=-22.6,
                y=0.0,
                z=-129.9,
                phi=0.0,
                theta=180.0,
                psi=0.0,
                angularDistribution='Gaussian')
main.add_module('OpticalGun',
                maxAlpha=45.0,
                na=0.5,
                startTime=0,
                pulseWidth=10.0e-3,
                numPhotons=10,
                diameter=10.0e-3,
                slotID=5,
                x=22.6,
                y=0.0,
                z=-129.9,
                phi=0.0,
                theta=180.0,
                psi=0.0,
                angularDistribution='Gaussian')

# Simulation
main.add_module('FullSim')

# TOP digitization
main.add_module('TOPDigitizer')

# Output
main.add_module('RootOutput',
                outputFileName='opticalGun.root')

# Show progress of processing
main.add_module('Progress')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
