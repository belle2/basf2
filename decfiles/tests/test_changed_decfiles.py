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

import basf2
import b2test_utils
import os
import subprocess

from git import Repo
from pathlib import Path
from tempfile import NamedTemporaryFile


def add_hint(errorstring: str):

    hint = "\nDon't forget to add custom (i.e. not yet discovered or measured) particles to " \
           "decfiles/tests/test_changed_decfiles.pdl, otherwise the test will not pass." \
           " Already discovered particles should go in framework/particledb/data/evt.pdl instead."

    if 'Unknown particle name' in errorstring:
        return errorstring + hint
    else:
        return errorstring


def decfile_uses_generic_dalitz(decfile_path) -> bool:
    """
    Decay files using the GENERIC_DALITZ model need an additional XML
    file specifying the resonances. The XML's path is given in the
    decfile as a path relative to the working directory. Therefore, in
    order to test decfiles using GENERIC_DALITZ, basf2 must be run from
    the directory containing the decfile, otherwise EvtGen will be
    unable to find the XML. This function returns True if the decfile
    uses GENERIC_DALITZ.
    """
    with open(decfile_path) as decfile:
        for ln in decfile:
            if not ln.startswith('#') and 'GENERIC_DALITZ' in ln:
                return True
    return False


if __name__ == '__main__':

    b2test_utils.configure_logging_for_tests()

    if not b2test_utils.is_ci():
        b2test_utils.skip_test("Will not test changed decfiles because $BELLE2_IS_CI is not set.")

    if not os.environ.get('BELLE2_LOCAL_DIR'):
        b2test_utils.skip_test("Test for changed decfiles failed because $BELLE2_LOCAL_DIR is not set.")

    topdir = Path(os.environ['BELLE2_LOCAL_DIR'])
    assert topdir.is_dir()

    repo = Repo(topdir)
    merge_base = repo.merge_base('origin/main', repo.head)
    diff_to_main = repo.head.commit.diff(merge_base)

    added_or_modified_decfiles = [topdir / new_file.a_path for new_file in diff_to_main
                                  if (Path(new_file.a_path).suffix == '.dec')
                                  and (Path('decfiles/dec') in Path(new_file.a_path).parents)]

    # in case some decfiles are removed, they end up in the list of modified files:
    # let's keep only the decfiles that are actually found by basf2.find_file
    added_or_modified_decfiles = [decfile for decfile in added_or_modified_decfiles
                                  if basf2.find_file(decfile.as_posix(), silent=True)]

    steering_file = basf2.find_file('decfiles/tests/test_changed_decfiles.py_noexec')
    custom_evtpdl = basf2.find_file("decfiles/tests/test_changed_decfiles.pdl")
    default_evtpdl = basf2.find_file('data/framework/particledb/evt.pdl')

    run_results = []
    if added_or_modified_decfiles:
        changed_file_string = '\n'.join(str(p) for p in added_or_modified_decfiles)
        print(f"Changed decayfiles: \n{changed_file_string}")

        with NamedTemporaryFile(mode='w', suffix='.pdl') as tempfile:

            for fname in [custom_evtpdl, default_evtpdl]:
                with open(fname) as infile:
                    tempfile.write(infile.read())

            for decfile in added_or_modified_decfiles:
                cwd = decfile.parent if decfile_uses_generic_dalitz(decfile) else None
                with b2test_utils.clean_working_directory():
                    run_results.append(subprocess.run(['basf2', steering_file, str(decfile), tempfile.name],
                                                      capture_output=True, cwd=cwd))

    files_and_errors = [f'Decfile {added_or_modified_decfiles[i]} failed with output \n'
                        f'{ret.stdout.decode()} \n and error \n {add_hint(ret.stderr.decode())}'
                        for i, ret in enumerate(run_results) if ret.returncode != 0]

    if len(files_and_errors):
        raise RuntimeError("At least one added decfile has failed.\n"
                           + '\n'.join(files_and_errors))
