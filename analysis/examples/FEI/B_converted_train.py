#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Example which can be used to train the FEI

import os

import basf2 as b2
import modularAnalysis as ma

import b2biiConversion

import fei

# To properly read the Belle database the user name is set to g0db
os.environ['PGUSER'] = 'g0db'
# Get FEI default channels for Belle conversion
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels()

# Set up FEI configuration specifying the FEI prefix
configuration = fei.config.FeiConfiguration(prefix='FEI_TEST', training=True)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Create Path
path = b2.create_path()

# Apply conversion initially
if feistate.stage <= 0:
    b2biiConversion.convertBelleMdstToBelleIIMdst(
        b2.find_file(
            'analysis/mdstBelle1_exp65_charged.root',
            'validation',
            False),
        applySkim=True,
        # Actually, the KS finder should be set to True.
        # However, here it's set to False because the necessary library is only present on kekcc and not on the build server.
        enableNisKsFinder=False,
        # Set this argument to True (default value) so that the local database on KEKCC is used.
        # It's only set to False here so that this example can be run on bamboo.
        enableLocalDB=False,
        path=path)
else:
    ma.inputMdstList('Belle', [], path)

# Add FEI path to the path to be processed
path.add_path(feistate.path)

# Add RootOutput to save particles reconstructing during the training stage
path.add_module('RootOutput')

print(path)
b2.process(path)
