#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Example which can be used to train the FEI

# William Sutcliffe 2019

import basf2 as b2
import modularAnalysis as ma

import b2biiConversion
import ROOT
from ROOT import Belle2

import fei

# Get FEI default channels for Belle conversion
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels(convertedFromBelle=True)

# Set up FEI configuration specifying the FEI prefix
configuration = fei.config.FeiConfiguration(prefix='FEI_TEST', b2bii=True, training=True)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Create Path
path = b2.create_path()

# Apply conversion initially
if feistate.stage <= 0:
    b2biiConversion.convertBelleMdstToBelleIIMdst(None, applyHadronBJSkim=True, path=path)
else:
    b2.inputMdstList('Belle', [], path)

# Add FEI path to the path to be processed
path.add_path(feistate.path)

# Add RootOutput to save particles reconstructing during the training stage
path.add_module('RootOutput')

print(path)
b2.process(path)
