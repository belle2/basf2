#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from caf import backends

local = backends.Local(max_processes=4)

from pathlib import Path

##############################
# Simple Job that doesn't do
# anything realted to basf2
##############################
job1 = backends.Job("TestJob1")  # Simple name, mainly for logging
# Where you want your stdout and output_patterns to go (subjobs go in path/0, path/1)
job1.output_dir = str(Path('testing_local', job1.name).absolute())
# Where you want to do your running (subjobs don't respect this and go in path/0, path/1)
job1.working_dir = str(Path('testing_local', job1.name).absolute())
# Bash command to run, notice how it references an environment variable that doesn't yet exist
job1.cmd = ['echo', '$CAF_TEST_ENV']
# Can append more bash commands to the setup which will be run before your main command.
# Here we can create the variable that our main command uses.
job1.setup_cmds.append("CAF_TEST_ENV=Help\n")

###############################
# How to do a basf2 job that
# includes input data files and
# running subjobs over those files
###############################
job2 = backends.Job("TestJob2")
job2.output_dir = str(Path('testing_local', job2.name).absolute())
job2.working_dir = str(Path('testing_local', job2.name).absolute())
job2.cmd = ['basf2', 'basic_basf2.py']
# Central basf2 releases can be set up more easily (you don't need to write your own setup commands)
job2.basf2_release = "release-00-08-00"
job2.basf2_tools = "/sw/belle2/tools/setup_belle2"
# This actually tells the Job to append the correct basf2 setup to the job.setup_cmds. Otherwise it won't do it.
job2.add_basf2_setup()
# Any files appended to this list will be place in the Job's working directory e.g. steering files, modules.
# Since we're running this file we need to include it.
job2.input_sandbox_files.append("basic_basf2.py")
# Input files are special. The Backend checks that they exist (unless you pass a root:// url) and creates paths
# for the ones that do.
# Input files are split between subjobs if there are any.
# Then it creates a pickle file of the valid data files ('input_data_files.data') and places it in the working
# directory of the job/subjob.
# It's your responsibility to write a program that can access this file and use the input data files correctly
job2.input_files = ["test_data/*.root"]
job2.max_files_per_subjob = 1

# You can submit one job at a time, or a list all at once
jobs = [job1, job2]
local.submit(jobs)

from basf2 import B2INFO
import time

# Jobs provide a job.ready() function which returns True once the Job (or all its SubJobs) have finished their running
ready = False
while not ready:
    ready = all(job.ready() for job in jobs)
    B2INFO("Not done yet")
    time.sleep(5)

# This lets you close and join the Pool object. Basically means "Wait for all jobs running on this backend to finish
# and don't let anyone run any more"
local.join()
B2INFO("Finished")
