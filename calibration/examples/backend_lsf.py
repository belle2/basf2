#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from caf import backends

lsf = backends.LSF()

from pathlib import Path

job1 = backends.Job("TestJob1")
job1.output_dir = str(Path('testing_lsf', job1.name).absolute())
job1.working_dir = str(Path('testing_lsf', job1.name).absolute())
job1.cmd = ['echo', '$CAF_TEST_ENV']
job1.setup_cmds.append("CAF_TEST_ENV=Help\n")
job1.basf2_release = "/home/belle/ddossett/release"
job1.basf2_tools = "/sw/belle2/tools/setup_belle2"
job1.add_basf2_setup()
job1.queue = "s"
job1.input_sandbox_files.append("caf_vxd.py")
job1.input_files = ["test_data/*.root"]
job1.max_files_per_subjob = 3

job2 = backends.Job("TestJob2")
job2.output_dir = str(Path('testing_lsf', job2.name).absolute())
job2.working_dir = str(Path('testing_lsf', job2.name).absolute())
job2.cmd = ['echo', '$CAF_TEST_ENV']
job2.setup_cmds.append("CAF_TEST_ENV=Help\n")
job2.basf2_release = "/home/belle/ddossett/release"
job2.basf2_tools = "/sw/belle2/tools/setup_belle2"
job2.add_basf2_setup()
job2.queue = "s"
job2.input_sandbox_files.append("caf_vxd.py")
job2.input_files = ["test_data/*.root"]
job2.max_files_per_subjob = -1

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
