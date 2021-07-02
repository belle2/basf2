#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Script using CAF backends to submit jobs for generating of mc files with PXDSimHits for
# PXD gain calibration.
#
# basf2 submit_create_mcruns.py -- --backend='local' --outputdir='pxd_mc_phase2' --runLow=4000 --runHigh=6522 --expNo=3
#
# author: benjamin.schwenker@phys.uni-goettingen.de

import sys
import os
import pickle
from caf import backends
from pathlib import Path
from caf.utils import IoV

#: Name for directory for putting all temporary files needed during mc generation
_tempdir_name = 'tmp_create_mc'

#: Number of mc mixer sets to draw from
_number_of_sets = 10

#: Name of GT to be used for mc generation
_tag = 'Calibration_Offline_Development'

#: Path to mixer files
_bg = '/group/belle2/BGFile/OfficialBKG/15thCampaign/phase2'


def run_job_submission(backend, mc_iov_list):

    # List of jobs to be submitted
    jobs = []

    for mc_iov in mc_iov_list:
        for setNo in range(0, _number_of_sets):
            expNo = mc_iov.exp_low
            runNo = mc_iov.run_low
            job = backends.Job("Create_Exp_{}_Run_{}_Set_{}".format(expNo, runNo, setNo))
            job.output_dir = str(Path(_tempdir_name, job.name).absolute())
            job.working_dir = str(Path(_tempdir_name, job.name).absolute())
            job.cmd = [
                'basf2',
                'create_mcrun.py',
                '--',
                '--tag={}'.format(_tag),
                '--bg={}'.format(_bg),
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


def main(backend_choice, mc_iov_list, output_dir):
    # Create a backend of our choosing
    if backend_choice == "qsub":
        backend = backends.PBS()
    elif backend_choice == "bsub":
        backend = backends.LSF()
    elif backend_choice == "local":
        backend = backends.Local(max_processes=20)
    else:
        print("How did I get here?")
        sys.exit(1)
    run_job_submission(backend, mc_iov_list)
    if backend_choice == 'local':
        # Should really be closing down the processing pool once we're done submitting.
        backend.join()

    # Find all output files
    import glob
    output_files = glob.glob(str(Path(_tempdir_name).absolute()) + '/**/beam*.root')

    # Copy all outputs to output_dir
    import shutil
    for file_name in output_files:
        shutil.move(file_name, output_dir)


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Submit jobs for creattion of SimHits for a run with user specified ExpRuns")
    parser.add_argument('--outputdir', default='pxd_mc_phase2', type=str,
                        help='Add all simulated files to this folder. Create it if it does not exist.')
    parser.add_argument('--backend', default='local', type=str, help='Where <backend> is one of: local, qsub, bsub.')
    parser.add_argument('--runLow', default=0, type=int, help='Compute mask for specific IoV')
    parser.add_argument('--runHigh', default=-1, type=int, help='Compute mask for specific IoV')
    parser.add_argument('--expNo', default=3, type=int, help='Compute mask for specific IoV')
    args = parser.parse_args()

    # Directory for collecting generated files for later usage in PXD gain calibration
    output_dir = str(Path(args.outputdir).absolute())

    # Make sure output_dir exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Set the IoV range for mc calibration
    iov_to_calibrate = IoV(exp_low=args.expNo, run_low=args.runLow, exp_high=args.expNo, run_high=args.runHigh)

    # Access files_to_iovs for beam runs
    with open("file_iov_map.pkl", 'br') as map_file:
        files_to_iovs = pickle.load(map_file)

    # Set of all IoVs for beam runs (data). Potentially very, very large.
    data_iov_set = set(files_to_iovs.values())

    # List of IoV for mc runs to be submitted to backends
    mc_iov_list = []
    for data_iov in data_iov_set:
        if iov_to_calibrate.contains(data_iov):
            mc_iov_list.append(data_iov)

    print('Number selected iovs for mc generation :  {}'.format(len(mc_iov_list)))

    main(args.backend, mc_iov_list, output_dir)
