#!/usr/bin/env python
# -*- coding: utf-8 -*-

from actorFunctions import *
from FullEventInterpretation import Particle

from basf2 import *
import unittest
import os


class MockPath(object):
    def __init__(self):
        self.modules = []

    def add_module(self, module):
        self.modules.append(module)


class TestSelectParticleList(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_flavour_specific(self):
        result = SelectParticleList(self.path, 'e+')
        self.assertDictEqual(result, {'RawParticleList_e+': 'e+:f85be0cdd943c9ad2f4d050d79ef49a870352bba',
                                      'RawParticleList_e-': 'e-:f85be0cdd943c9ad2f4d050d79ef49a870352bba'})
        self.assertEqual(len(self.path.modules), 1)

    def test_self_conjugated(self):
        result = SelectParticleList(self.path, 'J/Psi')
        self.assertDictEqual(result, {'RawParticleList_J/Psi': 'J/Psi:8534a63998eabdb26f4cda1cb905ab2ab0aa4a2c',
                                      'RawParticleList_J/Psi': 'J/Psi:8534a63998eabdb26f4cda1cb905ab2ab0aa4a2c'})
        self.assertEqual(len(self.path.modules), 1)


class TestCopyParticleLists(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_without_nones(self):
        result = CopyParticleLists(self.path, 'D+', None, ['D+:1', 'D+:2', 'D+:3'], {'cutstring': ''}, None)
        self.assertDictEqual(result, {'ParticleList_D+': 'D+:5cd7f2d37f66c44b92dbd64e10ada329133b6a63',
                                      'ParticleList_D-': 'D-:5cd7f2d37f66c44b92dbd64e10ada329133b6a63'})
        self.assertEqual(len(self.path.modules), 1)

    def test_with_nones(self):
        result = CopyParticleLists(self.path, 'D+', None, ['D+:1', None, 'D+:3', None], {'cutstring': ''}, None)
        self.assertDictEqual(result, {'ParticleList_D+': 'D+:fc01399545fea42891ffc8fc0b07b52de3317544',
                                      'ParticleList_D-': 'D-:fc01399545fea42891ffc8fc0b07b52de3317544'})
        self.assertEqual(len(self.path.modules), 1)

    def test_only_nones(self):
        result = CopyParticleLists(self.path, 'D+', None, [None, None], {'cutstring': ''}, None)
        self.assertDictEqual(result, {'ParticleList_D+': None,
                                      'ParticleList_D-': None})
        self.assertEqual(len(self.path.modules), 0)


class TestMakeAndMatchParticleList(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_with_precut(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'D+ -> pi+ K-', ['pi+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertDictEqual(result, {'RawParticleList_D+ -> pi+ K-_D+': 'D+:a82b536fc03a4a03b3e7f73db062250ec59a1f4f',
                                      'RawParticleList_D+ -> pi+ K-_D-': 'D-:a82b536fc03a4a03b3e7f73db062250ec59a1f4f'})
        self.assertEqual(len(self.path.modules), 2)

    def test_without_precut(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'D+ -> pi+ K-', ['pi+', 'K-'], None)
        self.assertDictEqual(result, {'RawParticleList_D+ -> pi+ K-_D+': None,
                                      'RawParticleList_D+ -> pi+ K-_D-': None})
        self.assertEqual(len(self.path.modules), 0)


mvaConfig = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'],
    target='isSignal', targetCluster=1
)


class TestSignalProbability(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_non_fsp_teacher(self):
        hash = '3727bb77c7f7f5577ea2eade97adea69e3eec95e'
        filename = 'D+:1_{hash}'.format(hash=hash)
        open(filename + '.root', 'a').close()
        open(filename + '.config', 'a').close()
        os.remove(filename + '.root')
        os.remove(filename + '.config')
        result = SignalProbability(self.path, 'D+', 'D+ -> pi+ K-', mvaConfig, 'D+:1', ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules), 1)

    def test_non_fsp_expert(self):
        hash = '3727bb77c7f7f5577ea2eade97adea69e3eec95e'
        filename = 'D+:1_{hash}'.format(hash=hash)
        open(filename + '.root', 'a').close()
        open(filename + '.config', 'a').close()
        result = SignalProbability(self.path, 'D+', 'D+ -> pi+ K-', mvaConfig, 'D+:1', ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {'SignalProbability_D+ -> pi+ K-_D+': 'D+:1_' + hash + '.config',
                                      'SignalProbability_D+ -> pi+ K-_D-': 'D+:1_' + hash + '.config'})
        os.remove(filename + '.root')
        os.remove(filename + '.config')
        self.assertEqual(len(self.path.modules), 1)

    def test_non_fsp_with_nones(self):
        result = SignalProbability(self.path, 'D+', 'D+ -> pi+ K-', mvaConfig, 'D+:1', [None, 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {'SignalProbability_D+ -> pi+ K-_D+': None,
                                      'SignalProbability_D+ -> pi+ K-_D-': None})
        self.assertEqual(len(self.path.modules), 0)

    def test_fsp_teacher(self):
        hash = 'd02f42dce674a689665bce945e81cfafe92c547f'
        filename = 'e+:1_{hash}'.format(hash=hash)
        open(filename + '.root', 'a').close()
        open(filename + '.config', 'a').close()
        os.remove(filename + '.root')
        os.remove(filename + '.config')
        result = SignalProbability(self.path, 'e+', 'e+', mvaConfig, 'e+:1')
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules), 1)

    def test_fsp_expert(self):
        hash = 'd02f42dce674a689665bce945e81cfafe92c547f'
        filename = 'e+:1_{hash}'.format(hash=hash)
        open(filename + '.root', 'a').close()
        open(filename + '.config', 'a').close()
        result = SignalProbability(self.path, 'e+', 'e+', mvaConfig, 'e+:1')
        os.remove(filename + '.root')
        os.remove(filename + '.config')
        self.assertDictEqual(result, {'SignalProbability_e+': 'e+:1_' + hash + '.config',
                                      'SignalProbability_e-': 'e+:1_' + hash + '.config'})
        self.assertEqual(len(self.path.modules), 1)


preCutConfig = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.7,
    purity=0.01
)


class TestCreatePreCutHistogram(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_create_hist(self):
        hash = 'f75ee3533d3c9475373e59ef33e236faf7548a39'
        filename = 'CutHistograms_D+_D+ -> pi+ K-_{hash}.root'.format(hash=hash)
        open(filename, 'a').close()
        os.remove(filename)
        result = CreatePreCutHistogram(self.path, 'D+', 'D+ -> pi+ K-', preCutConfig, ['pi+:1', 'K+:1'], [])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules), 1)

    def test_nothing_to_do(self):
        hash = 'b5119ce3e4709ebb035b7fec050621d1354c2b32'
        filename = 'CutHistograms_D+_D+ -> pi+ K-_{hash}.root'.format(hash=hash)
        open(filename, 'a').close()
        result = CreatePreCutHistogram(self.path, 'D+', 'D+ -> pi+ K-', preCutConfig, ['pi+:1', 'K+:1'], [])
        os.remove(filename)
        self.assertDictEqual(result, {'PreCutHistogram_D+ -> pi+ K-': (filename, 'D+:' + hash)})
        self.assertEqual(len(self.path.modules), 0)

    def test_create_hist_additionalDependency(self):
        hash = 'ddd94f65a16fe9d92343ff57256fd013684d56e1'
        filename = 'CutHistograms_D+_D+ -> pi+ K-_{hash}.root'.format(hash=hash)
        open(filename, 'a').close()
        os.remove(filename)
        result = CreatePreCutHistogram(self.path, 'D+', 'D+ -> pi+ K-', preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules), 1)

    def test_non_in_daughter(self):
        hash = 'f75ee3533d3c9475373e59ef33e236faf7548a39'
        filename = 'CutHistograms_D+_D+ -> pi+ K-_{hash}.root'.format(hash=hash)
        open(filename, 'a').close()
        os.remove(filename)
        result = CreatePreCutHistogram(self.path, 'D+', 'D+ -> pi+ K-', preCutConfig, [None, 'K+:1'], [])
        self.assertDictEqual(result, {'PreCutHistogram_D+ -> pi+ K-': None})
        self.assertEqual(len(self.path.modules), 0)

    def test_non_in_additionalDependencies(self):
        hash = 'ddd94f65a16fe9d92343ff57256fd013684d56e1'
        filename = 'CutHistograms_D+_D+ -> pi+ K-_{hash}.root'.format(hash=hash)
        open(filename, 'a').close()
        os.remove(filename)
        result = CreatePreCutHistogram(self.path, 'D+', 'D+ -> pi+ K-', preCutConfig, [None, 'K+:1'], ['bar', None])
        self.assertDictEqual(result, {'PreCutHistogram_D+ -> pi+ K-': None})
        self.assertEqual(len(self.path.modules), 0)


if __name__ == '__main__':
    unittest.main()
