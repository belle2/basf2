##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


'''
Simple launcher for decfiles/tests/test_changed_datfiles.py_noexec. This tests whether new decay files work for TauolaBelle2.
'''

import basf2
import b2test_utils
import os
import subprocess

from git import Repo
from pathlib import Path


def add_hint(exit_code):
    if exit_code == 2:
        return "[ERROR] The given datfile can't be interpreted by basf2."
    elif exit_code == 1:
        return "[ERROR] The given datfile contains value that doesn't exist yet."
    elif exit_code == 0:
        return "[ERROR] The given datfile contains unknown value."
    else:
        return "[ERROR] Unknown error occurred. Please Debug your Datfile"


if __name__ == '__main__':

    b2test_utils.configure_logging_for_tests()

    if not b2test_utils.is_ci():    # to determine whether the current execution environment is a Continuous Integration (CI) system
        b2test_utils.skip_test("Will not test changed decfiles because $BELLE2_IS_CI is not set.")

    if not os.environ.get('BELLE2_LOCAL_DIR'):
        b2test_utils.skip_test("Test for changed decfiles failed because $BELLE2_LOCAL_DIR is not set.")

    topdir = Path(os.environ['BELLE2_LOCAL_DIR'])
    assert topdir.is_dir()
    # Initialize the repository object
    repo = Repo(topdir)
    # Get the merge base of 'origin/main' and current HEAD
    merge_base = repo.merge_base('origin/main', repo.head)

    # Get the difference between the current HEAD and 'origin/main'
    diff_to_main = repo.head.commit.diff(merge_base)

    added_or_modified_decfiles = [topdir / new_file.a_path for new_file in diff_to_main
                                  if (Path(new_file.a_path).suffix == '.dat')
                                  and (Path('decfiles/dec/TauolaBelle2') in Path(new_file.a_path).parents)]
    # in case some decfiles are removed, they end up in the list of modified files:
    # let's keep only the decfiles that are actually found by basf2.find_file
    added_or_modified_decfiles = [decfile for decfile in added_or_modified_decfiles
                                  if basf2.find_file(decfile.as_posix(), silent=True)]

    steering_file = basf2.find_file('decfiles/tests/test_changed_tauolabelle2.py_noexec')

    run_results = []
    if added_or_modified_decfiles:
        changed_file_string = '\n'.join(str(p) for p in added_or_modified_decfiles)
        print(f"Changed decayfiles: \n{changed_file_string}")

        for decfile in added_or_modified_decfiles:
            with b2test_utils.clean_working_directory():
                run_results.append(subprocess.run(['basf2', steering_file, str(decfile)], capture_output=True))

    # The condition '5/     5 events' is essential because in certain cases, if a user inputs a value that does
    # not exist in the TauolaBelle2 database, the basf2 program may terminate with a return code of 0.
    # This behavior occurs because the KKMC generator, utilized by TauolaBelle2, is written in Fortran,
    # and in such instances, it abruptly halts the execution of the code with a return code of 0.
    files_and_errors = [f'Decfile {added_or_modified_decfiles[i]} failed with output \n'
                        f'{ret.stdout.decode()} \n and {add_hint(ret.returncode)}'
                        for i, ret in enumerate(run_results)
                        if ret.returncode != 0 or (b'5/     5 events' not in ret.stdout and ret.returncode == 0)]

    if len(files_and_errors):
        raise RuntimeError("At least one added decfile has failed.\n"
                           + '\n'.join(files_and_errors))
