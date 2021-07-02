import unittest
from unittest import TestCase


class Test_Prompt(TestCase):
    """
    Checking prompt validation scripts directory.
    """

    def test_check(self):
        """
        Uses b2val-prompt-check to make sure that cripts can all be imported and duplicate names
        for validations don't exist.
        """
        tool_name = "b2val-prompt-check"
        import subprocess
        try:
            subprocess.check_output(tool_name, stderr=subprocess.STDOUT, universal_newlines=True)
        except subprocess.CalledProcessError as e:
            print(e.output)
            raise e


def main():
    unittest.main()


if __name__ == '__main__':
    main()
