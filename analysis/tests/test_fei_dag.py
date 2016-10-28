#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from fei.dag import *
import os
import pickle
from basf2 import *
import unittest

# @cond internal_test


class TestCreateHash(unittest.TestCase):

    def test_create_hash(self):
        self.assertEqual(create_hash([1, 2]), create_hash([1, 2, None]))
        self.assertEqual(create_hash([1, 2.0]), create_hash([1, 2.000001]))
        self.assertNotEqual(create_hash([1, 2.0]), create_hash([1, 2.001]))
        self.assertNotEqual(create_hash([1, 2]), create_hash([1, 2, 3]))


def test_func(resource, a, b, c):
    assert resource.hash == create_hash([1, 2, 4, 5, 3, 6])
    assert resource.needed is False
    resource.needed = True
    assert resource.halt is False
    resource.halt = True
    assert resource.cache is False
    resource.cache = True
    assert resource.condition is None
    resource.condition = ('EventType', '==0')
    assert resource.env['Test'] is True
    assert resource.identifier == 'Identifier'
    assert a == 3
    assert b == [4, 5]
    assert c == 6
    resource.path.add_module('RootInput')
    # TODO Howto check if global_lock is locked?
    # with resource.EnableMultiThreading():
    return 42


def test_cache(resource):
    resource.cache_was_not_used = True
    return 42


class TestResource(unittest.TestCase):

    def test_value_resource(self):
        environment = {'Test': True}
        identifier = 'Identifier'
        provider = 42

        a = Resource(environment, identifier, provider, 'R1', 'R2', ['R3', 'R4'])
        self.assertEqual(a.identifier, identifier)
        self.assertDictEqual(a.env, environment)
        self.assertListEqual(sorted(a.requires), sorted(['R1', 'R2', 'R3', 'R4']))

        self.assertEqual(a.needed, False)
        self.assertEqual(a.cache, False)
        self.assertEqual(a.halt, False)
        self.assertEqual(a.path, None)
        self.assertEqual(a.condition, None)
        self.assertEqual(a.hash, None)
        self.assertEqual(a.value, None)
        self.assertEqual(a.loaded_from_cache, False)

        self.assertEqual(a({'R1': 1, 'R2': 2, 'R3': 3, 'R4': 4, 'R5': 5}), 42)
        self.assertEqual(a.hash, create_hash([1, 2, 3, 4]))
        self.assertEqual(a.value, 42)

        serialized_resource = pickle.dumps(a)
        deserialized_resource = pickle.loads(serialized_resource)

        self.assertEqual(deserialized_resource.identifier, identifier)
        self.assertDictEqual(deserialized_resource.env, environment)
        self.assertListEqual(sorted(deserialized_resource.requires), sorted(['R1', 'R2', 'R3', 'R4']))

        self.assertEqual(deserialized_resource.needed, False)
        self.assertEqual(deserialized_resource.cache, False)
        self.assertEqual(deserialized_resource.halt, False)
        self.assertEqual(deserialized_resource.path, None)
        self.assertEqual(deserialized_resource.condition, None)
        self.assertEqual(deserialized_resource.hash, create_hash([1, 2, 3, 4]))
        self.assertEqual(deserialized_resource.value, 42)
        self.assertEqual(deserialized_resource.loaded_from_cache, True)

    def test_function_resource(self):
        environment = {'Test': True}
        identifier = 'Identifier'
        provider = test_func

        a = Resource(environment, identifier, provider, '1', '2', a='3', b=['4', '5'])
        self.assertEqual(a.identifier, identifier)
        self.assertDictEqual(a.env, environment)
        self.assertListEqual(sorted(a.requires), sorted(['1', '2', '3', '4', '5', 'c']))

        self.assertEqual(a.needed, False)
        self.assertEqual(a.cache, False)
        self.assertEqual(a.halt, False)
        self.assertEqual(a.path, None)
        self.assertEqual(a.condition, None)
        self.assertEqual(a.hash, None)
        self.assertEqual(a.value, None)
        self.assertEqual(a.loaded_from_cache, False)

        self.assertEqual(a({'1': 1, '2': 2, '3': 3, '4': 4, '5': 5, 'c': 6, '7': 7}), 42)
        self.assertEqual(a.hash, create_hash([1, 2, 4, 5, 3, 6]))
        self.assertEqual(a.value, 42)
        self.assertEqual(a.needed, True)
        self.assertEqual(a.cache, True)
        self.assertEqual(a.halt, True)
        self.assertEqual(len(a.path.modules()), 1)
        self.assertEqual(a.path.modules()[0].name(), 'RootInput')
        self.assertEqual(a.condition, ('EventType', '==0'))

        serialized_resource = pickle.dumps(a)
        deserialized_resource = pickle.loads(serialized_resource)

        self.assertEqual(deserialized_resource.identifier, identifier)
        self.assertDictEqual(deserialized_resource.env, environment)
        self.assertListEqual(sorted(deserialized_resource.requires), sorted(['1', '2', '3', '4', '5', 'c']))

        self.assertEqual(deserialized_resource.hash, create_hash([1, 2, 4, 5, 3, 6]))
        self.assertEqual(deserialized_resource.needed, True)
        self.assertEqual(deserialized_resource.cache, True)
        self.assertEqual(deserialized_resource.halt, True)
        self.assertEqual(len(deserialized_resource.path.modules()), 1)
        self.assertEqual(deserialized_resource.path.modules()[0].name(), 'RootInput')
        self.assertEqual(a.condition, ('EventType', '==0'))
        self.assertEqual(deserialized_resource.value, 42)
        self.assertEqual(deserialized_resource.loaded_from_cache, True)

    def test_function_resource_fail(self):
        environment = {'Test': True}
        identifier = 'Identifier'
        provider = test_func

        a = Resource(environment, identifier, provider, '1', '2', a='3', b=['4', '5'])
        self.assertListEqual(sorted(a.requires), sorted(['1', '2', '3', '4', '5', 'c']))
        # Missing keyword requirement
        with self.assertRaises(KeyError):
            self.assertEqual(a({'1': 1, '2': 2, '3': 3, '5': 5, 'c': 6, '7': 7}), 42)
        # Missing positional requirement
        with self.assertRaises(KeyError):
            self.assertEqual(a({'1': 1, '4': 4, '3': 3, '5': 5, 'c': 6, '7': 7}), 42)
        # Missing automatic requirement
        with self.assertRaises(KeyError):
            self.assertEqual(a({'1': 1, '4': 4, '3': 3, '5': 5, '2': 2, '7': 7}), 42)

    def test_function_cached_resource(self):
        environment = {'Test': True}
        identifier = 'Identifier'
        provider = test_cache

        a = Resource(environment, identifier, provider, 'r')
        # Cache is not used on the first call, because value is None
        self.assertEqual(a({'r': 23}), 42)
        self.assertTrue(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Cache is used on the second call
        self.assertEqual(a({'r': 23}), 42)
        self.assertFalse(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Cache is not used if hash changes
        self.assertEqual(a({'r': 22}), 42)
        self.assertTrue(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Cache is used again
        self.assertEqual(a({'r': 22}), 42)
        self.assertFalse(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Cache is not used if self.halt is set
        a.halt = True
        self.assertEqual(a({'r': 22}), 42)
        self.assertTrue(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Halt flag is reset
        self.assertFalse(a.halt)
        # And cache is valid again
        self.assertEqual(a({'r': 22}), 42)
        self.assertFalse(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Enforce invalid cache
        environment['rerunCachedProviders'] = True
        self.assertEqual(a({'r': 22}), 42)
        self.assertTrue(a.cache_was_not_used)
        a.cache_was_not_used = False
        # Cache is valid again
        environment['rerunCachedProviders'] = False
        self.assertEqual(a({'r': 22}), 42)
        self.assertFalse(a.cache_was_not_used)
        a.cache_was_not_used = False


class MockModule(Module):

    def __init__(self, name='NoName', isParallelCertified=True):
        super(MockModule, self).__init__()
        self.name = name
        self.has_property = isParallelCertified

    def has_properties(self, *args):
        return self.has_property


class TestDAG(unittest.TestCase):

    def setUp(self):
        self.s = DAG()

    def test_addResource(self):
        self.s.add('a', 3)
        self.assertIsInstance(self.s.resources['a'], Resource)
        self.assertEqual(self.s.resources['a'].identifier, 'a')
        self.assertListEqual(self.s.resources['a'].requires, [])

        self.s.add('b', 3, 'a', ['b', 'c'], d='d', ef=['e', 'f'])
        self.assertIsInstance(self.s.resources['b'], Resource)
        self.assertEqual(self.s.resources['b'].identifier, 'b')
        self.assertListEqual(sorted(self.s.resources['b'].requires), sorted(['a', 'b', 'c', 'd', 'e', 'f']))

    def test_optimize_parallel_processing(self):
        def a1(resource):
            resource.path.add_module(MockModule('a1', isParallelCertified=True))

        def a2(resource, a1):
            resource.path.add_module(MockModule('a2', isParallelCertified=False))

        def a3(resource, a2):
            resource.path.add_module(MockModule('a3', isParallelCertified=True))

        def b1(resource):
            resource.path.add_module(MockModule('b1', isParallelCertified=True))

        def b2(resource, b1):
            resource.path.add_module(MockModule('b2', isParallelCertified=True))

        def b3(resource, b2):
            resource.path.add_module(MockModule('b3', isParallelCertified=True))

        def c1(resource):
            resource.path.add_module(MockModule('c1', isParallelCertified=True))

        def c2(resource, c1):
            resource.path.add_module(MockModule('c2', isParallelCertified=True))

        def c3(resource, c2):
            resource.path.add_module(MockModule('c3', isParallelCertified=False))

        def d(resource, a3, b3, c3):
            resource.path.add_module(MockModule('d', isParallelCertified=True))
            resource.needed = True

        self.s.add('a1', a1)
        self.s.add('a2', a2)
        self.s.add('a3', a3)

        self.s.add('b1', b1)
        self.s.add('b2', b2)
        self.s.add('b3', b3)

        self.s.add('c1', c1)
        self.s.add('c2', c2)
        self.s.add('c3', c3)

        self.s.add('d', d)

        path = create_path()
        self.s.run(path)

        self.assertEqual(len(path.modules()), 10)
        self.assertEqual(path.modules()[0].name, 'a1')
        self.assertEqual(path.modules()[1].name, 'b1')
        self.assertEqual(path.modules()[2].name, 'c1')
        self.assertEqual(path.modules()[3].name, 'b2')
        self.assertEqual(path.modules()[4].name, 'c2')
        self.assertEqual(path.modules()[5].name, 'b3')
        self.assertEqual(path.modules()[6].name, 'a2')
        self.assertEqual(path.modules()[7].name, 'a3')
        self.assertEqual(path.modules()[8].name, 'c3')
        self.assertEqual(path.modules()[9].name, 'd')

    def test_simple_run(self):
        self.s.add('a', 2)
        self.s.add('b1', 1, ['a'])
        self.s.add('b2', 2, ['b1', 'a'])

        def fun(resource, a, b):
            resource.path.add_module('RootInput')
            return 8

        self.s.add('result', fun, a='a', b=['b1', 'b2'])
        self.s.add('x', 1, 'result')

        path = create_path()
        self.assertTrue(self.s.run(path))

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(self.s.resources['result'].value, 8)

        # Add needed result and try again
        self.s.addNeeded('x')
        self.assertTrue(self.s.run(path))
        self.assertEqual(len(path.modules()), 1)

    def test_parallel_run(self):
        self.s.add('a', 2)
        self.s.add('b1', 1, ['a'])
        self.s.add('b2', 2, ['b1', 'a'])

        def fun(resource, a, b):
            resource.path.add_module('RootInput')
            return 8

        self.s.add('result', fun, a='a', b=['b1', 'b2'])
        self.s.add('x', 1, 'result')

        path = create_path()
        self.s.env['nThreads'] = 2
        self.assertTrue(self.s.run(path))

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(self.s.resources['result'].value, 8)

        # Add needed result and try again
        self.s.addNeeded('x')
        self.assertTrue(self.s.run(path))
        self.assertEqual(len(path.modules()), 1)

    def test_halt_run(self):
        self.s.add('a', 2)
        self.s.add('b1', 1, ['a'])
        self.s.add('b2', 2, ['b1', 'a'])

        def fun(resource, a, b):
            resource.halt = True
            resource.path.add_module('RootInput')
            return 8

        self.s.add('result', fun, a='a', b=['b1', 'b2'])
        self.s.add('x', 1, 'result')

        path = create_path()
        self.assertFalse(self.s.run(path))

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(self.s.resources['result'].value, 8)

        # Add needed result and try again
        self.s.addNeeded('x')
        # Not all resources are fulfilled due to resoure.halt = True
        self.assertFalse(self.s.run(path))
        self.assertEqual(len(path.modules()), 1)

    def test_condition_run(self):
        self.s.add('a', 2)
        self.s.add('b1', 1, ['a'])
        self.s.add('b2', 2, ['b1', 'a'])

        def fun(resource, a, b):
            resource.condition = ('EventType', '==0')
            resource.path.add_module('RootInput')
            return 8

        self.s.add('result', fun, a='a', b=['b1', 'b2'])
        self.s.add('x', 1, 'result')

        path = create_path()
        self.assertTrue(self.s.run(path))

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(self.s.resources['result'].value, 8)

        # Add needed result and try again
        self.s.addNeeded('x')
        self.assertTrue(self.s.run(path))
        # Check if condition is correctly implemented
        self.assertEqual(len(path.modules()), 1)
        first_module = path.modules()[0]

        self.assertEqual(first_module.type(), 'VariableToReturnValue')
        self.assertEqual(first_module.available_params()[0].name, 'variable')
        self.assertEqual(first_module.available_params()[0].values, 'EventType')
        self.assertTrue(first_module.has_condition())

        conditions = first_module.get_all_conditions()
        self.assertEqual(len(conditions), 1)
        self.assertEqual(conditions[0].get_after_path(), basf2.AfterConditionPath.CONTINUE)
        self.assertEqual(conditions[0].get_value(), 0)

        self.assertEqual(str(ConditionOperator.values[conditions[0].get_operator()]), '==')
        condition_paths = first_module.get_all_condition_paths()
        self.assertEqual(len(condition_paths), 1)
        self.assertEqual(len(condition_paths[0].modules()), 1)
        self.assertEqual(condition_paths[0].modules()[0].name(), 'RootInput')


if __name__ == '__main__':
    unittest.main()

# @endcond
