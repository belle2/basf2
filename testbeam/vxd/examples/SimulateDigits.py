#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script does full TB MC simulation
# Its outpt can be directly used by Digits2Tracks.py like real data

from basf2 import *
from alignment_tools import *

# Set the log level to show only warning, error and, fatal messages
# otherwise there's gonna be a segfault when python exits
set_log_level(LogLevel.WARNING)
release = str(os.getenv('BELLE2_LOCAL_DIR')) + '/'

# ----------------------------------------------------------------
#                  IMPORTANT SIMULATION SETTINGS
# ----------------------------------------------------------------

events = 10000
event_tracks = 1

momentum = 3.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # # degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v2.xml'
# geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v1.xml'

# Custom alignment must be set at the very begining as it modifies main xml
# during execution of basf2
# NOTE: You need a separated (from main xml) file for playing with alignment
alignment = release + 'testbeam/vxd/data/nominal_alignment.xml'

# Magnet ON/OFF
PCMAG_ON = True  # Only 0T/1T

# -----------------------------------------------------------------

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# Set the custom alignment
set_alignment = SetAlignment(release + 'data/' + geometry_file, alignment)

# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('fileName', geometry_file)

# Create geometry with/without Magnetic field
geometry = register_module('Geometry')
if PCMAG_ON:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# Set up particle gun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', event_tracks)
# DESY electrons:
particlegun.param('pdgCodes', [11])
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)

# Full simulation module
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)

# PXD digitizer
pxddigi = register_module('PXDDigitizer')
pxddigi.param('SimpleDriftModel', False)  # To load B-filed from memory

# SVD digitizer
svddigi = register_module('SVDDigitizer')

# EUDET digitizer ... need Lorentz angle (rescaled from 0.25 for 1.5T)
teldigi = register_module('TelDigitizer')
if PCMAG_ON:
    teldigi.param('tanLorentz', 0.1625)
else:
    teldigi.param('tanLorentz', 0.)

dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', 'MergedDigits.root')
# Here you can state which branches you want to output
# Using this branches, you remove Monte Carlo information and the resulting
# file will look exactly as merged digits from real testbeam data
dataWriter.param('branchNames', ['EventMetaData', 'TelEventInfo', 'PXDDigits',
                 'SVDDigits', 'TelDigits'])

progress = register_module('Progress')

# --------------------------------------------------------

main = create_path()
main.add_module(eventinfosetter)
main.add_module(set_alignment)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(pxddigi)
main.add_module(svddigi)
main.add_module(teldigi)
main.add_module(dataWriter)
main.add_module(progress)

process(main)
print statistics
