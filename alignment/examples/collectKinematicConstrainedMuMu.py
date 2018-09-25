#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2

import simulation as sim
import reconstruction as reco
import modularAnalysis as ana
import beamparameters as beam

main = basf2.create_path()

basf2.use_central_database('development')

main.add_module("EventInfoSetter")
main.add_module("Gearbox")
main.add_module("Geometry")
main.add_module("HistoManager", histoFileName="CollectorOutput.root")
beam.add_beamparameters(main, 'Y4S', None)

main.add_module('PairGen', pdgCode=13)
sim.add_simulation(main)
reco.add_reconstruction(main, pruneTracks=False)

# Pre-fit with beam+vertex constraint decays for alignment
ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 2.', writeOut=True, path=main)
ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=main)

ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')
ana.fitVertex(list_name='Z0:mumu', conf_level=0.0, fitter='kfitter', fit_type='fourC', daughtersUpdate=True, path=main)

ana.matchMCTruth('mu+:qed', main)
ana.matchMCTruth('Z0:mumu', main)

ana.printVariableValues('Z0:mumu', ['E', 'deltaE', 'M', 'mcPDG', 'p'], path=main)
ana.printVariableValues('mu+:qed', ['E', 'deltaE', 'M', 'mcPDG', 'p', 'muonID'], path=main)

main.add_module(
    'MillepedeCollector',
    components=['BeamParameters'],
    primaryTwoBodyDecays=['Z0:mumu'],
    calibrateVertex=True,
    useGblTree=False)

main.add_module('RootOutput')
main.add_module("Progress")

basf2.print_path(main)
basf2.process(main)
print(basf2.statistics)
