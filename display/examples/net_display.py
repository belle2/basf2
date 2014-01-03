#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Example to listen on port 1111 for events and show them (without blocking)
# Start this script first and run basf2 display/examples/net_display_send.py

# Note that this requires the software in daq/dataflow to be build.
# It is recommended NOT to run this example on a computer that exposes this
# port to the internet, it listens on all interfaces and cannot verify the
# data it receives.

import os
import random
from basf2 import *


rx = register_module('RxSocket')
#default
#rx.param('Port', 1111)

gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
# Since Geometry is only required for track extrapolation in inner detectors,
# we'll exclude ECL (saves about 10s in startup time)
geometry.param('excludedComponents', ['ECL'])


# create paths
main = create_path()

# add modules to paths
main.add_module(rx)

main.add_module(gearbox)
main.add_module(geometry)

# default parameters
display = register_module('AsyncDisplay')
display.param('showAllPrimaries', True)
main.add_module(display)

process(main)
print statistics
