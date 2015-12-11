#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output

set_log_level(LogLevel.ERROR)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10], 'runList': [1]})
main.add_module(eventinfosetter)

add_simulation(main)
add_reconstruction(main)

eclpuredigi = register_module('ECLDigitizerPureCsI')
eclpuredigi.param('Calibration', 1)
main.add_module(eclpuredigi)

add_mdst_output(main, additionalBranches=['ECLDspsPureCsI'])
progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
