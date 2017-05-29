#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from pathlib import Path
import time

from basf2 import B2INFO
from caf import backends
import ROOT

lsf = backends.LSF()

##############################
# Simple Job that doesn't do
# anything realted to basf2
##############################
job1 = backends.Job("TestJob1")
job1.output_dir = str(Path('testing_lsf', job1.name).absolute())
job1.working_dir = str(Path('testing_lsf', job1.name).absolute())
job1.cmd = ['echo', '$CAF_TEST_ENV']
job1.setup_cmds.append("CAF_TEST_ENV=Help")

###############################
# How to do a basf2 job that
# includes input data files and
# running subjobs over those files
###############################
job2 = backends.Job("TestJob2")
job2.output_dir = str(Path('testing_lsf', job2.name).absolute())
job2.working_dir = str(Path('testing_lsf', job2.name).absolute())

# job2.basf2_release = "release-00-08-00"
# job2.basf2_tools = "/sw/belle2/tools/setup_belle2"
job2.add_basf2_setup()

job2.cmd = ['basf2', 'basic_basf2.py']
job2.input_sandbox_files.append("basic_basf2.py")
# Checking that the example data directory exists, if not then we shouldn't continue
input_data_path = ROOT.Belle2.FileSystem.findFile('calibration/examples/test_data')
if not input_data_path:
    B2INFO("Missing necessary input data. Please run the calibration/examples/ scripts")
    sys.exit(1)
job2.input_files = ["test_data/*.root"]
job2.max_files_per_subjob = 1
job2.backend_args['queue'] = 's'

##################################

jobs = [job1, job2]
lsf.submit(jobs)

from basf2 import B2INFO
import time

ready = False
while not ready:
    ready = all(job.ready for job in jobs)
    B2INFO("Not done yet")
    time.sleep(15)

B2INFO("Finished")
