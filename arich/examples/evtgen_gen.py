#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Y(4S) -> BBbar event generation
#
# This script demonstrates how to generate
#
# e+e- -> Y(4S) -> BBbar
#
# events with EvtGen in BASF2, where the decay of Y(4S)
# is specified by the given .dec file.
#
# The generated events are saved to the output ROOT file.
# In each event the generated particles (MCParticle objects)
# are stored in the StoreArray<MCParticle>.
#
# Author: Luka Santelj (Oct. 2016)
######################################################

from basf2 import *
from generators import add_evtgen_generator
from modularAnalysis import setupEventInfo
from modularAnalysis import loadGearbox
from reconstruction import add_mdst_output
from modularAnalysis import analysis_main
from optparse import OptionParser
from ROOT import Belle2

parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=1000,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='B2Kpi_events.root')
(options, args) = parser.parse_args()

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)


# generation of 1000 events according to the specified DECAY table
# Y(4S) -> B0 B0bar
# one B0->K+pi-, other generic decay
setupEventInfo(int(options.nevents), analysis_main)
add_evtgen_generator(analysis_main, 'signal', Belle2.FileSystem.findFile('arich/examples/B2kpi.dec'))

# If the simulation and reconstruction is not performed in the sam job,
# then the Gearbox needs to be loaded with the loadGearbox() function.
loadGearbox()

# dump generated events in MDST format to the output ROOT file
add_mdst_output(analysis_main, True, options.filename)

# process all modules added to the analysis_main path
# (note: analysis_main is the default path created in the modularAnapys.py)
process(analysis_main)

# print out the summary
print(statistics)
