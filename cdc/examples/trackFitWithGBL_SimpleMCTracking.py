#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################################
#
# This steering file creates the Belle II detector geometry, and perfoms the
# simulation and MC based track finding and fitting.
#
# EventInfoSetter and EventInfoPrinter generates and shows event meta data (see example
# in the framework package). Gearbox and Geometry are used to create the Belle2
# detector geometry. The generator used in this example is geant4 particle gun
# (see example in the simulation or generator package). FullSim performs the
# full simulation.
#
# CDCDigitizer creates the detector response in the CDC for the simulated Hits.
# For the PXD and SVD currently the TrueHits are used (created directly by the
# sensitive detector), will be replaced by realistic clusters later on.
#
# MCTrackFinder creates relations between MCParticles and CDCHits/PXDTrueHits
# /SVDTrueHits produced by it. GenFitter fits the found MCTracks and created
# two track collections: GFTracks (Genfit class) and Tracks (class with helix
# parametrization)
#
# For details about module parameters just type > basf2 -m .
#
##############################################################################
##

import basf2 as b2

import simulation
import reconstruction

import modularAnalysis as ana

b2.set_log_level(b2.LogLevel.INFO)

# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [1])
eventinfoprinter = b2.register_module('EventInfoPrinter')

# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')

# particle gun to shoot particles in the detector
pGun = b2.register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {
    'pdgCodes': [13],
    'nTracks': 1,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [5.0, 5.01],
    'thetaGeneration': 'uniform',
    'thetaParams': [90., 90.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)

# simulation
g4sim = b2.register_module('FullSim')
# this is needed for the MCTrackFinder to work correctly <- obsolete ?

# digitizer
cdcDigitizer = b2.register_module('CDCDigitizer')

# find MCTracks
mctrackfinder = b2.register_module('TrackFinderMCTruthRecoTracks')

# select which detectors you would like to use
param_mctrackfinder = {  # select which particles to use: primary particles
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'WhichParticles': ['primary'],
}
mctrackfinder.param(param_mctrackfinder)

# setupgf
setupgf = b2.register_module('SetupGenfitExtrapolation')

# fitting
cdcfitting = b2.register_module('GBLRecoFitter2', resolveAmbiguities=1, externalIterations=1)

# build track
buildtrack = b2.register_module("TrackCreator")

# output
output = b2.register_module('RootOutput')
output.param('outputFileName', 'SimpleMCTrackingOutput.root')

# create path
main = b2.create_path()

# add modules to path
main.add_module(eventinfosetter)
main.add_module('HistoManager')
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(pGun)

simulation.add_simulation(main)
reconstruction.add_reconstruction(main, pruneTracks=False, add_muid_hits=False, components=["PXD", "SVD", "CDC"])

main.add_module(cdcfitting)
main.add_module('AlignDQM')
main.add_module('TrackDQM')

main.add_module(output)


ana.fillParticleList("pi+:allCharged", "", path=main)

track_variables = [
    'chiProb',
    'd0',
    'z0',
    'phi0',
    'omega',
    'tanLambda',
    'pt',
    'pionID',
    'protonID',
    'electronID',
    'muonID',
    'deuteronID',
    'nVXDHits',
    'nPXDHits',
    'nSVDHits',
    'nCDCHits',
    'nTracks',
    'x',
    'y',
    'z']

ana.variablesToNtuple("pi+:allCharged",
                      variables=track_variables,
                      filename='ntuples.root', path=main)

# Process events
b2.print_path(main)
b2.process(main)
print(b2.statistics)
