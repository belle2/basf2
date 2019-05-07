#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Particle gun muon events for EKLM, only EKLM digitization and reconstruction.

import os
import random
import basf2
import ROOT

basf2.set_log_level(basf2.LogLevel.INFO)

# EventInfoSetter - generate event meta data
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [200])

# Particle gun
particlegun = basf2.register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [13, -13])
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 3])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [17, 150])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])

# Geometry parameter loader
paramloader = basf2.register_module('Gearbox')

# Geometry builder
geobuilder = basf2.register_module('Geometry')
geobuilder.param('components', ['EKLM', 'BKLM'])

# Full Geant4 simulation
g4sim = basf2.register_module('FullSim')

# Root file output
output = basf2.register_module('RootOutput')
output.param('outputFileName', 'ParticleGunMuonsKLM.root')

# Digitizers
bklm_digitizer = basf2.register_module('BKLMDigitizer')
eklm_digitizer = basf2.register_module('EKLMDigitizer')

# Packers
bklm_packer = basf2.register_module('BKLMRawPacker')
eklm_packer = basf2.register_module('EKLMRawPacker')

# Unpacker
klm_unpacker = basf2.register_module('KLMUnpacker')
klm_unpacker.param('outputBKLMDigitsName', 'BKLMDigitsUnpacked')
klm_unpacker.param('outputEKLMDigitsName', 'EKLMDigitsUnpacked')

# Reconstructors
bklm_reconstructor = basf2.register_module('BKLMReconstructor')
eklm_reconstructor = basf2.register_module('EKLMReconstructor')

# Create main path
main = basf2.create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(bklm_digitizer)
main.add_module(eklm_digitizer)
main.add_module(bklm_packer)
main.add_module(eklm_packer)
main.add_module(klm_unpacker)
main.add_module(bklm_reconstructor)
main.add_module(eklm_reconstructor)

main.add_module(output)

# Process 100 events
basf2.process(main)
print(basf2.statistics)

# Print size of raw data
root_file = ROOT.TFile('ParticleGunMuonsKLM.root')
tree = root_file.Get('tree')
events = tree.GetEntriesFast()
print('Events: %d' % events)
for branch in tree.GetListOfBranches():
    name = branch.GetName()
    if name.startswith('Raw'):
        size = branch.GetTotBytes('*') * 1.0
        zipsize = branch.GetZipBytes('*') * 1.0
        print("%s %.2f (%.2f)" % (name, size / 1024. / events, zipsize / 1024. / events))
