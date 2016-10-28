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


def main():
    unittest.main()

if __name__ == '__main__':
    main()
