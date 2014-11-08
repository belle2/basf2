#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *
import os
import sys

# suppress messages and during processing:
set_log_level(LogLevel.WARNING)

# set event info for generated events
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])  # we want to process 100 events

# load parameters (i.e. beam energies)
gearbox = register_module('Gearbox')
# Find the path to this script so it also works when called from another
# directory
path = os.path.dirname(sys.argv[0])
# Look for xml files also in the current directory for files beginning with
# Upsilon1S (so geometry/SuperKEKB.xml will also be found as
# Upsilon1S-geometry-SuperKEKB.xml)
prefix = os.path.join(path, "Upsilon1S")
gearbox.param("backends", ["file:%s" % prefix, "file:"])

# to generate other parent particles with different energies one would need to
# make a copy of Upsilon1S-geometry-SuperKEKB.xml and change the beam energies
# to match the target energy, save it using a suitable prefix and ending in
# "-geometry-SuperKEKB.xml" and change the prefix above to match that prefix

# one can set the loglevel of gearbox to debug which will cause it to print
# which files are opened to check if the correct SuperKEKB.xml is used
#gearbox.logging.log_level = LogLevel.DEBUG

# configure evtgen to generate Upsilon as top particle
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', "Upsilon")

# print generated particles
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO

# creating the path for the processing
main = create_path()
# and all modules we created
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(evtgeninput)
main.add_module(mcparticleprinter)

# process the events
process(main)
