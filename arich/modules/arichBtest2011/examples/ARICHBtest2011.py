#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Usage: basf2 arich/modules/arichBtest2011/examples/ARICHBtest2011.py -- -r 102 -n 10000
#
from basf2 import *
from optparse import OptionParser
import os
import os.path
import sys

outroot = 'arichbtest.root'

parser = OptionParser()
parser.add_option('-r', '--run', dest='runno', default='068',
                  help='analyse runno')
parser.add_option('-p', '--path', dest='path',
                  default='/afs/f9.ijs.si/home/rok/aerorich/tests/11/ana/bdata/bt11'
                  , help='datapath')

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
    print 'File does not exist:'
    print fname
    fname += '.gz'
    print 'Appending .gz...'
if not os.path.isfile(fname):
    print 'File does not exist:'
    print fname
    sys.exit(-1)

if options.output == outroot:
    outroot = 'run_' + options.runno + '.root'
else:
    outroot = options.output

mask = int(options.mask, 16)
runno = int(options.runno)  # needed for geoarich module

print 'TrackMask:' + options.mask

averageagel = int(options.avgagel)  # this variable is called from GeoARICHBtest2011Creator

set_log_level(LogLevel.INFO)

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [int(options.neve)])  # we want to process 10 events
evtmetagen.param('RunList', [int(options.runno)])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1

# Load XML parameters
paramloader = register_module('Gearbox')

xmlgeometry = \
    'file://%s/arich/modules/arichBtest2011/geometry/arichBtest2011.xml' \
    % os.getcwd()
paramloader.param('Filename', xmlgeometry)
# paramloader.param('Backends', ['sql:'])
# paramloader.param('Filename','mysql://basf2:belle2@f9lab02.ijs.si:3306/b2config');
# paramloader.param('Filename','pgsql://basf2:belle2@f9lab02.ijs.si:5432/b2config');
# paramloader.param('Filename', 'oracle://basf2:belle2@/f9lab02')
# paramloader.param('Filename','file:///afs/f9.ijs.si/home/rok/public_html/basf2/public/Belle2.xml');
# paramloader.param('Filename','file:///net/f9pc137/data0/belle2/rok/local/basf2/test/Belle2-merged.xml');
# Create Geometry
geobuilder = register_module('Geometry')
geobuilder.param('Components', ['ARICHBtest2011'])

btest = register_module('arichBtest2011')
btest.param('mwpcTrackMask', [mask])
# btest.param('Filename', 'arich/modules/arichBtest2011/geometry/track.dat')
btest.param('runList', [fname])
btest.param('outputFileName', outroot)

# Simulation module
g4sim = register_module('FullSim')
# This line is necessary if you want to simulate Cerenkov photons!
# By default optical processes are not registered.
g4sim.param('RegisterOptics', 1)
# To speed up the simulation you can propagate only a selected fraction of photons.
# By default all photons are propagated.
g4sim.param('PhotonFraction', 0.3)
# Set up the visualization
g4sim.param('EnableVisualization', True)
# Here you can select visualization driver and visualization commands.
# This creates VRML file, change VRML2FILE to HepRepFile to create HepRep file.
g4sim.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
            '/vis/scene/add/axes 0 0 0 100 mm'])

# Saves the geometry as a Root file
geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'Belle2Geo.root')

arichrec = register_module('ARICHReconstructor')
arichrec.param('InputTrackType', 1)

main = create_path()
main.add_module(evtmetagen)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(btest)
main.add_module(arichrec)
# main.add_module(g4sim)
main.add_module(geosaver)

process(main)

# Print basic event statistics to stdout
print 'Event Statistics:'
print statistics
