#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    set of scripts to analyse  Y(4S) events                    #
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
# > ln -s release/tracking/examples/TrackingPerformanceEvaluation_scripts/a*.py
# > bsub -q s -o data_release.out basf2 a01_trackingEfficiency_createData.py release
# > bsub -q s -o simul_roi_bkg_release.out basf2 a02_trackingEfficiency_simulateData.py release roi bkg
# > bsub -q s -o reco_roi_bkg_vxdtf_release.out basf2 a03_trackingEfficiency_runTracking.py release roi bkg vxdtf
# > bsub -q s -o anal_roi_bkg_vxdtf_release.out basf2 a04_trackingEfficiency_createPlots.py release roi bkg vxdtf
#
#################################################################

import sys
from basf2 import *
from simulation import add_simulation
from beamparameters import add_beamparameters

set_random_seed(1509)

release = sys.argv[1]

print(release)

output_filename = './' + release + '/TV_data_' + release + '.root'

print(output_filename)

path = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10000])

progress = register_module('Progress')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

beamparameters = add_beamparameters(path, "Y4S")

root_output = register_module('RootOutput')
root_output.param('outputFileName', output_filename)

path.add_module(eventinfosetter)
path.add_module(progress)
path.add_module(beamparameters)
path.add_module(evtgeninput)
path.add_module(root_output)

process(path)
print(statistics)
