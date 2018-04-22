#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from generators import add_evtgen_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2


myMain = create_path()

# generation of 1000 events according to the specified DECAY table
# Y(4S) -> B-:tag- B+:sig
# B-:tag -> D0 pi-; D0 -> K- pi+
# B+:sig -> pi0 e+ nu_e
setupEventInfo(1000, myMain)
add_evtgen_generator(myMain, 'signal', Belle2.FileSystem.findFile(
    '/analysis/examples/exampleEvtgenDecayFiles/Btag2Dpi_Bsig2pi0enu.dec'))

# simulation
add_simulation(myMain)

# reconstruction
add_reconstruction(myMain)

# create final state particle lists
kaons = ('K-', '')
pions = ('pi+', '')
elecs = ('e+', '')
photons = ('gamma', '')

fillParticleLists([kaons, pions, elecs, photons], True, myMain)

# reconstruct pi0 -> gamma gamma decay
reconstructDecay('pi0 -> gamma gamma', '0.05 < M < 1.7', 1, True, myMain)

# reconstruct D0 -> K- pi+ decay (and c.c.)
reconstructDecay('D0 -> K- pi+', '1.800 < M < 1.900', 1, True, myMain)

# reconstruct Btag -> D0 pi- (and c.c.)
reconstructDecay('B-:tag -> D0 pi-', '5.000 < M < 6.000', 1, True, myMain)

# reconstruct Bsig -> pi0 e+ [nu_e] (and c.c.)
reconstructDecay('B+:sig -> pi0 e+', '0.000 < M < 6.000', 10, True, myMain)

# reconstruct Y(4S) -> Btag Bsig
reconstructDecay('Upsilon(4S) -> B-:tag B+:sig', '0.000 < M < 11.000', 1, True, myMain)

# perform MC matching
matchMCTruth('pi0', myMain)
matchMCTruth('B-:tag', myMain)
matchMCTruth('B+:sig', myMain)
matchMCTruth('Upsilon(4S)', myMain)

decayhashmodule = register_module('ParticleMCDecayString')
decayhashmodule.param('listName', 'B-:tag')
myMain.add_module(decayhashmodule)


# create and fill RestOfEvent for B- and Y(4S) particles
buildRestOfEvent('B-:tag', path=myMain)
buildRestOfEvent('Upsilon(4S)', path=myMain)

# define what should be dumped to ntuple for Btag
toolsB = ['MCTruth', '^B-:tag -> D0 pi-']
toolsB += ['DeltaEMbc', '^B-:tag -> D0 pi-']
toolsB += ['MCDecayString', '^B-:tag -> D0 pi-']
toolsB += ['ROEMultiplicities', '^B-:tag -> D0 pi-']

# define what should be dumped to ntuple for Upsilon(4S)
tools4S = ['MCTruth', '^Upsilon(4S) -> ^B-:tag ^B+:sig']
tools4S += ['DeltaEMbc', 'Upsilon(4S) -> ^B-:tag ^B+:sig']
tools4S += ['ROEMultiplicities', '^Upsilon(4S) -> B-:tag B+:sig']
tools4S += ['RecoilKinematics', '^Upsilon(4S) -> B-:tag B+:sig']
tools4S += ['ExtraEnergy', '^Upsilon(4S) -> B-:tag B+:sig']

# write flat ntuples
ntupleFile('restOfEvent_pi0enu.root', myMain)
ntupleTree('btag', 'B-:tag', toolsB, myMain)
ntupleTree('btagbsig', 'Upsilon(4S)', tools4S, myMain)

process(myMain)
print(statistics)
