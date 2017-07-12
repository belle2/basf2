#!/usr/bin/env python3

# Make sure the default decay files works fine

import basf2
from beamparameters import add_beamparameters
from generators import add_evtgen_generator

# disable database access, we only want to check if the decay files can be
# parsed
basf2.reset_database()
basf2.use_local_database("/dev/null", invertLogging=True)

path = basf2.create_path()
path.add_module("EventInfoSetter", evtNumList=[100])
add_beamparameters(path, "Y4S")
add_evtgen_generator(path, "mixed")
basf2.process(path)
