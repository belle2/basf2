#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

# basf2
from basf2 import find_file
from b2test_utils import clean_working_directory


def light_release() -> bool:
    """ Returns true if we're in a light release """
    try:
        import generators  # noqa
    except ModuleNotFoundError:
        return True
    return False


class AwesomeExamplesTest(unittest.TestCase):
    """ Test our awesome examples. """

    def _test_examples_dir(
            self,
            path_to_glob: str
    ):
        """
        Internal function to test a directory full of example scripts.

        Parameters:
            path_to_glob (str): the path to a directory to search for python
                scripts.
        """
        # Do not run the test if we are in a light release.
        if light_release():
            return
        working_dir = find_file(path_to_glob)
        examples = sorted(glob.glob(f'{working_dir}/*.py'))
        for example in examples:
            filename = os.path.basename(example)
            with self.subTest(msg=filename):
                result = subprocess.run(
                    ["basf2", "-n1", example],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                )
                if result.returncode != 0:
                    # failure running example so let's print the output
                    # on stderr so it's not split from output of unittest
                    # done like this since we don't want to decode/encode utf8
                    sys.stdout.buffer.write(result.stdout)
                self.assertEqual(result.returncode, 0)

    def test_examples(self):
        """Test examples of the awesome package. """

        self._test_examples_dir(
            path_to_glob='online_book/awesome/examples',
        )


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main()
