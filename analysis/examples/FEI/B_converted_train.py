#!/usr/bin/env python3

# Example which can be used to train the FEI

# William Sutcliffe 2019

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
        enableNisKsFinder=False,
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
