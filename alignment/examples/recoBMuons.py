#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

main = basf2.create_path()

main.add_module("RootInput")
main.add_module("HistoManager", histoFileName="CollectorOutput.root")
main.add_module('Gearbox')
main.add_module('Geometry')

# IMPORTANT: do not prune tracks!
reco.add_reconstruction(main, pruneTracks=False)

# Select single muons for aligment...
ana.fillParticleList('mu+:bbmu', 'muonID > 0.1 and useLabFrame(p) > 0.5', True, main)

"""
main.add_module('MillepedeCollector', components=dbobjects, tracks=[], particles=['mu+:bbmu'], vertices=[], primaryVertices=[])
main.add_module("RootOutput", branchNames=['EventMetaData'])
"""
main.add_module("RootOutput")

main.add_module("Progress")

basf2.process(main)
print(basf2.statistics)
