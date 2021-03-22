#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<output>CDCOutputGen.root</output>
<contact>Hiroki Kanda, kanda@m.tains.tohoku.ac.jp</contact>
</header>
"""
import basf2 as b2
from simulation import add_simulation
from beamparameters import add_beamparameters

b2.set_random_seed(12345)

main = b2.create_path()
add_beamparameters(main, "Y4S")

# specify number of events to be generated.
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# generate BBbar events
evtgeninput = b2.register_module('EvtGenInput')
main.add_module(evtgeninput)

# detector simulation
bg = None

# if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
#    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, components=['CDC'], bkgfiles=bg)

# Root output
simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', '../CDCOutputGen.root')
main.add_module(simpleoutput)
# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
