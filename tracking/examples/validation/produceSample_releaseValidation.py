#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ROOT import Belle2

import basf2
import logging

import tracking as tr
import reconstruction as re

basf2.set_random_seed(1234)

path = basf2.create_path()

environment = Belle2.Environment.Instance()
environment.setNumberEventsOverride(10000)

# Simulate events first (e.g. sample produced by validation/validation/EvtGenSim.py)
path.add_module('RootInput', inputFileName='./EvtGenSim.root')

# Some default modules
path.add_module('Progress')

path.add_module("Gearbox")
path.add_module("Geometry")

# Do not prune the tracks (usually all but first and last hit are removed)
re.add_reconstruction(path, pruneTracks=False, reconstruct_cdst="rawFormat")

# Set a few options of modules that we need for the studies
for module in path.modules():
    if 'TrackFinderMCTruthRecoTracks' in module.name():
        module.param({"UseReassignedHits": True, 'UseNLoops': 1})
    if 'V0Finder' in module.name():
        module.param("Validation", True)

# Add MC information for V0s
path.add_module('MCV0Matcher', V0ColName='V0ValidationVertexs')

# Add a bunch of branches that we need (on top of cdst)
re.add_cdst_output(
    path,
    mc=True,
    filename='./validationSample.root',
    additionalBranches=[
        'SVDClusters',
        'PXDClusters',
        'CDCSimHits',
        'CDCSimHitsToCDCHits',
        'MCRecoTracks',
        'RecoHitInformations',
        'RecoTracksToRecoHitInformations',
        'MCRecoTracksToRecoHitInformations',
        'MCRecoTracksToMCParticles',
        'MCRecoTracksToRecoTracks',
        'RecoTracksToMCParticles',
        'RecoTracksToMCRecoTracks',
        'V0ValidationVertexs',
        'V0ValidationVertexsToMCParticles',
        'V0sToV0ValidationVertexs',
        'TracksToRecoTracks',
        'TracksToMCParticles'])

basf2.print_path(path)
basf2.process(path)

logging.basicConfig(level=logging.INFO)

print(basf2.statistics)
