#!/usr/bin/env python
# -*- coding: utf-8 -*-

from actorFramework import *
import os
from basf2 import *
import unittest


class TestResource(unittest.TestCase):
    def setUp(self):
        self.r1 = Resource('TestName', 23, ['a', 'b'])
        self.r2 = Resource('TestName', 23)

    def test_members(self):
        self.assertEqual(self.r1.name, 'TestName')
        self.assertListEqual(self.r1.requires, ['a', 'b'])
        self.assertEqual(self.r2.name, 'TestName')
        self.assertListEqual(self.r2.requires, [])

    def test_call(self):
        self.assertDictEqual(self.r1({'a': None, 'b': 2, 'c': 3}), {'TestName': None})
        self.assertDictEqual(self.r1({'a': 1, 'b': 2, 'c': None}), {'TestName': 23})
        with self.assertRaises(RuntimeError):
            self.r1({'b': 2, 'c': None})
        self.assertDictEqual(self.r2({'a': None, 'b': 2, 'c': None}), {'TestName': 23})


class TestFunction(unittest.TestCase):
    def setUp(self):
        def fun(a, b):
            return {'r': a * sum(b)}
        self.f = Function(fun, a='a', b=['b1', 'b2'])

    def test_members(self):
        self.assertEqual(self.f.name, 'fun')
        self.assertListEqual(self.f.requires, ['a', 'b1', 'b2'])

    def test_call(self):
        self.assertDictEqual(self.f({'a': 2, 'b1': 1, 'b2': 3}), {'r': 8})
        with self.assertRaises(RuntimeError):
            self.f({'a': 2, 'b1': 1, 'b3': 3})


class MockModule(Module):
    def __init__(self, name='NoName', isParallelCertified=True):
        super(MockModule, self).__init__()
        self.name = name
        self.has_property = isParallelCertified

    def has_properties(self, *args):
        return self.has_property


class TestFunction(object):
    def __init__(self, func):
        self.func = func
        self.calls = 0
        self.__name__ = func.__name__

    def __call__(self, path, *args, **kwargs):
        self.calls = self.calls + 1
        self.args = args
        self.kwargs = kwargs
        self.result = self.func(*args, **kwargs)
        path.add_module(MockModule())
        return self.result


class TestSequence(unittest.TestCase):
    def setUp(self):
        self.s = Sequence()

    def test_addResource(self):
        self.s.addResource('c', 3, ['a', 'b'])
        self.assertIsInstance(self.s.seq[0], Resource)
        self.assertEqual(self.s.seq[0].name, 'c')
        self.assertListEqual(self.s.seq[0].requires, ['a', 'b'])

    def test_addFunction(self):
        def fun(a, b):
            return {'r': a * sum(b)}
        self.s.addFunction(fun, a='a', b=['b1', 'b2'])
        self.assertIsInstance(self.s.seq[0], Function)
        self.assertEqual(self.s.seq[0].name, 'fun')
        self.assertListEqual(self.s.seq[0].requires, ['a', 'b1', 'b2'])

    def test_simple_run(self):
        self.s.addResource('b1', 1, ['a'])
        self.s.addResource('a', 2)
        self.s.addResource('b2', 3, ['b1', 'a'])

        def fun(a, b):
            return {'result': a * sum(b)}
        t = TestFunction(fun)

        self.s.addFunction(t, path='Path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': 8})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [1, 3]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)
        self.assertEqual(len(path.modules()), 1)
        self.assertEqual(t.calls, 2)

    def test_parallel_run(self):
        self.s.addResource('b1', 1, ['a'])
        self.s.addResource('a', 2)
        self.s.addResource('b2', 3, ['b1', 'a'])

        def fun(a, b):
            return {'result': a * sum(b)}
        t = TestFunction(fun)

        self.s.addFunction(t, path='Path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False, 2)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': 8})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [1, 3]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)
        self.assertEqual(len(path.modules()), 1)
        self.assertEqual(t.calls, 2)

    def test_none_is_not_needed(self):
        self.s.addResource('b1', 1, ['a'])
        self.s.addResource('a', 2)
        self.s.addResource('b2', 3, ['b1', 'a'])

        def fun(a, b):
            return {'result': None}
        t = TestFunction(fun)

        self.s.addFunction(t, path='Path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': None})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [1, 3]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)

        # But modules should still be not in the path
        # Because fun returns None!
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 2)

    def test_returned_key_NotNeeded_is_not_needed(self):
        self.s.addResource('b1', 1, ['a'])
        self.s.addResource('a', 2)
        self.s.addResource('b2', 3, ['b1', 'a'])

        def fun(a, b):
            return {'result': a * sum(b), 'NotNeeded': None}
        t = TestFunction(fun)

        self.s.addFunction(t, path='Path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': 8, 'NotNeeded': None})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [1, 3]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)

        # But modules should still be not in the path
        # Because fun returns NotNeeded!
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 2)

    def test_optimize_parallel_processing(self):
        def a1(path):
            path.add_module(MockModule('a1', isParallelCertified=True))
            return {'a1': 1}

        def a2(path, a1):
            path.add_module(MockModule('a2', isParallelCertified=False))
            return {'a2': 1}

        def a3(path, a2):
            path.add_module(MockModule('a3', isParallelCertified=True))
            return {'a3': 1}

        def b1(path):
            path.add_module(MockModule('b1', isParallelCertified=True))
            return {'b1': 1}

        def b2(path, b1):
            path.add_module(MockModule('b2', isParallelCertified=True))
            return {'b2': 1}

        def b3(path, b2):
            path.add_module(MockModule('b3', isParallelCertified=True))
            return {'b3': 1}

        def c1(path):
            path.add_module(MockModule('c1', isParallelCertified=True))
            return {'c1': 1}

        def c2(path, c1):
            path.add_module(MockModule('c2', isParallelCertified=True))
            return {'c2': 1}

        def c3(path, c2):
            path.add_module(MockModule('c3', isParallelCertified=False))
            return {'c3': 1}

        def d(path, a3, b3, c3):
            path.add_module(MockModule('d', isParallelCertified=True))
            return {}

        self.s.addFunction(a1, path='Path')
        self.s.addFunction(a2, path='Path', a1='a1')
        self.s.addFunction(a3, path='Path', a2='a2')

        self.s.addFunction(b1, path='Path')
        self.s.addFunction(b2, path='Path', b1='b1')
        self.s.addFunction(b3, path='Path', b2='b2')

        self.s.addFunction(c1, path='Path')
        self.s.addFunction(c2, path='Path', c1='c1')
        self.s.addFunction(c3, path='Path', c2='c2')

        self.s.addFunction(d, path='Path', a3='a3', b3='b3', c3='c3')

        path = create_path()
        self.s.run(path, False)

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

if __name__ == '__main__':
    unittest.main()
