#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from beamparameters import add_beamparameters

# suppress messages and warnings during processing:
basf2.set_log_level(basf2.LogLevel.ERROR)
# set_log_level(LogLevel.DEBUG)

# creating the path for the processing
main = basf2.create_path()

# beam parameters
beamparameters = add_beamparameters(main, "SuperKEKB")

# to run the framework the used modules need to be registered
hepevtreader = basf2.register_module('HepevtInput')

# setting the options for the HEPEVT reader:
#
# indicate the list of filenames where the hepevtreader should read events from
hepevtreader.param('inputFileList', ['BhWide_10events.txt'])

# this file is in the same directory. For more and different hepevt files look
# and download from the Twiki. On the Twiki you will also find which options
# are needed for which hepevt file.
#
# in case you don't want to process the first events but only all events after
# event 1000 uncomment the following line hepevtreader.param("skipEvents",
# 1000) default is skipEvents = 0 (This is useful, if you want to process the
# data from the same HepEvtFile in more than 1 job)
#
# if the events in the HepEvt file are weighted and you want to use the event
# weights use this line hepevtreader.param("useWeights", True) default is
# useWeight = False, all events have the same weight
#
# if some of the particles in the event of the HEPEVT file should not be put to
# the simulation (incoming particles, virtual particles etc.) you can specify
# the number of virtual particles in each event. The BhWide generator from the
# input file used in this example gives first the two beam particles. So the
# first two particles should be treated as virtual particles.
hepevtreader.param('nVirtualParticles', 2)
# The default for this option is nVirtualParticles=0 NOTE: this option always
# assumes that the virtual particles are the first ones in the event and that
# there is a fixed number of them. For more complexe cases, this information
# should be provided by the generator inside the HEPEVT file. They will then
# require a specific input module taking account the peculiarities of this
# particular generator.
#
# if the generator calculated the interaction in the center of mass system, you
# can ask basf2 to boost automatically all particles to the LAB frame of
# BELLE2. This option is switched on with this line
hepevtreader.param('boost2Lab', True)
# For BhWide this option is needed. default is boost2LAB = false.
#
# if the generator uses a wrong convention for the directions of the positron
# and electron beams (wrong sign of Pz) and you want to use the boost to the
# LAB frame you should set the option wrongSignPz to true. Otherwise the boost
# goes wrong. This option is switched on with this line
hepevtreader.param('wrongSignPz', True)
# For BhWide this option is needed. default is wrongSignPz = false.
#
# for a simple simulation job with output to a root file these additional
# modules are needed
eventinfosetter = basf2.register_module('EventInfoSetter')
progress = basf2.register_module('Progress')
simpleoutput = basf2.register_module('RootOutput')

# Setting the option for all non-hepevt reader modules:
eventinfosetter.param('evtNumList', [100])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [0])  # and experiment number 1

simpleoutput.param('outputFileName', 'HepEvtReaderOutput.root')

# creating the path for the processing
main.add_module(eventinfosetter)
main.add_module(progress)

# Add hepevtreader module to path:
main.add_module(hepevtreader)
# and print parameters for hepevtreader on startup of process
basf2.print_params(hepevtreader)

# Add all other modules for simple processing to path
main.add_module(simpleoutput)

# Process the events
basf2.process(main)
# if there are less events in the input file the processing will be stopped at
# EOF.
