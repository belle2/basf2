#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from optparse import OptionParser

from basf2 import *
set_log_level(LogLevel.INFO)

import simulation

# Add the path of the svg display module to the search path of python
belle2_local_dir = os.environ['BELLE2_LOCAL_DIR']
cdcdisplay_module_path = os.path.join(belle2_local_dir, 'tracking',
                                      'cdcLocalTracking', 'python_code')
sys.path.append(cdcdisplay_module_path)
import cdcdisplay

# Setup the options parser
usage = \
    """Usage:
basf2 %prog -- [options] input_tracked_events_filename
%prog [options] input_tracked_events_filename
"""

parser = OptionParser(usage=usage)

(options, args) = parser.parse_args()

# Check if an input file has been given that shall be used as ROOT input
if args:
    simulated_events_file = args[0]
    if not os.path.exists(simulated_events_file):
        print 'The given input file', simulated_events_file, 'does not exist.'
        sys.exit(1)
    use_simulation = False
else:
    use_simulation = True

# set_random_seed(12345)

components = ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD',
              'CDC']

main = create_path()

if use_simulation:
    # Evtgen info setter
    # Specify number of events to be generated in job
    eventinfosetter = register_module('EventInfoSetter')
    eventinfosetter.param('evtNumList', [10])  # we want to process nOfEvents events
    eventinfosetter.param('runList', [1])  # from run number 1
    eventinfosetter.param('expList', [1])  # and experiment number 1
    main.add_module(eventinfosetter)

    # Show progress of processing
    progress = register_module('Progress')
    main.add_module(progress)

    # Evtgen generator
    evtgen = register_module('EvtGenInput')
    evtgen.param('boost2LAB', True)
    main.add_module(evtgen)

    # Standard simulation
    simulation.add_simulation(main, components=components)
else:

    rootInput = register_module('RootInput')
    rootInput.param('inputFileName', simulated_events_file)
    main.add_module(rootInput)

    gearbox = register_module('Gearbox')
    main.add_module(gearbox)

    geometry = register_module('Geometry')
    geometry.param('Components', components)
    main.add_module(geometry)

# Add the display module as the last module in the chain
svgdisplay = cdcdisplay.CDCSVGDisplayModule('/tmp')
svgdisplay.draw_wires = True
svgdisplay.draw_hits = True

svgdisplay.draw_superlayer_boundaries = True
svgdisplay.draw_interactionpoint = True

svgdisplay.draw_mcparticle_id = False
svgdisplay.draw_mcparticle_pdgcodes = False
svgdisplay.draw_mcparticle_primary = False

svgdisplay.draw_mcsegments = False

svgdisplay.draw_simhits = False
svgdisplay.draw_simhit_tof = False
svgdisplay.draw_simhit_posflag = False
svgdisplay.draw_simhit_pdgcode = False

svgdisplay.draw_connect_tof = False

svgdisplay.draw_rlinfo = False
svgdisplay.draw_reassigned = False

main.add_module(svgdisplay)

process(main)

# Print call statistics
print statistics

