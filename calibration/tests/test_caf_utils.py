import unittest
from unittest import TestCase

from caf.utils import topological_sort
from caf.utils import get_iov_from_file
from caf.utils import IoV


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


class TestUtils_getiov(TestCase):
    """
    Test getting IOVs
    """

    def test_get_iov_from_file(self):
        """
        test get_iov_from_file()
        """
        from ROOT import Belle2
        path = Belle2.FileSystem.findFile('framework/tests/root_input.root')
        iov = get_iov_from_file(path)
        self.assertTrue(IoV(0, 1, 0, 1).contains(iov))


def main():
    unittest.main()


if __name__ == '__main__':
    main()
