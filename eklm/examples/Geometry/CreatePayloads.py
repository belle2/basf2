#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM geometry payloads.

import os
import random
from basf2 import *

set_log_level(LogLevel.INFO)

eventinfosetter = register_module('EventInfoSetter')

# Gearbox
gearbox = register_module('Gearbox')

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['EKLM'])
geometry.param('useDB', False)
geometry.param('createPayloads', True)
geometry.param('payloadIov', [0, 0, -1, -1])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)

process(main)
