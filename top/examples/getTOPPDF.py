#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from enum import Enum
from argparse import ArgumentParser
from basf2 import *
from tracking import add_tracking_reconstruction, add_cr_tracking_reconstruction
from svd import add_svd_reconstruction, add_svd_simulation
from pxd import add_pxd_reconstruction, add_pxd_simulation
from simulation import add_simulation
from reconstruction import add_reconstruction


# command line options
ap = ArgumentParser('')
ap.add_argument('--local-db', action='store_true', help='should I read from the local database (e.g. for channel masking)')
ap.add_argument(
    '--config',
    type=int,
    help='particle gun setup code (0 == muons from origin, 1 == muons from above, 2 == muons from side)',
    default=1)
ap.add_argument(
    '--cosmics',
    default=True,
    help='should we run the cosmic ray tracking (be careful with this in combination with particle gun config)')
ap.add_argument('--npdfs', type=int, default=1, help='how many pdfs should we write out (-1 is one pdf for each track)')
ap.add_argument('--full-output', action='store_true', help='do you want a full mdst or just the relavent TOP objects?')
ap.add_argument('--particle', type=int, default=13, help='pdg code of the particles to generate (13, 211, 321, etc)')
ap.add_argument('--output', '-o', default='TOPOutput.root', help='Output filename')
opts = ap.parse_args()

# use a pyhon3 enum for readable particle gun configuration setup


class ParticleGunConfig(Enum):
    FROMORIGIN = 0
    FROMABOVE = 1
    FROMSIDE = 2


opts.particlegun = ParticleGunConfig(opts.config)
print("Particle gun configuration selected is: ", opts.particlegun)

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# channel mask
if opts.local_db:
    use_local_database("localDB/localDB.txt", "localDB", False)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)
geometry.param('components', [
        'MagneticField',
        'BeamPipe',
        'PXD',
        'SVD',
        'CDC',
        'TOP',
])

# Particle gun: generate multiple tracks
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [opts.particle])
# TODO +/-
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('independentVertices', False)

if opts.particlegun == ParticleGunConfig.FROMORIGIN:
    # from the IP up and at 45 degrees
    particlegun.param('momentumGeneration', 'uniform')
    particlegun.param('momentumParams', [2, 2])
    particlegun.param('thetaGeneration', 'uniformCos')
    particlegun.param('thetaParams', [45, 45])
    particlegun.param('phiGeneration', 'uniform')
    particlegun.param('phiParams', [90, 90])
    particlegun.param('vertexGeneration', 'fixed')
    particlegun.param('xVertexParams', [0])
    particlegun.param('yVertexParams', [0])
    particlegun.param('zVertexParams', [0])
elif opts.particlegun == ParticleGunConfig.FROMABOVE:
    # pseudo-cosmics: muons from above downwards at some angle
    particlegun.param('momentumGeneration', 'uniform')
    particlegun.param('momentumParams', [2, 2])
    particlegun.param('thetaGeneration', 'uniformCos')
    particlegun.param('thetaParams', [-80, -80])
    particlegun.param('phiGeneration', 'uniform')
    particlegun.param('phiParams', [270, 270])
    particlegun.param('vertexGeneration', 'fixed')
    particlegun.param('xVertexParams', [10])
    particlegun.param('yVertexParams', [136.5])
    particlegun.param('zVertexParams', [0])
elif opts.particlegun == ParticleGunConfig.FROMSIDE:
    # rare cosmic events from left to right
    particlegun.param('momentumGeneration', 'uniform')
    particlegun.param('momentumParams', [2, 2])
    particlegun.param('thetaGeneration', 'uniformCos')
    particlegun.param('thetaParams', [-70, -70])
    particlegun.param('phiGeneration', 'uniform')
    particlegun.param('phiParams', [8, 8])
    particlegun.param('vertexGeneration', 'fixed')
    particlegun.param('xVertexParams', [-130])
    particlegun.param('yVertexParams', [-25])
    particlegun.param('zVertexParams', [0])

main.add_module(particlegun)
# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

add_svd_simulation(main)
add_pxd_simulation(main)

# PXD digitization & clustering
add_pxd_reconstruction(main)

# SVD digitization & clustering
add_svd_reconstruction(main)

# CDC digitization
cdcDigitizer = register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# TOP digitization
topdigi = register_module('TOPDigitizer')
main.add_module(topdigi)

# tracking
if opts.cosmics:
    add_cr_tracking_reconstruction(main)
else:
    # then we want the normal tracking reconstruction
    add_tracking_reconstruction(main)

# Track extrapolation
ext = register_module('Ext')
main.add_module(ext)

# TOP reconstruction
top_cm = register_module('TOPChannelMasker')
main.add_module(top_cm)
topreco = register_module('TOPReconstructorPDF')
topreco.logging.log_level = LogLevel.DEBUG  # remove or comment to suppress printout
topreco.logging.debug_level = 2  # or set level to 0 to suppress printout
topreco.param("writeNPdfs", opts.npdfs)
main.add_module(topreco)

# TOP DQM
topdqm = register_module('TOPDQM')
main.add_module(topdqm)

# Output
output = register_module('RootOutput')
output.param('outputFileName', opts.output)
branches = [
    'TOPDigits',
    'TOPRawWaveforms',
    'TOPPDFCollections',
    'TOPSmallestPullCollections'
]
if not opts.full_output:
    print("saving a subset of top information only")
    output.param('branchNames', branches)
else:
    print("saving the whole datastore")
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
