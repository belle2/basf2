#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

import basf2 as b2
from generators import add_evtgen_generator
from modularAnalysis import setupEventInfo
from modularAnalysis import loadGearbox
from reconstruction import add_mdst_output
from optparse import OptionParser
from ROOT import Belle2

parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=1000,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='B2Kpi_events.root')
(options, args) = parser.parse_args()

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)


# generation of 1000 events according to the specified DECAY table
# Y(4S) -> B0 B0bar
# one B0->K+pi-, other generic decay
mypath = b2.create_path()
setupEventInfo(int(options.nevents), mypath)
add_evtgen_generator(mypath, 'signal', Belle2.FileSystem.findFile('arich/examples/B2kpi.dec'))

# If the simulation and reconstruction is not performed in the sam job,
# then the Gearbox needs to be loaded with the loadGearbox() function.
loadGearbox(mypath)

# dump generated events in MDST format to the output ROOT file
add_mdst_output(mypath, True, options.filename)

# process all modules added to the mypath path
b2.process(mypath)

# print out the summary
print(b2.statistics)
