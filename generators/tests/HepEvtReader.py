#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

# find the input file
input_file = b2.find_file("generators/examples/BhWide_10events.txt")

# prepare the path
path = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [0])  # and experiment number 1
path.add_module(eventinfosetter)

# configure the Hepevt input module
hepevtinput = b2.register_module('HepevtInput')
hepevtinput.param('inputFileList', [input_file])
hepevtinput.param('createEventMetaData', False)  # Don't create EventMetaData to test fallback
path.add_module(hepevtinput)
b2.print_params(hepevtinput)

# process
b2.process(path)

# test that m_mcEvents was set correctly in Environment
env = Belle2.Environment.Instance()
mc_events = env.getNumberOfMCEvents()
is_mc = env.isMC()

# The file contains 10 events
assert mc_events == 10, f'Expected 10 MC events, got {mc_events}'
assert is_mc, f'Expected isMC() to return True, got {is_mc}'
