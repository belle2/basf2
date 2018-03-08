#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
from caf import backends


def run_job_submission(backend):
    from pathlib import Path

    ##############################
    # Simple Job that doesn't do
    # anything realted to basf2
    ##############################
    job1 = backends.Job("TestJob1")  # Simple name, mainly for logging
    # Where you want your stdout and output_patterns to go (subjobs go in path/0, path/1)
    job1.output_dir = str(Path('testing', job1.name).absolute())
    # Where you want to do your running (subjobs don't respect this and go in path/0, path/1)
    job1.working_dir = str(Path('testing', job1.name).absolute())
    # Bash command to run, notice how it references an environment variable that doesn't yet exist
    job1.cmd = ['echo', '$CAF_TEST_ENV']
    # Can append more bash commands to the setup which will be run before your main command.
    # Here we can create the variable that our main command uses.
    job1.setup_cmds.append("CAF_TEST_ENV=Help")

    ###############################
    # How to do a basf2 job that
    # includes input data files and
    # running subjobs over those files
    ###############################
    job2 = backends.Job("TestJob2")
    job2.output_dir = str(Path('testing', job2.name).absolute())
    job2.working_dir = str(Path('testing', job2.name).absolute())
    job2.cmd = ['basf2', 'basic_basf2.py']
    # Central basf2 releases can be set up more easily (you don't need to write your own setup commands)
    # job2.basf2_release = "release-00-08-00"
    # job2.basf2_tools = "/sw/belle2/tools/setup_belle2"
    # This actually tells the Job to append the current central basf2 setup to the job2.setup_cmds. Otherwise it won't do it.
    # job2.add_basf2_setup()
    # Or you can set up a local release and your own commands by setting the commands manually
    job2.setup_cmds = [
        "CAF_TOOLS_LOCATION=/home/ddossett/Melbourne-Work/software/tools/setup_belle2",
        "CAF_RELEASE_LOCATION=/home/ddossett/Melbourne-Work/software/release",
        "source $CAF_TOOLS_LOCATION",
        "pushd $CAF_RELEASE_LOCATION",
        "setuprel",
        "popd"
    ]
    # Any files appended to this list will be place in the Job's working directory e.g. steering files, modules.
    # Since we're running this file we need to include it.
    job2.input_sandbox_files.append("basic_basf2.py")
    # Input files are special. The Backend checks that they exist (unless you pass a root:// url) and remembers the file paths
    # for the ones that do.
    # Input files are split between subjobs if requested.
    # Then it creates a pickle file of the valid data files ('input_data_files.data') and places it in the working
    # directory of the job/subjob.
    # It's your responsibility to write a steering file that can access this file and use the input data files correctly
    # Check the basic_basf2.py file to see how to pull in the data
    job2.input_files = ["../test_data/*.root"]
    job2.max_files_per_subjob = 1  # This could be set to -1 to disallow splitting input data files into subjobs (default)
    # Sometimes you need to configure how the backend will treat your Job but in a backend specific way
    # Any arguments to deal with this should be created in the backend_args dictionary
    # Here we set the batch queue used to submit the job
    if isinstance(backend, backends.LSF):
        job2.backend_args['queue'] = 's'
    elif isinstance(backend, backends.PBS):
        job2.backend_args['queue'] = 'short'
    # You can submit one job at a time, or a list all at once
    # backend.submit(job1)
    jobs = [job1, job2]
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
        backend = backends.Local(max_processes=2)
    else:
        print("How did I get here?")
        sys.exit(1)
    run_job_submission(backend)
    if backend_choice == 'local':
        # Should really be closing down the processing pool once we're done submitting.
        backend.join()


if __name__ == "__main__":
    if not (len(sys.argv) == 2) or (sys.argv[1] not in ("local", "qsub", "bsub")):
        print("Usage: python3 using_backends.py <backend_type>")
        print("Where <backend_type> is one of: local, qsub, bsub")
        sys.exit(1)
    main(sys.argv[1])
