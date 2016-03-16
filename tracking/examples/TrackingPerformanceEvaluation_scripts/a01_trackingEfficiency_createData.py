#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 10k Y(4S) events                        #
#                                                               #
#    written by Giulia Casarosa, INFN Pisa                      #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
# USAGE:
#
# > mkdir release/workdir
# > cd release/workdir
# > mkdir /group/belle2/users/casarosa/trackingValidation/release
# > ln -s /group/belle2/users/casarosa/trackingValidation/release .
# > bsub -q s -o data_release_1.out basf2 \
#    ../tracking/examples/TrackingPerformanceEvaluation_scripts/a01_trackingEfficiency_createData.py 1 release
# > bsub -q s -o reco_release_1.out basf2 \
#    ../tracking/examples/TrackingPerformanceEvaluation_scripts/a02_trackingEfficiency_runTracking.py 1 release
# > bsub -q s -o anal_release_1.out basf2 \
#    ../tracking/examples/TrackingPerformanceEvaluation_scripts/a03_trackingEfficiency_createPlots.py 1 release
#
#################################################################

import sys
from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters

set_random_seed(123 + int(sys.argv[1]))

release = sys.argv[2]

print(release)

output_filename = './' + release + '/TV_data_' + release + '_' + sys.argv[1] + '.root'

print(output_filename)

path = create_path()


"""Add needed modules to the path and set parameters and start it"""

# evt meta
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [1])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [2500])

progress = register_module('Progress')

# EvtGen
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

beamparameters = add_beamparameters(path, "Y4S")

# no bkg
background_files = []

# write output root file
root_output = register_module('RootOutput')
root_output.param('outputFileName', output_filename)

path.add_module(eventinfosetter)
path.add_module(progress)
path.add_module(beamparameters)
path.add_module(evtgeninput)
add_simulation(path)
path.add_module(root_output)

process(path)
print(statistics)
