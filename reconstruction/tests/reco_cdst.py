#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test more or less everything.
'''

import basf2
import b2test_utils as b2u
import argparse
import glob
import os
import subprocess as sp
import multiprocessing as mp
import concurrent.futures as cf
import functools as ft


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


def steer_this(steering_file, rawdata_file):
    '''
    Execute the steering file over the given input rawdata file.
    '''
    if 'cosmic' in os.path.basename(rawdata_file):
        basf2.B2INFO(f'Running the test using {rawdata_file} as input file.')
        return sp.call(['basf2', steering_file, rawdata_file, 'cosmic'])
    elif 'physics' in os.path.basename(rawdata_file):
        basf2.B2INFO(f'Running the test using {rawdata_file} as input file.')
        return sp.call(['basf2', steering_file, rawdata_file, 'physics'])
    else:
        basf2.B2INFO('Not a "cosmic" neither a "physics" run, skipping it.')
        return 0


if __name__ == '__main__':
    # Skip this test if needed.
    if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
        b2u.skip_test('BELLE2_VALIDATION_DATA_DIR environment variable not set, skipping the test.')
    # First, let's check in a smart way how many cores we can use (or override it).
    args = arg_parser().parse_args()
    num_workers = None
    if args.jobs is None:
        num_workers = max(int(mp.cpu_count() / (1. + os.getloadavg()[1])), 1)
    else:  # Ok, we override the number of workers...
        num_workers = args.jobs
    basf2.B2INFO(f'The test will be executed using {num_workers} workers.')
    num_workers = max(int(mp.cpu_count() / (1. + os.getloadavg()[1])), 1)
    # Then, let's run the test!
    steering_file = basf2.find_file('reconstruction/tests/reco_cdst.py_noexec')
    rawdata_files = glob.glob(os.environ['BELLE2_VALIDATION_DATA_DIR'] + '/rawdata/*HLT?.*.root')
    with cf.ProcessPoolExecutor(max_workers=num_workers) as pool:
        for result in pool.map(ft.partial(steer_this, steering_file), rawdata_files):
            assert(result == 0)
