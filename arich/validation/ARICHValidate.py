#!/usr/bin/env python3

"""
<header>
<contact>Luka Santelj</contact>
<description>This is a steering file to generate and analyze 10000 events used for ARICH
validation plots.
</description>
<output>../ARICHEvents.root</output>
</header>
"""

import basf2 as b2
from optparse import OptionParser
from simulation import add_simulation
from reconstruction import add_mc_reconstruction

# Options from command line
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=10000,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='../ARICHEvents.root')
parser.add_option('-d', '--debug', dest='debugLevel', default=10)
(options, args) = parser.parse_args()
nevents = int(options.nevents)
filename = options.filename
debugLevel = int(options.debugLevel)

# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
b2.set_log_level(b2.LogLevel.INFO)

# Create path
main = b2.create_path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevents]})
main.add_module(eventinfosetter)

# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Setting the random seed for particle generation:
b2.set_random_seed(123456)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-211, 211, 321, -321])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [3.0, 3.5])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [17.0, 35.0])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0.0, 0.0])
particlegun.param('yVertexParams', [0.0, 0.0])
particlegun.param('zVertexParams', [0.0, 0.0])
particlegun.param('independentVertices', False)
# Print the parameters of the particle gun
b2.print_params(particlegun)
main.add_module(particlegun)

# Add simulation
# choose second line if you want to add background
add_simulation(main)
# add_simulation(main, components, bkgfiles=glob.glob('/sw/belle2/bkg/ARICH*.root'))

# Add reconstruction
add_mc_reconstruction(main)

# Add module fpr ARICH efficiency analysis
arichEfficiency = b2.register_module('ARICHNtuple')
arichEfficiency.logging.log_level = b2.LogLevel.DEBUG
arichEfficiency.logging.debug_level = debugLevel
arichEfficiency.param('outputFile', filename)
main.add_module(arichEfficiency)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
