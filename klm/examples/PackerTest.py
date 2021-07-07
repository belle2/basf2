#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Particle gun muon events for KLM, only KLM digitization and reconstruction.

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
geobuilder.param('components', ['KLM'])
geobuilder.param('useDB', False)

# Full Geant4 simulation
g4sim = basf2.register_module('FullSim')

# Digitizers
klm_digitizer = basf2.register_module('KLMDigitizer')

# Packer
klm_packer = basf2.register_module('KLMPacker')

# Unpacker
klm_unpacker = basf2.register_module('KLMUnpacker')
klm_unpacker.param('outputKLMDigitsName', 'KLMDigitsUnpacked')

# Reconstructor
klm_reconstructor = basf2.register_module('KLMReconstructor')

# Root file output
output = basf2.register_module('RootOutput')
output.param('outputFileName', 'ParticleGunMuonsKLM.root')

# Create main path
main = basf2.create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(particlegun)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)

main.add_module(klm_digitizer)
main.add_module(klm_packer)
main.add_module(klm_unpacker)
main.add_module(klm_reconstructor)

main.add_module(output)

# Process the events
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
