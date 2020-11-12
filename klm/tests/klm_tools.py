#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test the klm tools.
"""

import os
import glob
import subprocess as sp

import basf2
import b2test_utils as b2u
import validation_gt as vgt

if __name__ == '__main__':

    # Test b2klm-create-dqm (it also tests b2klm-mask-dqm and b2klm-execute-masking)
    if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
        basf2.B2INFO('Skipping the b2klm-create-dqm test.')
    else:
        globaltags = ' '.join(vgt.get_validation_globaltags())
        input_files = glob.glob(os.environ['BELLE2_VALIDATION_DATA_DIR'] + '/rawdata/*HLT?.*.root')
        input_files.sort(reverse=True)
        with b2u.clean_working_directory():
            assert(0 == sp.call(['b2klm-create-dqm', '-i', f'{basf2.find_file(input_files[0])}', '-n', '100',
                                 '--prepend_gt'] + vgt.get_validation_globaltags(), stdout=sp.DEVNULL))

    # Test b2klm-numberToIndex
    assert(0 == sp.call(['b2klm-numberToIndex', '20', '50020']))

    # Test b2klm-indexToNumber
    assert(0 == sp.call(['b2klm-indexToNumber', '11489', '21997']))
