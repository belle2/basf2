##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Simple launcher for decfiles/tests/test_changed_decfiles.py_noexec. This tests whether new decay files work.
'''

import subprocess

import basf2
import b2test_utils

from git import Repo
from pathlib import Path
import os


if __name__ == '__main__':

    b2test_utils.configure_logging_for_tests()
    topdir = Path(os.environ['BELLE2_LOCAL_DIR'])
    assert topdir.is_dir()

    diff_to_main = Repo(topdir).head.commit.diff('main')

    added_or_modified_decfiles = [topdir / new_file.a_path for new_file in diff_to_main
                                  if (Path(new_file.a_path).suffix == '.dec')
                                  and (Path('decfiles/dec') in Path(new_file.a_path).parents)]

    if added_or_modified_decfiles:
        print(f"Changed decayfiles: {' '.join(str(p) for p in added_or_modified_decfiles)}")
    steering_file = basf2.find_file('decfiles/tests/test_changed_decfiles.py_noexec')

    run_results = []
    for decfile in added_or_modified_decfiles:
        with b2test_utils.clean_working_directory():
            run_results.append(subprocess.run(['basf2', steering_file, str(decfile)], capture_output=True))

    files_and_errors = [f'Decfile {added_or_modified_decfiles[i]} failed with error \n {ret.stderr.decode()}'
                        for i, ret in enumerate(run_results) if ret.returncode != 0]
    if len(files_and_errors):

        raise RuntimeError("At least one added decfile has failed. "
                           + '\n'.join(files_and_errors))
