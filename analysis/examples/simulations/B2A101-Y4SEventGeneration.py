#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# Y(4S) -> BBbar event generation                                        #
#                                                                        #
# This tutorial demonstrates how to generate                             #
#                                                                        #
# e+e- -> Y(4S) -> BBbar                                                 #
#                                                                        #
# events with EvtGen in BASF2, where the decay of Y(4S)                  #
# is specified by the given .dec file.                                   #
#                                                                        #
# The generated events are saved to the output ROOT file.                #
# In each event the generated particles (MCParticle objects)             #
# are stored in the StoreArray<MCParticle>.                              #
#                                                                        #
##########################################################################

import basf2 as b2
import generators as ge
import modularAnalysis as ma

# generation of 100 events according to the specified DECAY table
# Y(4S) -> Btag- Bsig+
# Btag- -> D0 pi-; D0 -> K- pi+
# Bsig+ -> mu+ nu_mu
#

# Defining custom path
my_path = b2.create_path()

# Setting up number of events to generate
ma.setupEventInfo(noEvents=100, path=my_path)

# Adding genberator
ge.add_evtgen_generator(path=my_path,
                        finalstate='signal',
                        signaldecfile=b2.find_file(
                            'analysis/examples/simulations/B2A101-Y4SEventGeneration.dec'))

# dump generated events in DST format to the output ROOT file
my_path.add_module('RootOutput', outputFileName='B2A101-Y4SEventGeneration-evtgen.root')

# process all modules added to the path
b2.process(path=my_path)

# print out the summary
print(b2.statistics)
