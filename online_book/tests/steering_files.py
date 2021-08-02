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
Test all the steering files used in the online_book lessons.
Proudly based on analysis/test/examples.py.
"""

# std
import os
import sys
import subprocess
import unittest
import glob
import shutil
from typing import Optional, List, Dict
import stat
from pathlib import Path

# basf2
from basf2 import find_file
from b2test_utils import clean_working_directory, is_ci


def light_release() -> bool:
    """Returns true if we're in a light release"""
    try:
        # pylint: disable=import-outside-toplevel
        # pylint: disable=unused-import
        import generators  # noqa
    except ModuleNotFoundError:
        return True
    return False


def _permission_report(folder: str) -> None:
    """Quick helper function to show permissions of folder and a selection
    of files in it
    """
    folder = Path(folder)
    print("-"*80)
    print(f"Permissions of {folder}: {folder.stat()}")
    content = list(folder.iterdir())
    print(content[0].exists())
    if content:
        print(
            f"Permission of one of its contents. {content[0]}: "
            f"{content[0].stat()}"
        )
    test_file = folder / "test_file_123456"
    try:
        test_file.touch()
    except Exception as e:
        print(f"Cannot create a new file in the folder: {e}")
    else:
        print("Able to create a new file in this folder")
    print("-"*80)


class SteeringFileTest(unittest.TestCase):
    """Test steering files"""

    def _test_examples_dir(
        self,
        path_to_glob: str,
        broken: Optional[List[str]] = None,
        additional_arguments: Optional[List[str]] = None,
        expensive_tests: Optional[List[str]] = None,
        skip_in_light: Optional[List[str]] = None,
        skip: Optional[List[str]] = None,
        n_events: Optional[Dict[str, int]] = None,
    ):
        """
        Internal function to test a directory full of example scripts with an
        optional list of broken scripts to be skipped.

        Parameters:
            path_to_glob (str): the path to a directory to search for python
                scripts (must end in .py)
            broken (list(str)): (optional) names of scripts that are known to
                be broken and can be skipped
            additional_arguments (list(str)): (optional) additional arguments
                for basf2 to be passed when testing the scripts
            expensive_tests (list(str)): (optional) names of scripts that take
                longer and should e.g. not run on bamboo
            skip_in_light (list(str)): (optional) names of scripts that have to
                be excluded in light builds
            skip (list(str)): (optional) names of scripts to always skip
            n_events (dict(str, int)): mapping of name of script to number of
                required events for it to run (`-n` argument). If a filename
                isn't listed, we assume 1
        """
        if additional_arguments is None:
            additional_arguments = []
        if broken is None:
            broken = []
        if expensive_tests is None:
            expensive_tests = []
        if skip_in_light is None:
            skip_in_light = []
        if skip is None:
            skip = []
        if n_events is None:
            n_events = {}
        # we have to copy all the steering files (plus other stuffs, like decfiles) we want to test
        # into a new directory and then cd it as working directory when subprocess.run is executed,
        # otherwise the test will fail horribly if find_file is called by one of the tested steerings.
        original_dir = find_file(path_to_glob)
        print(f"Our user id: {os.getuid()}")
        _permission_report(original_dir)
        working_dir = find_file(shutil.copytree(original_dir, "working_dir"))
        _permission_report(working_dir)
        # Add write permissions for user to this directory
        # os.chmod(working_dir, stat.S_IRUSR)
        # _permission_report(working_dir)
        all_egs = sorted(glob.glob(working_dir + "/*.py"))
        for eg in all_egs:
            filename = os.path.basename(eg)
            if filename in broken:
                continue
            if is_ci() and filename in expensive_tests:
                continue
            if light_release() and filename in skip_in_light:
                continue
            if filename in skip:
                continue
            with self.subTest(msg=filename):
                # pylint: disable=subprocess-run-check
                result = subprocess.run(
                    [
                        "basf2",
                        "-n",
                        str(n_events.get(filename, 1)),
                        eg,
                        *additional_arguments,
                    ],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    cwd=working_dir,
                )
                if result.returncode != 0:
                    # failure running example so let's print the output
                    # on stderr so it's not split from output of unittest
                    # done like this since we don't want to decode/encode utf8
                    sys.stdout.buffer.write(result.stdout)
                self.assertEqual(result.returncode, 0)

    # fixme: This should be made to run on buildbot, i.e. by adding the/some
    #   files to the examples/validation directory
    @unittest.skipIf(
        not os.path.exists(
            find_file(
                "starterkit/2021/1111540100_eph3_BGx0_1.root",
                "examples",
                silent=True,
            )
        ),
        "Test data files not found.",
    )
    def test_lessons_1_to_5(self):
        """Test lesson on basf2 basics."""
        self._test_examples_dir(
            path_to_glob="online_book/basf2/steering_files",
            additional_arguments=["1"],
            expensive_tests=["065_generate_mc.py", "067_generate_mc.py"],
            skip_in_light=[
                "065_generate_mc.py",
                "067_generate_mc.py",
                "085_module.py",
                "087_module.py",
            ],
            n_events={
                # See https://questions.belle2.org/question/11344/
                "091_cs.py": 3000,
            },
        )


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main()
