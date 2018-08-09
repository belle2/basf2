#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)

if len(sys.argv) < 5:
    print('Usage: basf2 1_generate.py data_output_directory experiment_number run_number num_events')
    sys.exit(1)

output_dir = sys.argv[1]
experiment = int(sys.argv[2])
run = int(sys.argv[3])
nevents = int(sys.argv[4])

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment], runList=[run], evtNumList=[nevents])
main.add_module('RootOutput', outputFileName=output_dir + '/DST_exp{:d}_run{:d}.root'.format(experiment, run))
main.add_module('Progress')
process(main)
print(statistics)
