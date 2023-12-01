#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys

b2.set_log_level(b2.LogLevel.INFO)

if len(sys.argv) != 6:
    print('Usage: basf2 generate_test_data.py data_output_directory experiment_number run_number_low run_number_high num_events')
    sys.exit(1)

output_dir = sys.argv[1]
experiment = int(sys.argv[2])
run_low = int(sys.argv[3])
run_high = int(sys.argv[4])
nevents = int(sys.argv[5])

runs = [run for run in range(run_low, run_high+1)]
print(runs)

main = b2.create_path()
main.add_module('EventInfoSetter', expList=[experiment]*len(runs), runList=runs, evtNumList=[nevents]*len(runs))
main.add_module('RootOutput',
                outputFileName=output_dir + f'/DST_exp{experiment:d}_run{run_low:d}_run{run_high:d}.root')
b2.process(main)
print(b2.statistics)
