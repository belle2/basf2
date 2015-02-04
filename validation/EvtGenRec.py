#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <input>EvtGenSim.root</input>
  <output>EvtGenRec.root,EvtGenmdst.root</output>
  <contact>tkuhr</contact>
  <description>This steering file run the standard reconstruction on an input file with generic BBbar events.</description>
</header>
"""

from basf2 import *
from reconstruction import add_reconstruction, add_mdst_output
import glob

set_random_seed(12345)

main = create_path()

# read file of simulated events
input = register_module('RootInput')
input.param('inputFileName', '../EvtGenSim.root')
main.add_module(input)

# geometry parameter database
main.add_module(register_module('Gearbox'))

# detector geometry
main.add_module(register_module('Geometry'))

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module(register_module('Profile'))

# output
output = register_module('RootOutput')
output.param('outputFileName', '../EvtGenRec.root')
main.add_module(output)
add_mdst_output(main, True, '../EvtGenmdst.root')

process(main)

# Print call statistics
print statistics

from validation import *
statistics_plots('EvtGenRec_statistics.root', contact='tkuhr',
                 jobDesc='a standard reconstruction job with generic EvtGen events',
                 prefix='EvtGenRec'
                 )
event_timing_plot('../EvtGenRec.root', 'EvtGenRec_statistics.root',
                  contact='tkuhr',
                  jobDesc='a standard reconstruction job with generic EvtGen events',
                  prefix='EvtGenRec'
                  )
