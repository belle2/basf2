#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys
from ROOT import Belle2

set_log_level(LogLevel.INFO)

if len(sys.argv) != 6:
    print('Usage: basf2 1_generate.py data_output_directory experiment_number run_number_low run_number_high num_events')
    sys.exit(1)

output_dir = sys.argv[1]
experiment = int(sys.argv[2])
run_low = int(sys.argv[3])
run_high = int(sys.argv[4])
nevents = int(sys.argv[5])

runs = [run for run in range(run_low, run_high+1)]
print(runs)

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment]*len(runs), runList=runs, evtNumList=[nevents]*len(runs))
main.add_module('RootOutput',
                outputFileName=output_dir + '/DST_exp{:d}_run{:d}_run{:d}.root'.format(experiment, run_low, run_high))
process(main)
print(statistics)
