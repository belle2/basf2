import unittest
from unittest import TestCase

from caf.utils import topological_sort
from caf.utils import decode_json_string


class TestUtils_topological_sort(TestCase):
    """
    Set of tests for the topological sort function
    """
    def test_order(self):
        """
        Checks that the resulting order is correct for a basic test
        """
        dependencies = {}
        dependencies['c'] = ['a', 'b']
        dependencies['b'] = ['a']
        dependencies['a'] = []
        order = topological_sort(dependencies)
        self.assertEqual(order, ['c', 'b', 'a'])

    def test_cyclic(self):
        """
        Tests that cyclic dependency returns empty list
        """
        dependencies = {}
        dependencies['c'] = ['a']
        dependencies['a'] = ['b']
        dependencies['b'] = ['c']
        order = topological_sort(dependencies)
        self.assertFalse(order)


class TestUtils_json(TestCase):
    """
    Set of tests for checking decoding of json
    """
    def setUp(self):
        """
        Build the config parser for these tests
        """
        import os
        config_file_path = os.environ.get('BELLE2_LOCAL_DIR')
        config_file_path = os.path.join(config_file_path, 'calibration/data/caf.cfg')
        import configparser
        #: Configuration object for testing
        self.config = configparser.ConfigParser()
        self.config.read(config_file_path)

    def test_decode_int(self):
        """
        Checks that int can be decoded from config file
        """
        self.assertTrue(decode_json_string(self.config['TESTS']['TestInt']) == 3)

    def test_decode_list(self):
        """
        Checks that a test list can be correctly decoded if
        it contains mixed string and int, string, list types.
        """
        test_list = ["a", "1", 2, 3, [4, "5"]]
        self.assertTrue(decode_json_string(self.config['TESTS']['TestList']) == test_list)


def main():
    unittest.main()

if __name__ == '__main__':
    main()
