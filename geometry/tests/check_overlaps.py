#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Check if the Belle II geometry has overlaps.
"""

import b2test_utils as b2tu
import basf2 as b2
import subprocess


def run_overlap_check(exp=None, use_db=True, points=100):
    """
    Run the Belle II geometry overlap checker and assert no overlaps are detected.
    Also ensures that the check actually ran by verifying 'Checking overlaps for volume' is present.
    """
    overlap_checker = b2.find_file('geometry/examples/check_geometry.py')
    cmd = ['basf2', overlap_checker, '--', str(points)]
    if exp is not None:
        cmd += ['--exp', str(exp)]
    if not use_db:
        cmd += ['--no-use-db']

    try:
        output = subprocess.check_output(cmd, encoding='utf-8')
    except subprocess.CalledProcessError as error:
        output = error.output

    good_string = 'Checking overlaps for volume'
    error_string = '[ERROR] Overlap'

    if good_string not in output:
        print(output)
        b2.B2FATAL(
            f"Overlap check did not run properly (exp={exp}, use_db={use_db}): "
            f"'{good_string}' was not found in output."
        )

    if error_string in output:
        errors = [e for e in output.splitlines() if not e.startswith(good_string)]
        print('\n'.join(errors))
        b2.B2FATAL(
            f"There are overlaps in the geometry (exp={exp}, use_db={use_db}). "
            f"Please run '{overlap_checker}' to fix them."
        )


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        # 1. Test local XML geometry (useDB=False) to catch MRs that change XML files
        run_overlap_check(exp=None, use_db=False)

        # 2. Test conditions database upgrade geometry (useDB=True)
        for exp in [2003, 2004, 2005, 2006]:
            run_overlap_check(exp=exp, use_db=True)
