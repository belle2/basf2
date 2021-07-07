#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#
# Usage:
# basf2 arich/examples/ARICHBtest2011.py --
#       -r 102 -n 10000
#
import basf2 as b2
from optparse import OptionParser
import os
import os.path

b2.set_log_level(b2.LogLevel.INFO)

outroot = 'arichbtest.root'

parser = OptionParser()
parser.add_option('-r', '--run', dest='runno', default='068',
                  help='analyse runno')

parser.add_option('-y', '--year', dest='year', default='2013',
                  help='beam test year')

parser.add_option('-m', '--avgagel', dest='avgagel', default='0',
                  help='average thc calculation based on the mean agel')

parser.add_option(
    '-o',
    '--output',
    dest='output',
    default=outroot,
    help='Output filename',
    metavar='FILE',
)

parser.add_option('-n', '--neve', dest='neve', default=20000,
                  help='Number of events to process')

(options, args) = parser.parse_args()

if options.output == outroot:
    outroot = 'run_' + options.runno + '.root'
else:
    outroot = options.output

runno = int(options.runno)  # needed for geoarich module

# this variable is called from GeoARICHBtest2011Creator
averageagel = int(options.avgagel)

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [int(options.neve)])
eventinfosetter.param('runList', [int(options.runno)])
eventinfosetter.param('expList', [1])

# Load XML parameters
paramloader = b2.register_module('Gearbox')

xmlgeometry = 'file://%s/arich/modules/arichBtest/data/%s/arichBtest%s.xml' \
    % (os.getcwd(), options.year, options.year)
paramloader.param('fileName', xmlgeometry)
print(xmlgeometry)
paramloader.param('fileName', xmlgeometry)

# database testing
# paramloader.param('Backends', ['sql:'])
# paramloader.param('Filename',
#             'mysql://basf2:belle2@f9lab02.ijs.si:3306/b2config');
# paramloader.param('Filename',
#             'pgsql://basf2:belle2@f9lab02.ijs.si:5432/b2config');
# paramloader.param('Filename', 'oracle://basf2:belle2@/f9lab02')
# paramloader.param('Filename',
#        'file:///afs/f9.ijs.si/home/rok/public_html/basf2/public/Belle2.xml');
# paramloader.param('Filename',
#   'file:///net/f9pc137/data0/belle2/rok/local/basf2/test/Belle2-merged.xml');

# Create Geometry

geobuilder = b2.register_module('Geometry')
geobuilder.param('components', ['ARICHBtest'])

# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Setting the random seed for particle generation:
b2.set_random_seed(1097)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-211, 211])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# if you set nTracks to 0, then for each PDG code in pdgCodes list a track
# will be generated on each event.
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [3.5, 3.51])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [0, 0.2])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
# Setting the vertex position of shooted particles
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-2.5, 2.5])
particlegun.param('yVertexParams', [18, 22])
particlegun.param('zVertexParams', [-100, -101])
# Print the parameters of the particle gun
b2.print_params(particlegun)

# Simulation module
g4sim = b2.register_module('FullSim')
# g4sim.param('StoreOpticalPhotons',True)
# g4sim.param('SecondariesEnergyCut',0)
# This line is necessary if you want to simulate Cerenkov photons!
# By default optical processes are not registered.
g4sim.param('RegisterOptics', 1)
# To speed up the simulation you can propagate
# only a selected fraction of photons.
# By default all photons are propagated.
g4sim.param('PhotonFraction', 0.3)

arichDIGI = b2.register_module('ARICHDigitizer')

arichrec = b2.register_module('ARICHReconstructor')
arichrec.param('inputTrackType', 1)
# choose 1 for measured data, 2 for MC, 3 for data /w likelihood
# calculation, and 4 for MC /w likelihood
arichrec.param('beamtest', 2)
# include tracking resolution
# arichrec.param('trackPositionResolution', 0.0)
# arichrec.param('trackAngleResolution', 0.0)
# file containing cerenkov angle distribution, etc.
arichrec.param('outfileName', outroot)

# Saves the geometry as a Root file
geosaver = b2.register_module('ExportGeometry')
geosaver.param('Filename', 'BeamtestGeo.root')

main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(particlegun)
main.add_module(g4sim)
main.add_module(arichDIGI)
main.add_module(arichrec)
# main.add_module(geosaver)
b2.process(main)

# Print basic event statistics to stdout
print('Event Statistics:')
print(b2.statistics)
