#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Largely based on analysis/test/examples.py. """

# std
import os
import sys
import subprocess
import unittest
import glob
from typing import Optional, List

# basf2
from basf2 import find_file
from b2test_utils import clean_working_directory


class SteeringFileTest(unittest.TestCase):
    """ Test steering files """

    def _test_examples_dir(
        self,
        path_to_glob: str,
        broken: Optional[List[str]] = None,
        additional_arguments: Optional[List[str]] = None,
        change_working_directory=True,
    ):
        """
        Internal function to test a directory full of example scripts with an
        optional list of broken scripts to be skipped.

        Parameters:
            path_to_glob (str): the path to a directory to search for python
                scripts (must end in .py)
            broken (list(str)): (optional) scripts that are known to be broken
                and can be skipped
            additional_arguments (list(str)): (optional) additional arguments
                for basf2 to be passed when testing the scripts
            change_working_directory: Change to path_to_glob for execution
        """
        if additional_arguments is None:
            additional_arguments = []
        if broken is None:
            broken = []
        working_dir = find_file(path_to_glob)
        all_egs = sorted(glob.glob(working_dir + "/*.py"))
        for eg in all_egs:
            filename = os.path.basename(eg)
            if filename not in broken:
                with self.subTest(msg=filename):
                    result = subprocess.run(
                        ["basf2", "-n1", eg, *additional_arguments],
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
        # Pass 1 for a potentially expected first argument (file number) and
        # -n 10 for e.g. the "generating MC" lesson
        self._test_examples_dir(
            "online_book/basf2/steering_files", additional_arguments=["1", "-n", "10"]
        )


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main()
