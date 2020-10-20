#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
Check if the validation scripts can be executed using basf2 with "--dry-run".
'''

import basf2
import glob
import os
import multiprocessing as mp
from concurrent.futures import ProcessPoolExecutor

__authors__ = ['Giacomo De Pietro']


def dry_run(validation_path):
    validation_file = basf2.find_file(validation_path)
    result = os.system(f'basf2 {validation_file} --dry-run -i input.root -o output.root')
    return result, validation_path


if __name__ == "__main__":

    # First let's check in a smart way how many cores we can use.
    num_workers = max(int(mp.cpu_count() / (1. + os.getloadavg()[1])), 1)
    basf2.B2INFO(f'The test will be executed using {num_workers} workers.')

    # Then, let's run the test!
    validation_paths = glob.glob('skim/validation/*.py')
    failed_files = []
    with ProcessPoolExecutor(max_workers=num_workers) as pool:
        for result in pool.map(dry_run, validation_paths):
            if not result[0] == 0:
                failed_files.append(result[1])
    if len(failed_files) > 0:
        basf2.B2FATAL('The following validation scripts in the skim package failed, check them:\n'
                      + ' \n'.join(failed_files))
