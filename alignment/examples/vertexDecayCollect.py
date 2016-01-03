#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

set_log_level(LogLevel.INFO)

reset_database()
use_local_database()

main = create_path()

main.add_module("RootInput", excludeBranchNames=['GF2Tracks', 'TrackCandsToGF2Tracks', 'GF2TracksToMCParticles'])
main.add_module('Gearbox')
main.add_module('Geometry')

# vertices = ['J/psi:real', 'K_S0:real', 'Lambda0:real']
primary_vertices = ['Z0:mumu']

main.add_module('UpdateParticleTrackCand', motherListNames=primary_vertices, removeBKLM=True, removeCDC=True)

main.add_module('GBLfit', UseClusters=False, addDummyVertexPoint=True, externalIterations=0, StoreFailedTracks=True)
main.add_module('MillepedeCollector', tracks=[], primaryVertices=primary_vertices)
main.add_module('GBLdiagnostics')
main.add_module("RootOutput", branchNames=['EventMetaData'])

process(main)
print(statistics)
