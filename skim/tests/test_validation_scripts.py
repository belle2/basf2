#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
Check if the validation scripts can be executed using basf2 with "--dry-run".
'''

import basf2
import b2test_utils as b2u
import argparse
import glob
import os
import subprocess as sp
import multiprocessing as mp
from concurrent.futures import ProcessPoolExecutor

__authors__ = ['Giacomo De Pietro']


def arg_parser():
    '''
    A very simple argument parser.
    '''
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-j', '--jobs',
                        default=None,
                        type=int,
                        help='Number of workers to be used by ProcessPoolExecutor.',
                        metavar='JOBS')
    return parser


def dry_run(validation_file):
    '''
    Check if the steering file at the given path can be run with the "--dry-run" option.
    '''
    result = sp.call(['basf2', validation_file, '--dry-run', '-i', 'input.root', '-o', 'output.root', '-l', 'ERROR'])
    return (result, validation_file)


if __name__ == '__main__':
    # Skip this test for light releases.
    b2u.skip_test_if_light()
    # First, let's check in a smart way how many cores we can use (or override it).
    args = arg_parser().parse_args()
    num_workers = None
    if args.jobs is None:
        num_workers = max(int(mp.cpu_count() / (1. + os.getloadavg()[1])), 1)
    else:  # Ok, we override the number of workers...
        num_workers = args.jobs
    basf2.B2INFO(f'The test will be executed using {num_workers} workers.')
    # Then, let's run the test!
    validation_path = basf2.find_file('skim/validation/')
    failed_files = []
    with b2u.clean_working_directory():
        with ProcessPoolExecutor(max_workers=num_workers) as pool:
            for result, validation_file in pool.map(dry_run, glob.glob(f'{validation_path}*.py')):
                if not result == 0:
                    failed_files.append(f'skim/validation/{os.path.basename(validation_file)}')
        if len(failed_files) > 0:
            basf2.B2FATAL('The following validation scripts in the skim package failed, check them:\n'
                          + ' \n'.join(failed_files))
