#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ROOT import Belle2

import basf2
import logging

import tracking as tr
import reconstruction as re

basf2.set_random_seed(1337)

path = basf2.create_path()

environment = Belle2.Environment.Instance()
environment.setNumberEventsOverride(10000)

# simulate events first (e.g. sample produced by validation/validation/EvtGenSim.py)
path.add_module('RootInput', inputFileName='./EvtGenSim.root')

# Progress module
path.add_module('Progress')

path.add_module("Gearbox")
path.add_module("Geometry", useDB=True)

re.add_reconstruction(path, pruneTracks=False, reconstruct_cdst="rawFormat")

for module in path.modules():
    if 'TrackFinderMCTruthRecoTracks' in module.name():
        module.param({"UseReassignedHits": True, 'UseNLoops': 1})
    if 'V0Finder' in module.name():
        module.param("Validation", True)

path.add_module('MCV0Matcher', V0ColName='V0ValidationVertexs')

re.add_cdst_output(
    path,
    mc=True,
    filename='/nfs/dust/belle2/user/kurzsimo/validation_study/testCDST2.root',
    additionalBranches=[
        'SVDClusters',
        'PXDClusters',
        'MCRecoTracks',
        'CDCSimHits',
        'CDCSimHitsToCDCHits',
        'RecoHitInformations',
        'RecoTracksToRecoHitInformations',
        'MCRecoTracksToRecoHitInformations',
        'V0ValidationVertexs',
        'V0ValidationVertexsToMCParticles',
        'V0sToV0ValidationVertexs'])

basf2.print_path(path)
basf2.process(path)

logging.basicConfig(level=logging.INFO)

print(basf2.statistics)
