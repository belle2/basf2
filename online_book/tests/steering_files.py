#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Largely based on analysis/test/examples.py. """

# std
import os
import sys
import subprocess
import unittest
import glob

# basf2
from basf2 import find_file
from b2test_utils import clean_working_directory, skip_test_if_light


class SteeringFileTest(unittest.TestCase):
    """ Test steering files """

    def _test_examples_dir(self, path_to_glob, broken=None, additional_arguments=None):
        """
        Internal function to test a directory full of example scripts with an
        optional list of broken scripts to be skipped.

        Parameters:
            path_to_glob (str): the path to search for scripts
            broken (list(str)): (optional) scripts that are known to be broken
                and can be skipped
        """
        if additional_arguments is None:
            additional_arguments = []
        if broken is None:
            broken = []
        all_egs = sorted(glob.glob(find_file(path_to_glob) + "/*.py"))
        for eg in all_egs:
            filename = os.path.basename(eg)
            if filename not in broken:
                with self.subTest(msg=filename):
                    result = subprocess.run(
                        ["basf2", "-n1", eg, *additional_arguments],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT,
                    )
                    if result.returncode != 0:
                        # failure running example so let's print the output
                        # on stderr so it's not split from output of unittest
                        # done like this since we don't want to decode/encode utf8
                        sys.stdout.buffer.write(result.stdout)
                    self.assertEqual(result.returncode, 0)

    def test_lessons_1_to_5(self):
        self._test_examples_dir(
            "online_book/basf2/steering_files", additional_arguments=["1"]
        )


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main()
