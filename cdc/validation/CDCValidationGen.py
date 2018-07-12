#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>CDCOutputGen.root</output>
<contact>Hiroki Kanda, kanda@m.tains.tohoku.ac.jp</contact>
</header>
"""
from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters
import glob

set_random_seed(12345)

main = create_path()
add_beamparameters(main, "Y4S")

# specify number of events to be generated.
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# detector simulation
bg = None

# if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
#    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, components=['CDC'], bkgfiles=bg)

# Root output
simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', '../CDCOutputGen.root')
main.add_module(simpleoutput)
# generate events
process(main)

# show call statistics
print(statistics)
