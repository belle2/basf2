#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from caf import backends

pbs = backends.PBS()
pbs.release = "/coepp/cephfs/mel/ddossett/software/release"
pbs.queue = "short"

from pathlib import Path

job1 = backends.Job("TestJob1")
job1.output_dir = str(Path('testing_pbs', job1.name).absolute())
job1.working_dir = str(Path('testing_pbs', job1.name).absolute())
job1.cmd = ['echo', '$CAF_TEST_ENV']
job1.setup_cmds.append("CAF_TEST_ENV=Help\n")
job1.add_basf2_setup()
job1.queue = "short"
job1.input_sandbox_files.append("caf_vxd.py")
job1.input_files = ["test_data/*.root"]
job1.max_files_per_subjob = 3

job2 = backends.Job("TestJob2")
job2.output_dir = str(Path('testing_pbs', job2.name).absolute())
job2.working_dir = str(Path('testing_pbs', job2.name).absolute())
job2.cmd = ['echo', '$CAF_TEST_ENV']
job2.setup_cmds.append("CAF_TEST_ENV=Help\n")
job2.add_basf2_setup()
job2.queue = "short"
job2.input_sandbox_files.append("caf_vxd.py")
job2.input_files = ["test_data/*.root"]
job2.max_files_per_subjob = -1

jobs = [job1, job2]
pbs.submit(jobs)

from basf2 import B2INFO
import time

ready = False
while not ready:
    ready = all(job.ready for job in jobs)
    B2INFO("Not done yet")
    time.sleep(15)

B2INFO("Finished")
