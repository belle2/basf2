#!/usr/bin/env python
# -*- coding: utf-8 -*-

#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# Run PYTHIA on unfragmented final states
#
# Example steering file
########################################################

from basf2 import *
from ROOT import Belle2

# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)

# event info setter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# add root input module (if kkmc is not used)
rootinput = register_module('RootInput')
rootinput.param('inputFileName', './continuum.root')

# get the gearbox
gearbox = register_module('Gearbox')

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# add fragmentation module
fragmentation = register_module('Fragmentation')
fragmentation.param('ParameterFile', '../modules/fragmentation/data/pythia_belle2.dat')

# add root output module
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', './fragmented_output.root')

# main
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(rootinput)
main.add_module(fragmentation)
main.add_module(rootoutput)
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)
process(main)
