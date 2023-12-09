#!/usr/bin/env python

##################################
# Parameters:
# 1:  Particle name
# 2:  Number of events
# 3:  Random number seed
##################################

import sys
import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction


def getpdgCode(x):
    return {
      'muon+': -13,
      'muon-': 13,
      'pion+': 211,
      'pion-': -211,
      'kaon+': 321,
      'kaon-': -321,
      'electron+': -11,
      'electron-': 11,
      'proton+': 2212,
      'proton-': -2212,
      'deuteron+': 1000010020,
      'deuteron-': -1000010020,
      'gamma': 22,
      'klong': 130,
    }[x]


particle = sys.argv[1]
pdgCode = getpdgCode(particle)
pdgCodeNeg = -pdgCode
nEvent = int(sys.argv[2])
stream = sys.argv[3]
print(pdgCode)
print(nEvent)

main = basf2.create_path()

# specify number of events to be generated in job
eventInfoSetter = basf2.register_module('EventInfoSetter')
eventInfoSetter.param('expList', [0])
eventInfoSetter.param('runList', [0])
eventInfoSetter.param('evtNumList', [nEvent])
main.add_module(eventInfoSetter)

# generate single-track events with positive pions
# particle gun to shoot particles in the detector
# choose the particles you want to simulate
pGun = basf2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [pdgCode, pdgCodeNeg],
    'nTracks': 1,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [2.5, 7.0],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [15., 150.],
    'phiGeneration': 'uniform',
    'phiParams': [0.0, 360.0],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }
pGun.param(param_pGun)
main.add_module(pGun)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

output = basf2.register_module('RootOutput')
outfile = f'{particle}_{stream}.root'
output.param('outputFileName', outfile)
output.param('branchNames', ['MCParticles', 'Muids'])
main.add_module(output)

basf2.process(main)

# Print call statistics
print(basf2.statistics)
