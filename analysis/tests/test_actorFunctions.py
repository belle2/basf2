#!/usr/bin/env python
# -*- coding: utf-8 -*-

from actorFunctions import *
from FullEventInterpretation import Particle, Preloader

from basf2 import *
import unittest
import os


class MockPath(object):
    """ Unittest class """
    def __init__(self):
        """ Unittest function """
        self.mmodules = []

    def add_module(self, module):
        """ Unittest function """
        self.mmodules.append(module)

    def modules(self):
        """ Unittest function """
        return self.mmodules


class TestCountMCParticles(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        result = CountMCParticles(self.path, ['e+', 'gamma', 'D+'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertListEqual(parameters['variables'], ['NumberOfMCParticlesInEvent(11)', 'NumberOfMCParticlesInEvent(22)', 'NumberOfMCParticlesInEvent(411)'])
        self.assertEqual(parameters['fileName'], 'mcParticlesCount.root')
        self.assertEqual(parameters['treeName'], 'mccounts')


class TestSelectParticleList(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        result = SelectParticleList(self.path, 'hash', Preloader(None), 'dummy', 'e+', 'generic', False)
        print result
        self.assertTrue('RawParticleList_e+:generic' in result)
        self.assertEqual(result['RawParticleList_e+:generic'], 'e+:hash')
        self.assertDictEqual(result, {'RawParticleList_e+:generic': 'e+:hash'})
        self.assertEqual(len(self.path.modules()), 1)

        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['decayString'], 'e+:hash')
        self.assertEqual(parameters['cut'], '')
        self.assertEqual(parameters['persistent'], True)

    def test_roe(self):
        """ Unittest function """
        result = SelectParticleList(self.path, 'hash', Preloader(None), 'dummy', 'e+', 'generic', True)
        print result
        self.assertTrue('RawParticleList_e+:generic' in result)
        self.assertEqual(result['RawParticleList_e+:generic'], 'e+:hash')
        self.assertDictEqual(result, {'RawParticleList_e+:generic': 'e+:hash'})
        self.assertEqual(len(self.path.modules()), 1)

        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['decayString'], 'e+:hash')
        self.assertEqual(parameters['cut'], 'isInRestOfEvent > 0.5')
        self.assertEqual(parameters['persistent'], True)


class TestMakeAndMatchParticleList(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        result = MakeAndMatchParticleList(self.path, 'hash', Preloader(None), 'D+', 'generic', 'D+ChannelUnique', ['pi+', 'K-'], {'cutstring': '0 < M < 10'})
        self.assertTrue('RawParticleList_D+ChannelUnique' in result)
        self.assertEqual(result['RawParticleList_D+ChannelUnique'], 'D+:hash')
        self.assertDictEqual(result, {'RawParticleList_D+ChannelUnique': 'D+:hash'})
        self.assertEqual(len(self.path.modules()), 2)

        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['decayString'], 'D+:hash ==> pi+ K-')
        self.assertEqual(parameters['cut'], '0 < M < 10')
        self.assertEqual(parameters['persistent'], True)

        parameters = {p.name: p.values for p in self.path.modules()[1].available_params()}
        self.assertEqual(parameters['listName'], 'D+:hash')

    def test_missing_precut(self):
        """ Unittest function """
        result = MakeAndMatchParticleList(self.path, 'hash', Preloader(None), 'D+', 'generic', 'D+ChannelUnique', ['pi+', 'K-'], None)
        self.assertDictEqual(result, {'RawParticleList_D+ChannelUnique': None})
        self.assertEqual(len(self.path.modules()), 0)


class TestCopyParticleLists(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        result = CopyParticleLists(self.path, 'hash', Preloader(None), 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3'], {'cutstring': '0.1 < M'})
        self.assertDictEqual(result, {'ParticleList_D+:generic': 'D+:hash',
                                      'ParticleList_D-:generic': 'D-:hash'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['outputListName'], 'D+:hash')
        self.assertListEqual(parameters['inputListNames'], ['D+:1', 'D+:2', 'D+:3'])
        self.assertEqual(parameters['cut'], '0.1 < M')
        self.assertEqual(parameters['persistent'], True)

    def test_some_missing_daughter_lists(self):
        """ Unittest function """
        result = CopyParticleLists(self.path, 'hash', Preloader(None), 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3', None], {'cutstring': '0.1 < M'})
        self.assertDictEqual(result, {'ParticleList_D+:generic': 'D+:hash',
                                      'ParticleList_D-:generic': 'D-:hash'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['outputListName'], 'D+:hash')
        self.assertListEqual(parameters['inputListNames'], ['D+:1', 'D+:2', 'D+:3'])

    def test_all_missing_daughter_lists(self):
        """ Unittest function """
        result = CopyParticleLists(self.path, 'hash', Preloader(None), 'D+', 'generic', [None, None, None], [{'cutstring': '0.1 < M'}] * 3)
        self.assertDictEqual(result, {'ParticleList_D+:generic': None,
                                      'ParticleList_D-:generic': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_all_missing_post_cuts(self):
        """ Unittest function """
        result = CopyParticleLists(self.path, 'hash', Preloader(None), 'D+', 'generic', ['D+:1', 'D+:2', 'D+:3'], None)
        self.assertDictEqual(result, {'ParticleList_D+:generic': 'D+:hash',
                                      'ParticleList_D-:generic': 'D-:hash'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['outputListName'], 'D+:hash')
        self.assertListEqual(parameters['inputListNames'], ['D+:1', 'D+:2', 'D+:3'])
        self.assertEqual(parameters['cut'], '')
        self.assertEqual(parameters['persistent'], True)


class TestLoadGeometry(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        result = LoadGeometry(self.path)
        self.assertDictEqual(result, {'geometry': 'dummy'})
        self.assertEqual(len(self.path.modules()), 2)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        parameters = {p.name: p.values for p in self.path.modules()[1].available_params()}
        self.assertListEqual(parameters['components'], ['MagneticField'])


class TestFitVertex(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        result = FitVertex(self.path, 'hash', Preloader(None), 'UniqueChannelName', 'D+:1', ['v1', 'v2'], 'dummy')
        self.assertDictEqual(result, {'VertexFit_UniqueChannelName': 'hash'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['listName'], 'D+:1')
        self.assertEqual(parameters['confidenceLevel'], 0)

    def test_missing_particle_list(self):
        """ Unittest function """
        result = FitVertex(self.path, 'hash', Preloader(None), 'UniqueChannelName', None, ['v1', 'v2'], 'dummy')
        self.assertDictEqual(result, {'VertexFit_UniqueChannelName': None})
        self.assertEqual(len(self.path.modules()), 0)


mvaConfig = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'],
    target='isSignal'
)

preCutConfig = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 2, 4),
    efficiency=0.7,
    purity=0.01,
    userCut='M > 0.2'
)


class TestCreatePreCutHistogram(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()
        self.standardHash = '590506f812c4754e487d42d6c2a829d745ab70d5'
        self.standardFilename = 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash)
        open(self.standardFilename, 'a').close()

    def tearDown(self):
        """ Unittest function """
        if os.path.isfile(self.standardFilename):
            os.remove(self.standardFilename)

    def test_standard(self):
        """ Unittest function """
        os.remove(self.standardFilename)
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['fileName'], self.standardFilename)
        self.assertEqual(parameters['decayString'], 'D+:' + self.standardHash + ' ==> pi+:1 K+:1')
        self.assertEqual(parameters['variable'], 'M')
        self.assertEqual(parameters['cut'], 'M > 0.2')
        self.assertEqual(parameters['target'], 'isSignal')

    def test_nothing_to_do(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {'PreCutHistogram_UniqueChannelName': (self.standardFilename, 'D+:' + self.standardHash)})
        self.assertEqual(len(self.path.modules()), 0)

    def test_missing_daughter(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, preCutConfig, [None, 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {'PreCutHistogram_UniqueChannelName': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_missing_additionalDependencies(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', None])
        self.assertDictEqual(result, {'PreCutHistogram_UniqueChannelName': None})
        self.assertEqual(len(self.path.modules()), 0)

    def test_hash_depends_on_particle_name(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'B+', 'UniqueChannelName', mvaConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_channel_name(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'D+', 'other', mvaConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_other:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_pre_cut_variable(self):
        """ Unittest function """
        myPreCutConfig = Particle.PreCutConfiguration(
            variable='Q',
            binning=(500, 2, 4),
            efficiency=0.7,
            purity=0.01,
            userCut='M > 0.2'
        )
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, myPreCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_pre_cut_binning(self):
        """ Unittest function """
        myPreCutConfig = Particle.PreCutConfiguration(
            variable='M',
            binning=(400, 2, 4),
            efficiency=0.7,
            purity=0.01,
            userCut='M > 0.2'
        )
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, myPreCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_pre_cut_userCut(self):
        """ Unittest function """
        myPreCutConfig = Particle.PreCutConfiguration(
            variable='M',
            binning=(500, 2, 4),
            efficiency=0.7,
            purity=0.01,
            userCut='M > 0.5'
        )
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, myPreCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_doesnt_depends_on_pre_cut_in_general(self):
        """ Unittest function """
        myPreCutConfig = Particle.PreCutConfiguration(
            variable='M',
            binning=(500, 2, 4),
            efficiency=0.6,
            purity=0.1,
            userCut='M > 0.2'
        )
        os.remove(self.standardFilename)
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, myPreCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] == 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_on_mva_target(self):
        """ Unittest function """
        myMVAConfig = Particle.MVAConfiguration(
            name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
            variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'],
            target='isSignalAcceptMissingNeutrino'
        )
        os.remove(self.standardFilename)
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', myMVAConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_doesnt_depends_on_mva_in_general(self):
        """ Unittest function """
        myMVAConfig = Particle.MVAConfiguration(
            name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
            variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb', 'pz'],
            target='isSignal'
        )
        os.remove(self.standardFilename)
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', myMVAConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] == 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_daughters(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, preCutConfig, ['pi+:1', 'K-:1'], ['bar', 'foo'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))

    def test_hash_depends_on_additional_dependencies(self):
        """ Unittest function """
        result = CreatePreCutHistogram(self.path, 'D+', 'UniqueChannelName', mvaConfig, preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'bla'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertTrue(parameters['fileName'] != 'CutHistograms_UniqueChannelName:{hash}.root'.format(hash=self.standardHash))


class TestPreCutDetermination(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        # TODO Write awesome test
        pass


class TestPostCutDetermination(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()

    def test_standard(self):
        """ Unittest function """
        # TODO Write awesome test
        pass


class TestSignalProbability(unittest.TestCase):
    """ Unittest class """
    def setUp(self):
        """ Unittest function """
        self.path = MockPath()
        self.standardFilename = 'D+:1_{hash}'.format(hash='hash')
        self.preCut = {'nSignal': 100, 'nBackground': 100}
        open(self.standardFilename + '.root', 'a').close()
        open(self.standardFilename + '.config', 'a').close()

    def tearDown(self):
        """ Unittest function """
        if os.path.isfile(self.standardFilename + '.root'):
            os.remove(self.standardFilename + '.root')
        if os.path.isfile(self.standardFilename + '.config'):
            os.remove(self.standardFilename + '.config')

    def test_standard_expert(self):
        """ Unittest function """
        result = SignalProbability(self.path, 'hash', Preloader(None), 'Identifier', 'D+:1', mvaConfig, self.preCut, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {'SignalProbability_Identifier': self.standardFilename + '.config'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['prefix'], 'D+:1_hash')
        self.assertEqual(parameters['method'], mvaConfig.name)
        self.assertEqual(parameters['signalFraction'], -1)
        self.assertEqual(parameters['signalProbabilityName'], 'SignalProbability')
        self.assertEqual(parameters['signalClass'], 1)
        self.assertDictEqual(parameters['inverseSamplingRates'], {})
        self.assertEqual(parameters['listNames'], ['D+:1'])

    def test_standard_expert_sampling(self):
        """ Unittest function """
        result = SignalProbability(self.path, 'hash', Preloader(None), 'Identifier', 'D+:1', mvaConfig, {'nSignal': 1e8, 'nBackground': 2e8}, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {'SignalProbability_Identifier': self.standardFilename + '.config'})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['prefix'], 'D+:1_hash')
        self.assertEqual(parameters['method'], mvaConfig.name)
        self.assertEqual(parameters['signalFraction'], -1)
        self.assertEqual(parameters['signalProbabilityName'], 'SignalProbability')
        self.assertEqual(parameters['signalClass'], 1)
        self.assertDictEqual(parameters['inverseSamplingRates'], {0: 20, 1: 10})
        self.assertEqual(parameters['listNames'], ['D+:1'])

    def test_standard_teacher(self):
        """ Unittest function """
        os.remove(self.standardFilename + '.root')
        os.remove(self.standardFilename + '.config')
        result = SignalProbability(self.path, 'hash', Preloader(None), 'Identifier', 'D+:1', mvaConfig, self.preCut, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['prefix'], 'D+:1_hash')
        self.assertEqual(parameters['methods'], [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        self.assertEqual(parameters['variables'], mvaConfig.variables)
        self.assertEqual(parameters['target'], mvaConfig.target)
        self.assertEqual(parameters['doNotTrain'], True)
        self.assertDictEqual(parameters['inverseSamplingRates'], {})
        self.assertEqual(parameters['listNames'], ['D+:1'])

    def test_standard_teacher_sampling(self):
        """ Unittest function """
        os.remove(self.standardFilename + '.root')
        os.remove(self.standardFilename + '.config')
        result = SignalProbability(self.path, 'hash', Preloader(None), 'Identifier', 'D+:1', mvaConfig, {'nSignal': 1e8, 'nBackground': 2e8}, ['SignalProbabilityHashPi', 'SignalProbabilityHashK'])
        self.assertDictEqual(result, {})
        self.assertEqual(len(self.path.modules()), 1)
        parameters = {p.name: p.values for p in self.path.modules()[0].available_params()}
        self.assertEqual(parameters['prefix'], 'D+:1_hash')
        self.assertEqual(parameters['methods'], [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        self.assertEqual(parameters['variables'], mvaConfig.variables)
        self.assertEqual(parameters['target'], mvaConfig.target)
        self.assertEqual(parameters['doNotTrain'], True)
        self.assertDictEqual(parameters['inverseSamplingRates'], {0: 20, 1: 10})
        self.assertEqual(parameters['listNames'], ['D+:1'])

    def test_missing_additional_dependencies(self):
        """ Unittest function """
        result = SignalProbability(self.path, 'hash', Preloader(None), 'Identifier', 'D+:1', mvaConfig, self.preCut, ['SignalProbabilityHashPi', None])
        self.assertDictEqual(result, {'SignalProbability_Identifier': None,
                                      'SignalProbability_Identifier': None})
        self.assertEqual(len(self.path.modules()), 0)


if __name__ == '__main__':
    unittest.main()
