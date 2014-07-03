#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2


myMain = create_path()

# generation of 100 events according to the specified DECAY table
# Y(4S) -> B-:tag- B+:sig
# B-:tag -> D0 pi-; D0 -> K- pi+
# B+:sig -> pi0 e+ nu_e
generateY4S(100, Belle2.FileSystem.findFile('/analysis/examples/exampleEvtgenDecayFiles/Btag2Dpi_Bsig2pi0enu.dec'), myMain)

# simulation
add_simulation(myMain)

# reconstruction
add_reconstruction(myMain)

# do the analysis
loadReconstructedParticles(myMain)

# create final state particle lists
selectParticle('K-', [], True, myMain)
selectParticle('pi+', [], True, myMain)
selectParticle('pi-', [], True, myMain)
selectParticle('e+', [], True, myMain)
selectParticle('gamma', [], True, myMain)

# reconstruct pi0 -> gamma gamma decay
makeParticle('pi0 -> gamma gamma', {'M': (0.05, 1.7)}, True, myMain)

# reconstruct D0 -> K- pi+ decay (and c.c.)
makeParticle('D0 -> K- pi+', {'M': (1.800, 1.900)}, True, myMain)

# reconstruct Btag -> D0 pi- (and c.c.)
makeParticle('B-:tag -> D0 pi-', {'M': (5.000, 6.000)}, True, myMain)

# reconstruct Bsig -> pi0 e+ [nu_e] (and c.c.)
makeParticle('B+:sig -> pi0 e+', {'M': (0.000, 6.000)}, True, myMain)

# reconstruct Y(4S) -> Btag Bsig
makeParticle('Upsilon(4S) -> B-:tag B+:sig', {'M': (0.000, 11.000)}, True, myMain)

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
print statistics
