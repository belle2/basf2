#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from pathlib import Path
import time

# Add timestamp to all INFO messages
from basf2 import B2INFO, logging, LogLevel, LogInfo
currentInfo = logging.get_info(LogLevel.INFO)
logging.set_info(LogLevel.INFO, currentInfo | LogInfo.TIMESTAMP)

from caf import backends
import ROOT

# we'll use some of your config here to set up the job directories
import re
with open('location') as config_file:
    lines = config_file.readlines()
    for line in lines:
        if re.match(r'dir_data', line):
            data_dir = line.split('"')[1]
        if re.match(r'dir_root', line):
            root_dir = line.split('"')[1]
'''
runs = []
with open('runlist') as runlist:
    lines = runlist.readlines()
    for line in lines:
        runs.append('cr.' + line.rstrip() + '.root')
'''

###############################
# How to do a basf2 job that
# includes input data files and
# running subjobs over those files
###############################
job1 = backends.Job("CDC_Colllector")
job1.output_dir = str(Path(root_dir).absolute())
job1.working_dir = str(Path(root_dir).absolute())

# This is the command that gets run inside the batch job
job1.cmd = ['basf2', 'run_collector.py']
# Need to copy in the steering file so that each subjob has access to it in its working dir
job1.input_sandbox_files.append("run_collector.py")
# Your config file may be necessary for the run_collector.py in each job, so we copy it to the working directory
# job1.input_sandbox_files.append("location")
# You can either create a list of input files manually e.g. with glob
import glob
input_files = glob.glob(data_dir + "/cr*.root")
# input_files = [data_dir + f for f in runs]
# This lets us reduce the number of input files for testing purposes
# job1.input_files = input_files[:5]
job1.input_files = input_files
# Or you could just give the wildcard expansions directly in the input files of the job
# job1.input_files = [data_dir+"/cr*.root"]

# You can choose how many input files to send to each subjob (-1 means all to one main job)
job1.max_files_per_subjob = 1
# Choose your queue
job1.backend_args['queue'] = 's'
##################################

# Here we submit the job to the batch system. Subjobs for the input data files are automatically created
lsf = backends.LSF()
lsf.submit(job1)
# Here we start waiting for the overall job to complete
ready = False
while not ready:
    B2INFO("Are we done?")
    ready = job1.ready()
    B2INFO("Not done yet, will sleep")
    time.sleep(60)

B2INFO("Finished")
