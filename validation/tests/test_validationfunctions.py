#!/usr/bin/env python3

import unittest
import os
import validationfunctions
import tempfile

call_iteration = 0


class ValidationFunctionstTest(unittest.TestCase):

    """
    Various test cases for the validation utility functions
    """

    def test_git_hash(self):
        """
        Test if the extraction of the git hash works and if not, if the
        functions returns the expected result
        """

        # fixme: disabled for now
        # test disabled for now, until a way to run this properly on the
        # build bot can be found
        return
        # noinspection PyUnreachableCode

        cwd = os.getcwd()

        with tempfile.TemporaryDirectory() as td:
            # switch to this folder
            os.chdir(str(td))
            ret = validationfunctions.get_compact_git_hash(os.getcwd())
            self.assertTrue(ret is None)

        os.chdir(cwd)

        # in the source folder, we should get a sensible output
        local_dir = os.environ.get('BELLE2_LOCAL_DIR', None)
        if local_dir:
            ret = validationfunctions.get_compact_git_hash(local_dir)
            self.assertTrue(ret is not None)
            print(f"This source code has git hash {ret}")


if __name__ == "__main__":
    unittest.main()
