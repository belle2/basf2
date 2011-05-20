#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file which shows all usage options for the
# touschek TURTLE file reader module in the generators package.
# The generated particles are taken from an input file and
# fed through the Geant4 simulation and the output
# is stored in a root file.
#
# The different options for the touschek TURTLE reader are
# explained below.
# Uncomment/comment different lines to get the wanted
# settings
#
# Example steering file - 2011 Belle II Collaboration
########################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)

# to run the framework the used module needs to be registered
touschekinput = register_module('TouschekTURTLEInput')

# setting the options for the TouschekTURTLE reader:
touschekinput.param('FilenameLER',
                    '/home/iwsatlas1/nakano/mywork/outputs/test/out_LER_tous/BEAMtree.root'
                    )
touschekinput.param('FilenameHER',
                    '/home/iwsatlas1/nakano/mywork/outputs/test/out_LER_tous/BEAMtree.root'
                    )

# number of particles to be read in 1 go
# for the time being, it is recommended to
# process the particles in the same event.
# at some point this will be refined.
touschekinput.param('MaxParticles', 1)
# default is MaxParticles = 10.

touschekinput.param('ReadHER', False)
touschekinput.param('ReadLER', True)

# for a simple simulation job with output to a root file
# these additional modules are needed
evtmetagen = register_module('EvtMetaGen')
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# Setting the option for all non-touschek reader modules:
evtruninfo = {'ExpList': [0], 'RunList': [0], 'EvtNumList': [10]}
evtmetagen.param(evtruninfo)

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

simpleoutput.param('outputFileName', 'TouschekReaderOutput.root')

# creating the path for the processing
main = create_path()
main.add_module(evtmetagen)
main.add_module(paramloader)
main.add_module(geobuilder)

# Add Particle Gun module to path:
main.add_module(touschekinput)
# and print parameters for hepevtreader
# on startup of process
print_params(touschekinput)

# Add all other modules for simple processing to path
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process 100 events
process(main)
# if there are less events in the input file
# the processing will be stopped at EOF.
