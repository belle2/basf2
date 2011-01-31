#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file which shows all usage options for the
# hepevt file reader moduile in the generators package.
# The generated particles are taken from an input file and
# fed through the Geant4 simulation and the output
# is stored in a root file.
#
# The different options for the particle gun are explained
# below.
# Uncomment/comment different lines to get the wanted
# settings
#
# Example steering file - 2011 Belle II Collaboration
########################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(3)

# to run the framework the used modules need to be registered
hepevtreader = register_module('HepevtInput')

# setting the options for the HEPEVT reader:

# indicate the filename where the hepevtreader
# should read events from
hepevtreader.param('inputFileName', 'BhWide_10events.txt')
# this file is in the same directory. For more and different
# hepevt files look and download from the Twiki.

# in case you don't want to process the first events
# but only all events after event 1000
# uncomment the following line
# hepevtreader.param("skipEvents", 1000)
# default is skipEvents = 0

# if the events in the HepEvt file are weighted and
# you want to use the event weights use this line
# hepevtreader.param("useWeights", True)
# default is useWeight = False, all events have the same weight

# for a simple simulation job with output to a root file
# these additional modules are needed
evtmetagen = register_module('EvtMetaGen')
paramloader = register_module('ParamLoaderXML')
geobuilder = register_module('GeoBuilder')
g4sim = register_module('FullSim')
simpleoutput = register_module('SimpleOutput')

# Setting the option for all non-hepevt reader modules:
evtmetagen.param('EvtNumList', [100])

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

simpleoutput.param('outputFileName', 'HepEvTReaderOutput.root')

# creating the path for the processing
main = create_path()
main.add_module(evtmetagen)

# Add Particle Gun module to path:
main.add_module(hepevtreader)
# and print parameters for hepevtreader
# on startup of process
print_params(hepevtreader)

# Add all other modules for simple processing to path
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(simpleoutput)

# Process events
process(main)
