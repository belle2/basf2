#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Particle gun muon events for EKLM, only EKLM digitization and reconstruction.

import os
import random
from basf2 import *
import ROOT

set_log_level(LogLevel.INFO)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [200])

# Particle gun
particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [13, -13])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 3])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [17, 150])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# Geometry parameter loader
paramloader = register_module('Gearbox')

# Geometry builder
geobuilder = register_module('Geometry')
geobuilder.param('components', ['EKLM'])

# Full Geant4 simulation
g4sim = register_module('FullSim')

# Root file output
output = register_module('RootOutput')
output.param('outputFileName', 'ParticleGunMuonsEKLM.root')

# EKLM digitizer
eklmdigi = register_module('EKLMDigitizer')

# EKLM packer
eklmpack = register_module('EKLMRawPacker')

# EKLM unpacker
eklmUpack = register_module('EKLMUnpacker')
eklmUpack.param('outputDigitsName', 'EKLMDigits_UP')

# EKLM reconstructor
eklmreco = register_module('EKLMReconstructor')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(eklmdigi)
main.add_module(eklmpack)
main.add_module(eklmUpack)
main.add_module(eklmreco)

main.add_module(output)

# Process 100 events
process(main)
print(statistics)

# Print size of raw data
root_file = ROOT.TFile('ParticleGunMuonsEKLM.root')
tree = root_file.Get('tree')
events = tree.GetEntriesFast()
print('Events: %d' % events)
for branch in tree.GetListOfBranches():
    name = branch.GetName()
    if name.startswith('Raw'):
        size = branch.GetTotBytes('*') * 1.0
        zipsize = branch.GetZipBytes('*') * 1.0
        print("%s %.2f (%.2f)" % (name, size / 1024. / events, zipsize / 1024. / events))
