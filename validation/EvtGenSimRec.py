#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('evtNumList', [100])
evtmetagen.param('runList', [1])
evtmetagen.param('expList', [1])
main.add_module(evtmetagen)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
main.add_module(evtgeninput)

# detector simulation
bg = []
if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
    bg += glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# output
output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimRec.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
