#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from optparse import OptionParser

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
Use
%prog --options
to see available drawing options
"""

parser = OptionParser(usage=usage)

# Setup options with matching names like in the display module
svgdisplay = cdcdisplay.CDCSVGDisplayModule('/tmp')
for option in svgdisplay.drawoptions:
    options_flag = '--' + option.replace('_', '-')
    parser.add_option(options_flag, dest=option, action='store_true',
                      default=getattr(svgdisplay, option))

parser.add_option('--options', dest='show_options', action='store_true',
                  default=False)

(options, args) = parser.parse_args()

try:
    if options.show_options:
        print 'Available drawing options are:'
        drawoptions = svgdisplay.drawoptions
        drawoptions.sort()
        for option in drawoptions:
            print '--' + option.replace('_', '-')
        sys.exit(0)
except AttributeError:
    pass

for option in svgdisplay.drawoptions:
    try:
        is_active_option = getattr(options, option)
    except AttributeError:
        continue
    else:
        print 'Setting', option, 'to', is_active_option
        setattr(svgdisplay, option, is_active_option)

# Check if an input file has been given that shall be used as ROOT input
if args:
    simulated_events_file = args[0]
    if not os.path.exists(simulated_events_file):
        print 'The given input file', simulated_events_file, 'does not exist.'
        sys.exit(1)
    use_simulation = False
else:
    use_simulation = True

from basf2 import *
import simulation

set_log_level(LogLevel.INFO)

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
    geometry.param('components', components)
    main.add_module(geometry)

# Add the tracking module
cdclocaltracking = register_module('CDCLocalTracking')
main.add_module(cdclocaltracking)

# Maybe view other track finders
# trasan = register_module('Trasan')
# trasan.param('DebugLevel', 10)
# trasan.param('GFTrackCandidatesColName', 'GFTrackCands')

# cdclegendretracking = register_module('CDCLegendreTracking')

# param_cdclegendretracking = {
#     'GFTrackCandidatesColName': 'GFTrackCands',
#     'Threshold': 10,
#     'InitialAxialHits': 48,
#     'MaxLevel': 10,
#     'StepScale': 0.75,
#     'Resolution StereoHits': 2.,
#     'Reconstruct Curler': True,
#     }

# cdclegendretracking.param(param_cdclegendretracking)

## ---------------------------------------------------------------
## MC finder
# mctrackfinder = register_module('TrackFinderMCTruth')
# param_mctrackfinder = {
#     'UseCDCHits': 1,
#     'UseSVDHits': 1,
#     'UsePXDHits': 1,
#     'WhichParticles': ['primary'],
#     'EnergyCut': 0.1,
#     'Neutrals': 0,
#     'GFTrackCandidatesColName': 'GFTrackCands',
#     }

# mctrackfinder.param(param_mctrackfinder)

# if options.mcTracking:
#     main.add_module(mctrackfinder)
# elif (options.trasan):
#    main.add_module(trasan)
# elif options.legendreTracking:
#     main.add_module(cdclegendretracking)

# Print the current content of the DataStore
printCollections = register_module('PrintCollections')
main.add_module(printCollections)

# Add the display module as the last module in the chain
main.add_module(svgdisplay)

process(main)

# Print call statistics
print statistics

