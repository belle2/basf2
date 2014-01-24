#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction

myMain = create_path()

# generation of 100 events according to the specified DECAY table
# Y(4S) -> Btag- Bsig+
# Btag- -> D0 pi-; D0 -> K- pi+
# Bsig+ -> pi0 e+ nu_e
generateEvents(20, os.environ['BELLE2_LOCAL_DIR']
               + '/analysis/examples/exampleEvtgenDecayFiles/Btag2Dpi_Bsig2pi0enu.dec'
               , myMain)

# simulation
add_simulation(myMain)

# reconstruction
add_reconstruction(myMain)

# do the analysis
loadReconstructedParticles(myMain)

# create final state particle lists
selectParticle('K-', -321, [], True, myMain)
selectParticle('pi+', 211, [], True, myMain)
selectParticle('pi-', -211, [], True, myMain)
selectParticle('pi0', 111, [], True, myMain)
selectParticle('e+', -11, [], True, myMain)
selectParticle('g', 22, [], True, myMain)

# reconstruct D0 -> K- pi+ decay (and c.c.)
makeParticle(
    'D0',
    421,
    ['K-', 'pi+'],
    1.800,
    1.900,
    True,
    myMain,
    )

# reconstruct Btag -> D0 pi- (and c.c.)
makeParticle(
    'B-',
    -521,
    ['D0', 'pi-'],
    5.000,
    6.000,
    True,
    myMain,
    )

# reconstruct Bsig -> pi0 e+ [nu_e] (and c.c.)
makeParticle(
    'B+',
    521,
    ['pi0', 'e+'],
    0.000,
    6.000,
    True,
    myMain,
    )

# reconstruct Y(4S) -> Btag Bsig
makeParticle(
    'Upsilon(4S)',
    300553,
    ['B-', 'B+'],
    0.000,
    11.000,
    True,
    myMain,
    )

# perform MC matching
matchMCTruth('Upsilon(4S)', myMain)

# create and fill RestOfEvent for B- and Y(4S) particles
buildRestOfEvent('B-', myMain)
buildRestOfEvent('Upsilon(4S)', myMain)

# define what should be dumped to ntuple for Btag
toolsB = ['MCTruth', '^B- -> D0 pi-']
toolsB += ['DeltaEMbc', '^B- -> D0 pi-']
toolsB += ['ROEMultiplicities', '^B- -> D0 pi-']

# define what should be dumped to ntuple for Upsilon(4S)
tools4S = ['MCTruth', '^Upsilon(4S) -> ^B- ^B+']
tools4S += ['DeltaEMbc', 'Upsilon(4S) -> ^B- ^B+']
tools4S += ['ROEMultiplicities', '^Upsilon(4S) -> B- B+']
tools4S += ['RecoilKinematics', '^Upsilon(4S) -> B- B+']
tools4S += ['ExtraEnergy', '^Upsilon(4S) -> B- B+']

# write flat ntuples
ntupleFile('restOfEvent_genu.root', myMain)
ntupleTree('btag', 'B-', toolsB, myMain)
ntupleTree('btagbsig', 'Upsilon(4S)', tools4S, myMain)

process(myMain)
print statistics

