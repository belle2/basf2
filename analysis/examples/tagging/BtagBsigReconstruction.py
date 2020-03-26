#!/usr/bin/env python3

import basf2
import modularAnalysis as ma
from generators import add_evtgen_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
import variables.collections as vc
import variables.utils as vu
from ROOT import Belle2


myMain = basf2.create_path()

# generation of 1000 events according to the specified DECAY table
# Y(4S) -> B-:tag- B+:sig
# B-:tag -> D0 pi-; D0 -> K- pi+
# B+:sig -> pi0 e+ nu_e
ma.setupEventInfo(1000, myMain)
add_evtgen_generator(myMain, 'signal', Belle2.FileSystem.findFile(
    'analysis/examples/exampleEvtgenDecayFiles/Btag2Dpi_Bsig2pi0enu.dec'))

# simulation
add_simulation(myMain)

# reconstruction
add_reconstruction(myMain)

# create final state particle lists
kaons = ('K-', '')
pions = ('pi+', '')
elecs = ('e+', '')
photons = ('gamma', '')

ma.fillParticleLists([kaons, pions, elecs, photons], True, myMain)

# reconstruct pi0 -> gamma gamma decay
ma.reconstructDecay('pi0 -> gamma gamma', '0.05 < M < 1.7', 1, True, myMain)

# reconstruct D0 -> K- pi+ decay (and c.c.)
ma.reconstructDecay('D0 -> K- pi+', '1.800 < M < 1.900', 1, True, myMain)

# reconstruct Btag -> D0 pi- (and c.c.)
ma.reconstructDecay('B-:tag -> D0 pi-', '5.000 < M < 6.000', 1, True, myMain)

# reconstruct Bsig -> pi0 e+ [nu_e] (and c.c.)
ma.reconstructDecay('B+:sig -> pi0 e+', '0.000 < M < 6.000', 10, True, myMain)

# reconstruct Y(4S) -> Btag Bsig
ma.reconstructDecay('Upsilon(4S) -> B-:tag B+:sig', '0.000 < M < 11.000', 1, True, myMain)

# perform MC matching
ma.matchMCTruth('pi0', myMain)
ma.matchMCTruth('B-:tag', myMain)
ma.matchMCTruth('B+:sig', myMain)
ma.matchMCTruth('Upsilon(4S)', myMain)

decayhashmodule = basf2.register_module('ParticleMCDecayString')
decayhashmodule.param('listName', 'B-:tag')
myMain.add_module(decayhashmodule)


# create and fill RestOfEvent for B- and Y(4S) particles
ma.buildRestOfEvent('B-:tag', path=myMain)
ma.buildRestOfEvent('Upsilon(4S)', path=myMain)

# define variables for Btag ntuple
commonVariables = vc.mc_truth + vc.deltae_mbc
BvariableList = commonVariables + vc.roe_multiplicities

# define variables for Upsilon(4S) ntuple
Y4SvariableList = vc.mc_truth + vc.roe_multiplicities + vc.recoil_kinematics + vc.extra_energy
Y4SvariableList += vu.create_aliases(commonVariables, 'daughter(0, {variable})', 'Btag')
Y4SvariableList += vu.create_aliases(commonVariables, 'daughter(1, {variable})', 'Bsig')

# write flat ntuples
ma.variablesToNtuple('B-:tag', variables=BvariableList, filename='restOfEvent_pi0enu.root', treename='btag', path=myMain)
ma.variablesToNtuple('Upsilon(4S)', variables=Y4SvariableList, filename='restOfEvent_pi0enu.root', treename='btagbsig', path=myMain)

basf2.process(myMain)
print(basf2.statistics)
