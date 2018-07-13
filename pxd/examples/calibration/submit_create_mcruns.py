#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script using CAF backends to submit jobs for generating of mc files with PXDSimHits for
# PXD gain calibration.
#
# basf2 submit_create_mcruns.py local
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

import sys
import os
from caf import backends
from pathlib import Path

# Directory for collecting generated files for later usage in PXD gain calibration
output_dir = str(Path('pxd_mc_files').absolute())

# Name for directory for putting all temporary files needed during mc generation
tempdir_name = 'tmp_create_mc'

# List of needed ExpRuns should come from on iov_list matched to the data runs to calibrate
# FIXME: put some hardcoded numbers here for a start.
ExpRuns = [(3, 3360), (3, 3361), (3, 3362)]


def run_job_submission(backend):

    # List of jobs to be submitted
    jobs = []

    for expNo, runNo in ExpRuns:
        for setNo in range(0, 4):

            job = backends.Job("Create_Exp_{}_Run_{}_Set_{}".format(expNo, runNo, setNo))
            job.output_dir = str(Path(tempdir_name, job.name).absolute())
            job.working_dir = str(Path(tempdir_name, job.name).absolute())
            job.cmd = [
                'basf2',
                'create_mcrun.py',
                '--',
                '--expNo={}'.format(expNo),
                '--runNo={}'.format(runNo),
                '--setNo={}'.format(setNo)]

            # Any files appended to this list will be place in the Job's working directory
            job.input_sandbox_files.append("create_mcrun.py")

            # Sometimes you need to configure how the backend will treat your Job but in a backend specific way
            # Any arguments to deal with this should be created in the backend_args dictionary
            # Here we set the batch queue used to submit the job
            if isinstance(backend, backends.LSF):
                job.backend_args['queue'] = 's'
            elif isinstance(backend, backends.PBS):
                job.backend_args['queue'] = 'short'

            # Append to job to jobs
            jobs.append(job)

    # Submit alls jobs at once
    backend.submit(jobs)

    from basf2 import B2INFO
    import time

    # Jobs provide a job.ready() function which returns True once the Job (or all its SubJobs) have finished their running
    ready = False
    while not ready:
        ready = all(job.ready() for job in jobs)
        B2INFO("Not finished all jobs yet")
        time.sleep(5)
    B2INFO("Finished")


def main(backend_choice):
    # Create a backend of our choosing
    if backend_choice == "qsub":
        backend = backends.PBS()
    elif backend_choice == "bsub":
        backend = backends.LSF()
    elif backend_choice == "local":
        backend = backends.Local(max_processes=4)
    else:
        print("How did I get here?")
        sys.exit(1)
    run_job_submission(backend)
    if backend_choice == 'local':
        # Should really be closing down the processing pool once we're done submitting.
        backend.join()

    # Find all output files
    import glob
    output_files = glob.glob(tempdir_name + '/**/beam*.root')

    # Make sure output_dir exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Copy all outputs to output_dir
    import shutil
    for file_name in output_files:
        shutil.move(file_name, output_dir)


if __name__ == "__main__":
    if not (len(sys.argv) == 2) or (sys.argv[1] not in ("local", "qsub", "bsub")):
        print("Usage: python3 using_backends.py <backend_type>")
        print("Where <backend_type> is one of: local, qsub, bsub")
        sys.exit(1)
    main(sys.argv[1])
