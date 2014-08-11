#!/usr/bin/env python
# -*- coding: utf-8 -*-

from actorFunctions import *
from FullEventInterpretation import Particle

from basf2 import *
import unittest
import os


class MockPath(object):
    def __init__(self):
        self.mmodules = []

    def add_module(self, module):
        self.mmodules.append(module)

    def modules(self):
        return self.mmodules


class TestCountMCParticles(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_standard(self):
        result = CountMCParticles(self.path, ['e+', 'gamma', 'D+'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertListEqual(parameters['variables'], ['NumberOfMCParticlesInEvent(11)', 'NumberOfMCParticlesInEvent(22)', 'NumberOfMCParticlesInEvent(411)'])
        self.assertEqual(parameters['fileName'], 'mcParticlesCount.root')
        self.assertEqual(parameters['treeName'], 'mccounts')


class TestSelectParticleList(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()
        self.standardHash = '578e965dbcc8088a0f596181d9f662ee6d23c6ab'

    def test_standard(self):
        result = SelectParticleList(self.path, 'e+', 'generic')
        self.assertTrue('RawParticleList_e+:generic' in result)
        self.assertEqual(result['RawParticleList_e+:generic'], 'e+:' + self.standardHash)
        self.assertDictEqual(result, {'RawParticleList_e+:generic': 'e+:' + self.standardHash})
        self.assertEqual(len(self.path.modules()), 1)

        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['decayString'], 'e+:' + self.standardHash)
        self.assertEqual(parameters['cut'], '')
        self.assertEqual(parameters['persistent'], False)

    def test_hash_depends_on_particle_name(self):
        result = SelectParticleList(self.path, 'K+', 'generic')
        self.assertTrue('RawParticleList_K+:generic' in result)
        self.assertTrue(result['RawParticleList_K+:generic'] != 'K+:' + self.standardHash)

    def test_hash_depends_on_particle_label(self):
        result = SelectParticleList(self.path, 'e+', 'other')
        self.assertTrue('RawParticleList_e+:other' in result)
        self.assertTrue(result['RawParticleList_e+:other'] != 'e+:' + self.standardHash)


class TestMakeAndMatchParticleList(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()
        self.standardHash = '1a558c640b3ddffa3a9d6a8eb860897123270b36'

    def test_standard(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'generic', 'D+ChannelUnique', ['pi+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertTrue('RawParticleList_D+ChannelUnique' in result)
        self.assertEqual(result['RawParticleList_D+ChannelUnique'], 'D+:' + self.standardHash)
        self.assertDictEqual(result, {'RawParticleList_D+ChannelUnique': 'D+:' + self.standardHash})
        self.assertEqual(len(self.path.modules()), 2)

        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['decayString'], 'D+:' + self.standardHash + ' ==> pi+ K-')
        self.assertEqual(parameters['cut'], '0 < M < 10')
        self.assertEqual(parameters['persistent'], False)

        parameters = {p.name: p.values for p in self.path.modules()[1].available_params()}
        self.assertEqual(parameters['listName'], 'D+:' + self.standardHash)

    def test_missing_precut(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'generic', 'D+ChannelUnique', ['pi+', 'K-'], None)
        self.assertDictEqual(result, {'RawParticleList_D+ChannelUnique': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_hash_depends_on_particle_name(self):
        result = MakeAndMatchParticleList(self.path, 'B+', 'generic', 'D+ChannelUnique', ['pi+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertTrue('RawParticleList_D+ChannelUnique' in result)
        self.assertTrue(result['RawParticleList_D+ChannelUnique'] != 'B+:' + self.standardHash)

    def test_hash_depends_on_particle_label(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'other', 'D+ChannelUnique', ['pi+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertTrue('RawParticleList_D+ChannelUnique' in result)
        self.assertTrue(result['RawParticleList_D+ChannelUnique'] != 'D+:' + self.standardHash)

    def test_hash_depends_on_channel_name(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'generic', 'other', ['pi+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertTrue('RawParticleList_other' in result)
        self.assertTrue(result['RawParticleList_other'] != 'D+:' + self.standardHash)

    def test_hash_depends_on_daughters(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'generic', 'D+ChannelUnique', ['K+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertTrue('RawParticleList_D+ChannelUnique' in result)
        self.assertTrue(result['RawParticleList_D+ChannelUnique'] != 'D+:' + self.standardHash)

    def test_hash_depends_on_cut(self):
        result = MakeAndMatchParticleList(self.path, 'D+', 'generic', 'D+ChannelUnique', ['pi+', 'K-'], {'cutstring': '0.1 < M < 10'})
        self.assertTrue('RawParticleList_D+ChannelUnique' in result)
        self.assertTrue(result['RawParticleList_D+ChannelUnique'] != 'D+:' + self.standardHash)


class TestCopyParticleLists(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()
        self.standardHash = 'fc5afcc45e39bf38c1923f7992c31fe290c92a7d'

    def test_standard(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3'], [{'cutstring': '0.1 < M'}] * 3)
        self.assertDictEqual(result, {'ParticleList_D+:generic': 'D+:' + self.standardHash,
                                      'ParticleList_D-:generic': 'D-:' + self.standardHash})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['outputListName'], 'D+:' + self.standardHash)
        self.assertListEqual(parameters['inputListNames'], ['D+:1', 'D+:2', 'D+:3'])
        self.assertEqual(parameters['cut'], '0.1 < M')

    def test_some_missing_daughter_lists(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3', None], [{'cutstring': '0.1 < M'}] * 4)
        self.assertDictEqual(result, {'ParticleList_D+:generic': 'D+:' + self.standardHash,
                                      'ParticleList_D-:generic': 'D-:' + self.standardHash})
        self.assertEqual(len(self.path.modules()), 1)

    def test_some_missing_post_cuts(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3', 'D+:4'], [{'cutstring': '0.1 < M'}] * 3 + [None])
        self.assertDictEqual(result, {'ParticleList_D+:generic': 'D+:' + self.standardHash,
                                      'ParticleList_D-:generic': 'D-:' + self.standardHash})
        self.assertEqual(len(self.path.modules()), 1)

    def test_all_missing_daughter_lists(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', [None, None, None], [{'cutstring': '0.1 < M'}] * 3)
        self.assertDictEqual(result, {'ParticleList_D+:generic': None,
                                      'ParticleList_D-:generic': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_all_missing_post_cuts(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3'], [None, None, None])
        self.assertDictEqual(result, {'ParticleList_D+:generic': None,
                                      'ParticleList_D-:generic': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_hash_depends_on_particle_name(self):
        result = CopyParticleLists(self.path, 'B+', 'generic', ['D+:1', 'D+:2', 'D+:3'], [{'cutstring': '0.1 < M'}] * 3)
        self.assertTrue('ParticleList_B+:generic' in result)
        self.assertTrue(result['ParticleList_B+:generic'] != 'B+:' + self.standardHash)

    def test_hash_depends_on_particle_label(self):
        result = CopyParticleLists(self.path, 'D+', 'other', ['D+:1', 'D+:2', 'D+:3'], [{'cutstring': '0.1 < M'}] * 3)
        self.assertTrue('ParticleList_D+:other' in result)
        self.assertTrue(result['ParticleList_D+:other'] != 'D+:' + self.standardHash)

    def test_hash_depends_on_daughters(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', ['D+:1', 'D+:2', 'D+:4'], [{'cutstring': '0.1 < M'}] * 3)
        self.assertTrue('ParticleList_D+:generic' in result)
        self.assertTrue(result['ParticleList_D+:generic'] != 'D+:' + self.standardHash)

    def test_hash_depends_on_post_cuts(self):
        result = CopyParticleLists(self.path, 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3'], [{'cutstring': '0.2 < M'}] * 3)
        self.assertTrue('ParticleList_D+:generic' in result)
        self.assertTrue(result['ParticleList_D+:generic'] != 'D+:' + self.standardHash)


class TestLoadGeometry(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_standard(self):
        result = LoadGeometry(self.path)
        self.assertDictEqual(result, {'Geometry': 'dummy'})
        self.assertEqual(len(self.path.modules()), 2)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        parameters = {p.name: p.values for p in self.path.modules()[1].available_params()}
        self.assertListEqual(parameters['components'], ['MagneticField'])


class TestFitVertex(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()
        self.standardHash = 'f6f3978c4c8f896b3fd5b0e7a2f7302a3cf3669d'

    def test_standard(self):
        result = FitVertex(self.path, 'UniqueChannelName', 'D+:1', ['v1', 'v2'], 'dummy')
        self.assertDictEqual(result, {'VertexFit_UniqueChannelName': self.standardHash})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['listName'], 'D+:1')
        self.assertEqual(parameters['confidenceLevel'], 0)

    def test_missing_particle_list(self):
        result = FitVertex(self.path, 'UniqueChannelName', None, ['v1', 'v2'], 'dummy')
        self.assertDictEqual(result, {'VertexFit_UniqueChannelName': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_hash_depends_on_channel_name(self):
        result = FitVertex(self.path, 'other', 'D+:1', ['v1', 'v2'], 'dummy')
        self.assertTrue('VertexFit_other' in result)
        self.assertTrue(result['VertexFit_other'] != self.standardHash)

    def test_hash_depends_on_particle_list(self):
        result = FitVertex(self.path, 'UniqueChannelName', 'D+:2', ['v1', 'v2'], 'dummy')
        self.assertTrue('VertexFit_UniqueChannelName' in result)
        self.assertTrue(result['VertexFit_UniqueChannelName'] != self.standardHash)


preCutConfig = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.7,
    purity=0.01
)


class TestCreatePreCutHistogram(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()
        self.standardHash = 'b008fcee89f8f867af9d0904e9b2727878948598'
        self.standardFilename = 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash)
        open(self.standardFilename, 'a').close()

    def tearDown(self):
        if os.path.isfile(self.standardFilename):
            os.remove(self.standardFilename)

    def test_standard(self):
        os.remove(self.standardFilename)
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['fileName'], self.standardFilename)
        self.assertEqual(parameters['decayString'], 'D+:' + self.standardHash + ' ==> pi+:1 K+:1')
        self.assertEqual(parameters['variable'], 'M')

    def test_nothing_to_do(self):
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {'PreCutHistogram_UniqueChannelName': (self.standardFilename, 'D+:' + self.standardHash)})
        self.assertEqual(len(self.path.modules()), 0)

    def test_missing_daughter(self):
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', preCutConfig, [None, 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {'PreCutHistogram_UniqueChannelName': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_missing_additionalDependencies(self):
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', preCutConfig, ['pi+:1', 'K+:1'], ['bar', None])
        self.assertDictEqual(result, {'PreCutHistogram_UniqueChannelName': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_hash_depends_on_particle_name(self):
        result = CreatePreCutHistogram(self.path, 'B+', 'UniqueChannelName', preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_channel_name(self):
        result = CreatePreCutHistogram(self.path, 'D+', 'other', preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_other:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_pre_cut_variable(self):
        myPreCutConfig = Particle.PreCutConfiguration(
            variable='Q',
            method='Same',
            efficiency=0.7,
            purity=0.01
        )
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', myPreCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_doesnt_depends_on_pre_cut_in_general(self):
        myPreCutConfig = Particle.PreCutConfiguration(
            variable='M',
            method='S/B',
            efficiency=0.6,
            purity=0.1
        )
        os.remove(self.standardFilename)
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', myPreCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] == 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_daughters(self):
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', preCutConfig, ['pi+:1', 'K-:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_additional_dependencies(self):
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'bla'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))


class TestPreCutDetermination(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_standard(self):
        # TODO Write awesome test
        pass


class TestPostCutDetermination(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()

    def test_standard(self):
        # TODO Write awesome test
        pass


mvaConfig = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'],
    target='isSignal', targetCluster=1
)


class TestSignalProbability(unittest.TestCase):
    def setUp(self):
        self.path = MockPath()
        self.standardHash = '3b16051880492d465eb5f58e7202409bab57ff02'
        self.standardFilename = 'D+:1_{hash}'.format(hash=self.standardHash)
        open(self.standardFilename + '.root', 'a').close()
        open(self.standardFilename + '.config', 'a').close()

    def tearDown(self):
        if os.path.isfile(self.standardFilename + '.root'):
            os.remove(self.standardFilename + '.root')
        if os.path.isfile(self.standardFilename + '.config'):
            os.remove(self.standardFilename + '.config')

    def test_standard_expert(self):
        result = SignalProbability(self.path, 'Identifier', 'D+:1', mvaConfig, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {'SignalProbability_Identifier': self.standardFilename + '.config'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['prefix'], 'D+:1_' + self.standardHash)
        self.assertEqual(parameters['method'], mvaConfig.name)
        self.assertEqual(parameters['signalFraction'], -2)
        self.assertEqual(parameters['signalProbabilityName'], 'SignalProbability')
        self.assertEqual(parameters['signalClass'], mvaConfig.targetCluster)
        self.assertEqual(parameters['listNames'], ['D+:1'])

    def test_standard_teacher(self):
        os.remove(self.standardFilename + '.root')
        os.remove(self.standardFilename + '.config')
        result = SignalProbability(self.path, 'Identifier', 'D+:1', mvaConfig, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['prefix'], 'D+:1_' + self.standardHash)
        self.assertEqual(parameters['methods'], [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        self.assertEqual(parameters['variables'], mvaConfig.variables)
        self.assertEqual(parameters['target'], mvaConfig.target)
        self.assertEqual(parameters['doNotTrain'], True)
        self.assertEqual(parameters['listNames'], ['D+:1'])

    def test_missing_additional_dependencies(self):
        result = SignalProbability(self.path, 'Identifier', 'D+:1', mvaConfig, ['SignalProbabilityHashPi', None])
        self.assertDictEqual(result, {'SignalProbability_Identifier': None,
                                      'SignalProbability_Identifier': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_hash_depends_particle_list(self):
        result = SignalProbability(self.path, 'Identifier', 'B+:1', mvaConfig, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['prefix'] != 'B+:1_' + self.standardHash)

    def test_hash_depends_identifier(self):
        result = SignalProbability(self.path, 'other', 'D+:1', mvaConfig, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['prefix'] != 'D+:1_' + self.standardHash)

    def test_hash_depends_additional_dependencies(self):
        result = SignalProbability(self.path, 'Identifier', 'D+:1', mvaConfig, ['SignalProbabilityHashPi', 'test'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['prefix'] != 'D+:1_' + self.standardHash)

    def test_hash_depends_mva_config(self):
        myMvaConfig = Particle.MVAConfiguration(
            name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
            variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'],
            target='isSignal', targetCluster=0
        )
        result = SignalProbability(self.path, 'Identifier', 'D+:1', myMvaConfig, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['prefix'] != 'D+:1_' + self.standardHash)


if __name__ == '__main__':
    unittest.main()
