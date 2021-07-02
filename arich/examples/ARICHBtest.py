#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Usage:
# basf2 arich/examples/ARICHBtest2011.py --
#       -r 102 -n 10000
#
import basf2 as b2
from optparse import OptionParser
import os
import os.path
import sys

b2.set_log_level(b2.LogLevel.INFO)

outroot = 'arichbtest.root'

parser = OptionParser()
parser.add_option('-r', '--run', dest='runno', default='068',
                  help='analyse runno')
parser.add_option('-p', '--path', dest='path',
                  default='/afs/f9.ijs.si/data/belle/data/beamtests/2011/run',
                  help='datapath')

parser.add_option('-y', '--year', dest='year', default='2011',
                  help='beam test year')

parser.add_option('-t', '--track-mask', dest='mask', default='0x5',
                  help='track mask 4 bits')

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

fname = options.path + '/run' + options.runno + '.dat'

if not os.path.isfile(fname):
    print('File does not exist:')
    print(fname)
    fname += '.gz'
    print('Appending .gz...')
if not os.path.isfile(fname):
    print('File does not exist:')
    print(fname)
    sys.exit(-1)

if options.output == outroot:
    outroot = 'run_' + options.runno + '.root'
else:
    outroot = options.output

mask = int(options.mask, 16)
runno = int(options.runno)  # needed for geoarich module

print('TrackMask:' + options.mask)

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

btest = b2.register_module('arichBtest')
btest.param('mwpcTrackMask', [mask])
# btest.param('Filename', 'arich/modules/arichBtest/data/2011/track.dat')
btest.param('runList', [fname])
btest.param('outputFileName', outroot)
momentum = 120.0
if options.year == '2013':
    momentum = 3.0
print('Beam momentum ' + str(momentum))
btest.param('beamMomentum', momentum)

# Simulation module
g4sim = b2.register_module('FullSim')
# This line is necessary if you want to simulate Cerenkov photons!
# By default optical processes are not registered.
g4sim.param('RegisterOptics', 1)
# To speed up the simulation you can propagate
# only a selected fraction of photons.
# By default all photons are propagated.
g4sim.param('PhotonFraction', 0.3)
# Set up the visualization
g4sim.param('EnableVisualization', True)
# Here you can select visualization driver and visualization commands.
# This creates VRML file,
# change VRML2FILE to HepRepFile to create HepRep file.
g4sim.param('UICommandsAtIdle', ['/vis/open VRML2FILE', '/vis/drawVolume',
                                 '/vis/scene/add/axes 0 0 0 100 mm'])

# Saves the geometry as a Root file
geosaver = b2.register_module('ExportGeometry')
geosaver.param('Filename', 'Belle2Geo.root')

arichrec = b2.register_module('ARICHReconstructor')
arichrec.param('inputTrackType', 1)
arichrec.param('beamtest', 3)
arichrec.param('trackPositionResolution', 0.0)
arichrec.param('trackAngleResolution', 0.0)

profile = b2.register_module('Profile')
profile.param('outputFileName', 'profileusage.ps')

main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(paramloader)
# main.add_module(profile)
main.add_module(geobuilder)
main.add_module(btest)
main.add_module(arichrec)
# main.add_module(g4sim)
main.add_module(geosaver)

b2.process(main)

# Print basic event statistics to stdout
print('Event Statistics:')
print(b2.statistics)
