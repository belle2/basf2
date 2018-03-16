#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2

import reconstruction as reco
import modularAnalysis as ana

main = basf2.create_path()

main.add_module("RootInput")
main.add_module("HistoManager", histoFileName="CollectorOutput.root")
main.add_module("Gearbox")
main.add_module("Geometry")

reco.add_reconstruction(main, pruneTracks=False)

# Pre-fit with beam+vertex constraint decays for alignment
ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 2.', writeOut=True, path=main)
ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=main)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')

ana.matchMCTruth('mu+:qed', main)
ana.matchMCTruth('Z0:mumu', main)

ana.printVariableValues('Z0:mumu', ['E', 'deltaE', 'M', 'mcPDG', 'p'], path=main)
ana.printVariableValues('mu+:qed', ['E', 'deltaE', 'M', 'mcPDG', 'p', 'muonID'], path=main)

# main.add_module('MillepedeCollector', components=['BeamParameters'], tracks=[], primaryVertices=['Z0:mumu'], calibrateVertex=True)

main.add_module('RootOutput')
main.add_module("Progress")

basf2.print_path(main)
basf2.process(main)
print(basf2.statistics)
