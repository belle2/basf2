#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')
output = register_module('SimpleOutput')

# Set module parameters
param_evtmetagen = {
    'ExpList': [1, 1],
    'RunList': [1, 2],
    'EvtStartList': [1, 101],
    'EvtEndList': [100, 200],
    }
evtmetagen.param(param_evtmetagen)

output.param('outputFileName', 'sample.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(output)

# Process events
process(main)
