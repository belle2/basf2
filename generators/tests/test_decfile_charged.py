#!/usr/bin/env python3

# Make sure the default decay files works fine

import basf2
from beamparameters import add_beamparameters
from generators import add_evtgen_generator

path = basf2.create_path()
add_beamparameters(path, "Y4S")
path.add_module("EventInfoSetter", evtNumList=[100])
add_evtgen_generator(path, "charged")
basf2.process(path)
