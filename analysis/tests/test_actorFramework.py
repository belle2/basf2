#!/usr/bin/env python
# -*- coding: utf-8 -*-

from actorFramework import *
import os
from basf2 import *
import unittest


class TestProperty(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.r1 = Property('TestName', 23)

    def test_members(self):
        """ Unittest function """
        self.assertEqual(self.r1.name, 'TestName')
        self.assertListEqual(self.r1.requires, [])

    def test_call(self):
        self.assertDictEqual(self.r1({'a': 1, 'b': 2, 'c': None}), {'TestName': 23})


class TestCollection(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.r1 = Collection('TestName', ['a', 'b'])

    def test_members(self):
        """ Unittest function """
        self.assertEqual(self.r1.name, 'TestName')
        self.assertListEqual(self.r1.requires, ['a', 'b'])

    def test_call(self):
        """ Unittest function """
        self.assertDictEqual(self.r1({'a': 1, 'b': 2, 'c': 3}), {'TestName': {'a': 1, 'b': 2}})


class TestActor(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        def fun(a, b, c):
            return {'r': a * sum(b)}
        self.f = Actor(fun, a='a_t', b=['b1', 'b2'])

    def test_construct(self):
        """ Unittest function """
        def fun(a, b, c):
            return {'r': a * sum(b)}
        with self.assertRaises(KeyError):
            Actor(fun, d='a_t', b=['b1', 'b2'])({'a_t': 2, 'b1': 1, 'b2': 3, 'c': 4, 'd': 5})

    def test_members(self):
        """ Unittest function """
        self.assertEqual(self.f.name, 'fun')
        self.assertListEqual(self.f.requires, ['c', 'a_t', 'b1', 'b2'])

    def test_call(self):
        """ Unittest function """
        self.assertDictEqual(self.f({'a_t': 2, 'b1': 1, 'b2': 3, 'c': 4}), {'r': 8})
        with self.assertRaises(KeyError):
            self.f({'a': 2, 'b1': 1, 'b3': 3})


class MockModule(Module):
    """ Unittest class """
    def __init__(self, name='NoName', isParallelCertified=True):
        """ Unittest function """
        super(MockModule, self).__init__()
        self.name = name
        self.has_property = isParallelCertified

    def has_properties(self, *args):
        """ Unittest function """
        return self.has_property


class TestFunction(object):
    """ Unittest class """
    def __init__(self, func):
        """ Unittest function """
        self.func = func
        self.calls = 0
        self.__name__ = func.__name__

    def __call__(self, path, *args, **kwargs):
        """ Unittest function """
        self.calls = self.calls + 1
        self.args = args
        self.kwargs = kwargs
        self.result = self.func(*args, **kwargs)
        path.add_module(MockModule())
        return self.result


class TestPlay(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.s = Play()

    def test_addProperty(self):
        """ Unittest function """
        self.s.addProperty('c', 3)
        self.assertIsInstance(self.s.seq[0], Property)
        self.assertEqual(self.s.seq[0].name, 'c')
        self.assertListEqual(self.s.seq[0].requires, [])

    def test_addCollection(self):
        """ Unittest function """
        self.s.addCollection('c', ['a', 'b'])
        self.assertIsInstance(self.s.seq[0], Collection)
        self.assertEqual(self.s.seq[0].name, 'c')
        self.assertListEqual(self.s.seq[0].requires, ['a', 'b'])

    def test_addActor(self):
        """ Unittest function """
        def fun(a, b):
            return {'r': a * sum(b)}
        self.s.addActor(fun, a='a', b=['b1', 'b2'])
        self.assertIsInstance(self.s.seq[0], Actor)
        self.assertEqual(self.s.seq[0].name, 'fun')
        self.assertListEqual(self.s.seq[0].requires, ['a', 'b1', 'b2'])

    def test_hash(self):
        """ Unittest function """
        self.s.addProperty('a', 1)
        self.s.addProperty('b', 2)

        def fun(hash, a, b):
            self.assertEqual(hash, create_hash(['', 1, 2]))
            return {'result': hash}

        self.s.addActor(fun)
        self.s.addNeeded('result')

        path = create_path()
        self.s.run(path, False)

    def test_hash_ignores_none(self):
        """ Unittest function """
        self.s.addProperty('a', 1)
        self.s.addProperty('b', 2)
        self.s.addProperty('c', None)

        def fun(hash, a, b, c):
            self.assertEqual(hash, create_hash(['', 1, 2]))
            return {'result': hash}

        self.s.addActor(fun)
        self.s.addNeeded('result')

        path = create_path()
        self.s.run(path, False)

    def test_simple_run(self):
        """ Unittest function """
        self.s.addProperty('a', 2)
        self.s.addCollection('b1', ['a'])
        self.s.addCollection('b2', ['b1', 'a'])

        def fun(a, b):
            return {'result': 8}
        t = TestFunction(fun)

        self.s.addActor(t, path='path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': 8})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [{'a': 2}, {'b1': {'a': 2}, 'a': 2}]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)
        self.assertEqual(len(path.modules()), 1)
        self.assertEqual(t.calls, 2)

    def test_parallel_run(self):
        """ Unittest function """
        self.s.addProperty('a', 2)
        self.s.addCollection('b1', ['a'])
        self.s.addCollection('b2', ['b1', 'a'])

        def fun(a, b):
            return {'result': 8}
        t = TestFunction(fun)

        self.s.addActor(t, path='path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False, nProcesses=2)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': 8})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [{'a': 2}, {'b1': {'a': 2}, 'a': 2}]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)
        self.assertEqual(len(path.modules()), 1)
        self.assertEqual(t.calls, 2)

    def test_none_is_not_needed(self):
        """ Unittest function """
        self.s.addProperty('a', 2)
        self.s.addCollection('b1', ['a'])
        self.s.addCollection('b2', ['b1', 'a'])

        def fun(a, b):
            return {'result': None}
        t = TestFunction(fun)

        self.s.addActor(t, path='path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': None})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [{'a': 2}, {'b1': {'a': 2}, 'a': 2}]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)

        # But modules should still be not in the path
        # Because fun returns None!
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 2)

    def test_returned_key_NotNeeded_is_not_needed(self):
        """ Unittest function """
        self.s.addProperty('a', 2)
        self.s.addCollection('b1', ['a'])
        self.s.addCollection('b2', ['b1', 'a'])

        def fun(a, b):
            return {'result': 8, '__needed__': False}
        t = TestFunction(fun)

        self.s.addActor(t, path='path', a='a', b=['b1', 'b2'])

        path = create_path()
        self.s.run(path, False)

        # Expect no module in main path
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 1)
        self.assertDictEqual(t.result, {'result': 8, '__needed__': False})
        self.assertDictEqual(t.kwargs, {'a': 2, 'b': [{'a': 2}, {'b1': {'a': 2}, 'a': 2}]})
        self.assertTupleEqual(t.args, tuple())

        # Add needed result and try again
        self.s.addNeeded('result')
        self.s.run(path, False)

        # But modules should still be not in the path
        # Because fun returns NotNeeded!
        self.assertEqual(len(path.modules()), 0)
        self.assertEqual(t.calls, 2)

    def test_optimize_parallel_processing(self):
        """ Unittest function """
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

        self.s.addActor(a1)
        self.s.addActor(a2)
        self.s.addActor(a3)

        self.s.addActor(b1)
        self.s.addActor(b2)
        self.s.addActor(b3)

        self.s.addActor(c1)
        self.s.addActor(c2)
        self.s.addActor(c3)

        self.s.addActor(d)

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
