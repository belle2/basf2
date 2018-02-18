#!/usr/bin/env python
# -*- coding: utf-8 -*-

# -------------------------------------------------------------------------------------
# Example steering file for beast2 simulation
# Builds beast2 geometry and uses particle gun module to shoot particles
# Uncomment the lines related to the display module to show the detector geometry
# Simulation output hits are stored into "beast_test.root"
#
# Author: Luka Santelj
# Date: 15.2.2018
# -------------------------------------------------------------------------------------

from basf2 import *
import sys
import os
from background import add_output

# set parameters
main = create_path()

# Event info setter
eventinfosetter = register_module('EventInfoSetter')
# set number of events
eventinfosetter.param('evtNumList', [200])
main.add_module(eventinfosetter)

# Uncomment if you include your study/analysis module into the path
# Study modules (in your detector folder) are HistoModules, they produce histograms,
# and HistoManager module takes care to store them in the output root file
#
# histo = register_module('HistoManager')
# histo.param('histoFileName', 'histograms.root')  # File to save histograms
# main.add_module(histo)

# Gearbox - reads parameters from the xml files
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
main.add_module(gearbox)

# Geant geometry
geometry = register_module('Geometry')
# select detectors to be built (IR structures+VXD+Beast2)
geometry.param('components', ["STR", "BeamPipe", "Cryostat",
                              "HeavyMetalShield", "PXD", "SVD", "MICROTPC", "PINDIODE",
                              "BEAMABORT", "HE3TUBE", "CLAWS", "FANGS", "PLUME", "QCSMONITOR",
                              "MagneticField3dQuadBeamline"])
main.add_module(geometry)

# Overlap checker / if you make g4 geometry changes please check
# that there are no overlaps
# main.add_module("OverlapChecker")
# Save overlaps to file to be able to view them with b2display
# main.add_module("RootOutput", outputFileName="Overlaps.root")


# particle gun module, shoot particles into your detector
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [11])  # electron
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [1, 1.01])  # 1 GeV
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [0, 360])  # theta angle
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])  # phi angle
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
main.add_module(particlegun)

# Geant simulation
fullsim = register_module('FullSim')
fullsim.param('PhysicsList', 'FTFP_BERT_HP')
fullsim.param('UICommands', ['/process/inactivate nKiller'])
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.000001)  # [MeV] need for CDC EB neutron flux
main.add_module(fullsim)

# Add additional modules, like digitization and study/analysis
# for example He3Digitizer
# dataobjects created in these modules are also put into the datastore
# and finally saved into the output root file (he3 digits in this case)
#
# he3digi = register_module('He3Digitizer')
# main.add_module(he3digi)
#
# if you add study/analysis module uncomment also HistoManager lines
#
# he3study = register_module('He3tubeStudy')
# main.add_module(he3study)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# uncomment below to add the event display module
# display = register_module('Display')
# display.param('fullGeometry', True)
# main.add_module(display)

# store output hits into root file (these is collection of hits, not histograms)
# by default all content of datastore is stored
output = register_module('RootOutput')
output.param('outputFileName', "beast_test.root")
# if you want to store only branches of interest (hits in your detector, etc.) use
# output.param('branchNames', ['PlumeSimHits'])
main.add_module(output)


# Process events
process(main)

# Print call statistics
print(statistics)
