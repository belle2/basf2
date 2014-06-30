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
    pass


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

if __name__ == '__main__':
    unittest.main()
