#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Check if the Belle II geometry has some overlaps.
"""

import b2test_utils as b2tu
import basf2 as b2
import subprocess


with b2tu.clean_working_directory():

    overlap_checker = b2.find_file('geometry/examples/check_geometry.py')
    try:
        output = subprocess.check_output(['basf2', overlap_checker, '100'], encoding='utf-8')
    except subprocess.CalledProcessError as error:
        output = error.output

    good_string = 'Checking overlaps for volume'
    error_string = '[ERROR] Overlap'

    if error_string in output:
        errors = [e for e in output.splitlines() if not e.startswith(good_string)]
        print('\n'.join(errors))
        b2.B2FATAL('There are overlaps in the geometry. '
                   f'Please run the script {overlap_checker} and fix them.')
