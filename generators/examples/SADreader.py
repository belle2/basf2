#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import set_log_level, LogLevel, register_module, create_path, process, statistics

# Set the global log level
set_log_level(LogLevel.ERROR)

# Register the event meta generator and set the number of events to a very
# high number which exceeds the number of events in the input file.
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10000000], 'runList': [1]})

# Register the SADInput module and specify the location of the SAD input file.
# The file can be downloaded from the TWiki.
sadinput = register_module('SADInput')
sadinput.param('Filename', 'SADreaderInput.root')

# Set the ReadMode of the SAD input module. 0 = one SAD particle per event.
# This produces weighted events. 1 = one real particle per event. This produces
# unweighted events. 2 = all SAD particles in one event. Can be used for
# visualization.
sadinput.param('ReadMode', 1)

# Set the accelerator ring with which the SAD input has been generated. 0 = LER
# 1 = HER.
sadinput.param('AccRing', 0)

# Set the readout time for your subdetector in [ns]. Please note: The value
# given here corresponds to the readout time of the PXD ! This setting is only
# used if the 'ReadMode' is set to 1.
sadinput.param('ReadoutTime', 20000)

# Set the range around the IP in which SAD particles are accepted into the
# simulation. The value given below is highly recommended.
sadinput.param('Range', 390)

# If you would like to see some information about the created particles set the
# logging output of the Input module to DEBUG.
sadinput.set_log_level(LogLevel.DEBUG)

# Register the standard chain of modules to the framework, which are required
# for the simulation.
gearbox = register_module('Gearbox')

geometry = register_module('Geometry')

fullsim = register_module('FullSim')

# Add additional modules according to your own needs
# pxddigi = register_module('PXDDigitizer')
# progress  = register_module('Progress')

# Write the output to a file
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', 'SADreaderOutput.root')

progress = register_module('Progress')

# Create the main path and add the required modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(sadinput)
main.add_module(geometry)
main.add_module(fullsim)

# Add additional modules if you like main.add_module(pxddigi)
main.add_module(progress)

# Add the output module
main.add_module(rootoutput)

# Start the event processing
process(main)

# Print some basic event statistics
print('Event Statistics:')
print(statistics)
