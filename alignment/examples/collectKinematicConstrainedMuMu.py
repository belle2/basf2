#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2

import simulation as sim
import reconstruction as reco
import modularAnalysis as ana
import beamparameters as beam

main = basf2.create_path()

main.add_module("RootInput")
# main.add_module("EventInfoSetter")
main.add_module("Gearbox")
main.add_module("Geometry")
main.add_module("HistoManager", histoFileName="CollectorOutput.root")
beam.add_beamparameters(main, 'Y4S', 10.5796, vertex=[0., 0., 0.])

# main.add_module('PairGen', pdgCode=13)
sim.add_simulation(main)
reco.add_reconstruction(main, pruneTracks=False)

# Pre-fit with beam+vertex constraint decays for alignment
ana.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 2.', writeOut=True, path=main)
ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', 'InvM > 10.5296 and InvM < 10.6296', writeOut=True, path=main)

ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, silence_warning=True)

ana.matchMCTruth('mu+:qed', main)
ana.matchMCTruth('Z0:mumu', main)

ana.printVariableValues('Z0:mumu', ['E', 'deltaE', 'M', 'InvM', 'mcPDG', 'p', 'Ecms'], path=main)
ana.printVariableValues('mu+:qed', ['E', 'deltaE', 'M', 'mcPDG', 'p', 'muonID'], path=main)

main.add_module('VariablesToEventBasedTree',
                particleList='Z0:mumu',
                variables=['InvM', 'Ecms',
                           'daughter(0, p)', 'daughter(1, p)'],
                event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])
main.add_module(
    'MillepedeCollector',
    components=['beamparameters'],
    primaryTwoBodyDecays=['Z0:mumu'], tracks=[],
    calibrateVertex=True,
    useGblTree=False)

main.add_module('RootOutput')
main.add_module("Progress")

basf2.print_path(main)
basf2.process(main)
print(basf2.statistics)
