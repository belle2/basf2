#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 Bd->[J/psi->ee] [K_S->pipi] events.
#
# Set environment variables BELLE2_VTX_BACKGROUND_DIR and
# BELLE2_VTX_UPGRADE_GT according to confluence page
# https://confluence.desy.de/display/BI/Full+simulation+effort
# recommendation.
#
# Usage: basf2 reco.py
#
# Input: None
# Output: vtx_mdst_1111540100.root
#
# Example steering file for VTX upgrade - 2020 Belle II Collaboration
#############################################################


import basf2 as b2
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from mdst import add_mdst_output

# Need to use default global tag prepended with upgrade GT
from vtx import get_upgrade_globaltag
b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())


# create path
main = b2.create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# print event numbers
main.add_module('EventInfoPrinter')

# NickName: Bd_JpsiKS_ee
# That is the example from the online book in chapter 21.4.2:
# https://software.belle2.org/development/sphinx/online_book/basf2/first_steering_file.html
main.add_module("EvtGenInput", userDECFile=b2.find_file('decfiles/dec/1111540100.dec'))

# detector simulation
add_simulation(main, useVTX=True)


# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main, useVTX=True)

# mdst output
add_mdst_output(main, filename='vtx_mdst_1111540100.root')

# cdst output (for calibration)
# add_cdst_output(main)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
