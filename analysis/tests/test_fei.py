#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import unittest
import unittest.mock
import os
import tempfile
import atexit
import shutil
import contextlib
import IPython
import subprocess
import pickle
import ROOT

import fei.config
import fei.dag
import fei.provider

import numpy as np

import basf2_mva

# @cond

# Define equality operators for a bunch of pybasf2 classes
import pybasf2
pybasf2.Module.__eq__ = lambda a, b: a.type() == b.type() and\
    all(x == y for x, y in zip(a.available_params(), b.available_params()))
pybasf2.ModuleParamInfo.__eq__ = lambda a, b: a.name == b.name and a.values == b.values
pybasf2.Path.__eq__ = lambda a, b: all(x == y for x, y in zip(a.modules(), b.modules()))

fei.provider.MaximumNumberOfMVASamples = int(1e7)
fei.provider.MinimumNumberOfMVASamples = int(10)


class TestMVAConfiguration(unittest.TestCase):

    def test_Defaults(self):
        config = fei.config.MVAConfiguration()
        self.assertEqual(config.method, 'FastBDT')
        self.assertEqual(config.config, '--nTrees 400  --nCutLevels 10 --nLevels 3 --shrinkage 0.1 --randRatio 0.5')
        # We want None, because [] can have nasty side effects when used as default parameter!
        self.assertEqual(config.variables, None)
        self.assertEqual(config.target, 'isSignal')
        self.assertEqual(config.sPlotVariable, None)


class TestPreCutConfiguration(unittest.TestCase):

    def test_Defaults(self):
        config = fei.config.PreCutConfiguration()
        self.assertEqual(config.userCut, '')
        self.assertEqual(config.vertexCut, -2)
        self.assertEqual(config.bestCandidateVariable, None)
        self.assertEqual(config.bestCandidateCut, 0)
        self.assertEqual(config.bestCandidateMode, 'lowest')


class TestPostCutConfiguration(unittest.TestCase):

    def test_Defaults(self):
        config = fei.config.PostCutConfiguration()
        self.assertEqual(config.value, 0.0)
        self.assertEqual(config.bestCandidateCut, 0)


class TestDecayChannel(unittest.TestCase):

    def test_Defaults(self):
        channel = fei.config.DecayChannel(name='D0 ==> K- pi+', daughters=['K-', 'pi+'],
                                          mvaConfig=fei.config.MVAConfiguration(variables=['E']),
                                          preCutConfig=fei.config.PreCutConfiguration(userCut='test'),
                                          decayModeID=23)
        self.assertEqual(channel.name, 'D0 ==> K- pi+')
        self.assertEqual(channel.daughters, ['K-', 'pi+'])
        self.assertEqual(channel.mvaConfig, fei.config.MVAConfiguration(variables=['E']))
        self.assertEqual(channel.preCutConfig, fei.config.PreCutConfiguration(userCut='test'))
        self.assertEqual(channel.decayModeID, 23)


class TestParticle(unittest.TestCase):

    def test_Defaults(self):
        mvaConfig = fei.config.MVAConfiguration(variables=['E'])
        preCutConfig = fei.config.PreCutConfiguration()
        postCutConfig = fei.config.PostCutConfiguration()
        particle = fei.config.Particle('D0', mvaConfig)

        self.assertEqual(particle.identifier, 'D0:generic')
        self.assertEqual(particle.name, 'D0')
        self.assertEqual(particle.label, 'generic')
        self.assertEqual(particle.mvaConfig, mvaConfig)
        self.assertEqual(particle.preCutConfig, preCutConfig)
        self.assertEqual(particle.postCutConfig, postCutConfig)
        self.assertListEqual(particle.channels, [])
        self.assertListEqual(particle.daughters, [])

    def test_NonDefaults(self):
        mvaConfig = fei.config.MVAConfiguration(variables=['E'])
        preCutConfig = fei.config.PreCutConfiguration(userCut='test')
        postCutConfig = fei.config.PostCutConfiguration(value=0.1)
        particle = fei.config.Particle('D0', mvaConfig, preCutConfig, postCutConfig)

        self.assertEqual(particle.identifier, 'D0:generic')
        self.assertEqual(particle.name, 'D0')
        self.assertEqual(particle.label, 'generic')
        self.assertEqual(particle.mvaConfig, mvaConfig)
        self.assertEqual(particle.preCutConfig, preCutConfig)
        self.assertEqual(particle.postCutConfig, postCutConfig)
        self.assertListEqual(particle.channels, [])
        self.assertListEqual(particle.daughters, [])

    def test_Label(self):
        mvaConfig = fei.config.MVAConfiguration(variables=['E'])
        particle = fei.config.Particle('D0:specific', mvaConfig)
        self.assertEqual(particle.identifier, 'D0:specific')
        self.assertEqual(particle.name, 'D0')
        self.assertEqual(particle.label, 'specific')

    def test_Equality(self):
        a = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        a.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        a.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertEqual(a, b)

        b = fei.config.Particle('D+', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertNotEqual(a, b)

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertNotEqual(a, b)

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertNotEqual(a, b)

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertNotEqual(a, b)

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration())
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='test'))
        self.assertNotEqual(a, b)

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='test'), fei.config.PostCutConfiguration(value=0.1))
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertNotEqual(a, b)

        b = fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})']),
                                fei.config.PreCutConfiguration(userCut='dummy'), fei.config.PostCutConfiguration())
        b.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        b.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        self.assertNotEqual(a, b)

    def test_AddChannel(self):
        mvaConfig = fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)', 'daughterAngle({}, {})'])
        preCutConfig = fei.config.PreCutConfiguration(userCut='test')
        postCutConfig = fei.config.PostCutConfiguration()
        particle = fei.config.Particle('D0', mvaConfig, preCutConfig, postCutConfig)

        particle.addChannel(['pi-', 'pi+'], mvaConfig=fei.config.MVAConfiguration(variables=['E', 'daughter({}, p)']))
        particle.addChannel(['K-', 'pi+'], preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'))
        particle.addChannel(['K-:specific', 'pi+', 'pi0'])
        particle.addChannel(['K-', 'K+'])

        channels = []
        channels.append(fei.config.DecayChannel(name='D0:generic ==> pi-:generic pi+:generic',
                                                daughters=['pi-:generic', 'pi+:generic'],
                                                mvaConfig=fei.config.MVAConfiguration(variables=['E',
                                                                                                 'daughter(0, p)',
                                                                                                 'daughter(1, p)']),
                                                preCutConfig=fei.config.PreCutConfiguration(userCut='test'),
                                                decayModeID=0))
        channels.append(fei.config.DecayChannel(name='D0:generic ==> K-:generic pi+:generic',
                                                daughters=['K-:generic', 'pi+:generic'],
                                                mvaConfig=fei.config.MVAConfiguration(variables=['E',
                                                                                                 'daughter(0, p)',
                                                                                                 'daughter(1, p)',
                                                                                                 'daughterAngle(0, 1)']),
                                                preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'),
                                                decayModeID=1))
        channels.append(fei.config.DecayChannel(name='D0:generic ==> K-:specific pi+:generic pi0:generic',
                                                daughters=['K-:specific', 'pi+:generic', 'pi0:generic'],
                                                mvaConfig=fei.config.MVAConfiguration(variables=['E',
                                                                                                 'daughter(0, p)',
                                                                                                 'daughter(1, p)',
                                                                                                 'daughter(2, p)',
                                                                                                 'daughterAngle(0, 1)',
                                                                                                 'daughterAngle(0, 2)',
                                                                                                 'daughterAngle(1, 2)']),
                                                preCutConfig=fei.config.PreCutConfiguration(userCut='test'),
                                                decayModeID=2))
        channels.append(fei.config.DecayChannel(name='D0:generic ==> K-:generic K+:generic',
                                                daughters=['K-:generic', 'K+:generic'],
                                                mvaConfig=fei.config.MVAConfiguration(variables=['E',
                                                                                                 'daughter(0, p)',
                                                                                                 'daughter(1, p)',
                                                                                                 'daughterAngle(0, 1)']),
                                                preCutConfig=fei.config.PreCutConfiguration(userCut='test'),
                                                decayModeID=3))
        self.assertEqual(particle.channels, channels)
        self.assertEqual(sorted(particle.daughters), sorted(['K+:generic', 'K-:generic', 'pi+:generic',
                                                             'pi-:generic', 'pi0:generic', 'K-:specific']))


class TestVariables2MonitoringBinning(unittest.TestCase):

    def test_Variables2MonitoringBinning1D(self):
        self.assertEqual(fei.config.variables2binnings(['mcErrors']), [('mcErrors', 513, -0.5, 512.5)])
        self.assertEqual(fei.config.variables2binnings(['default']), [('default', 100, -10.0, 10.0)])

    def test_Variables2MonitoringBinning2D(self):
        self.assertEqual(fei.config.variables2binnings_2d([('mcErrors', 'default')]),
                         [('mcErrors', 513, -0.5, 512.5, 'default', 100, -10.0, 10.0)])
        self.assertEqual(fei.config.variables2binnings_2d([('default', 'default'), ('mcErrors', 'mcErrors')]),
                         [('default', 100, -10.0, 10.0, 'default', 100, -10.0, 10.0),
                          ('mcErrors', 513, -0.5, 512.5, 'mcErrors', 513, -0.5, 512.5)])


class TestHashRequirements(unittest.TestCase):

    def test_HashRequirements(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, hash='42')
        self.assertEqual(fei.provider.HashRequirements(resource), '42')
        self.assertEqual(resource.cache, True)


class TestPDGConjugate(unittest.TestCase):

    def test_PDGConjugate(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource)
        self.assertEqual(fei.provider.PDGConjugate(resource, 'e+:generic'), 'e-:generic')
        self.assertEqual(resource.cache, True)

        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource)
        self.assertEqual(fei.provider.PDGConjugate(resource, 'J/psi:generic'), 'J/psi:generic')
        self.assertEqual(resource.cache, True)


class TestLoadParticles(unittest.TestCase):

    def test_GenericMode(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'ROE': False, 'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        self.assertIs(fei.provider.LoadParticles(resource, ['K+', 'mu+', 'D0']), '42')

        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                                                ('mu+:FSP', ''), ('gamma:FSP', ''), ('K_S0:V0', ''),
                                                                ('p+:FSP', ''), ('K_L0:FSP', ''), ('Lambda0:FSP', '')],
                        writeOut=True)
        path.add_module('ParticleLoader', decayStringsWithCuts=[('gamma:V0', '')], addDaughters=True, writeOut=True)
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_SpecificMode(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'ROE': 'B+', 'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        self.assertIs(fei.provider.LoadParticles(resource, ['K+', 'mu+', 'D0']), '42')

        cut = 'isInRestOfEvent > 0.5'
        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', cut), ('pi+:FSP', cut), ('e+:FSP', cut),
                                                                ('mu+:FSP', cut), ('gamma:FSP', cut), ('K_S0:V0', cut),
                                                                ('p+:FSP', cut), ('K_L0:FSP', cut), ('Lambda0:FSP', cut)],
                        writeOut=True)
        path.add_module('ParticleLoader', decayStringsWithCuts=[('gamma:V0', cut)], addDaughters=True, writeOut=True)
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'ROE': False, 'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        self.assertIs(fei.provider.LoadParticles(resource, ['K+', 'mu+', 'D0']), '42')

        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                                                ('mu+:FSP', ''), ('gamma:FSP', ''), ('K_S0:V0', ''),
                                                                ('p+:FSP', ''), ('K_L0:FSP', ''), ('Lambda0:FSP', '')],
                        writeOut=True)
        path.add_module('ParticleLoader', decayStringsWithCuts=[('gamma:V0', '')], addDaughters=True, writeOut=True)
        hist_variables = [('NumberOfMCParticlesInEvent({i})'.format(i=pdgcode), 100, -0.5, 99.5) for pdgcode in [321, 13, 421]]
        path.add_module('VariablesToHistogram', particleList='',
                        variables=hist_variables,
                        fileName='Monitor_MCCounts.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestMakeParticleList(unittest.TestCase):

    def test_MakeParticleList(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable=None)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'D0', ['K-', 'pi+'], preCutConfig, mvaConfig, 23), 'D0:42')

        path = basf2.create_path()
        path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True, decayMode=23, cut='1.5 < M < 2.0')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_BestCandidateLowest(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable='dM',
                                                     bestCandidateCut=10, bestCandidateMode='lowest')
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'D0', ['K-', 'pi+'], preCutConfig, mvaConfig, 23), 'D0:42')

        path = basf2.create_path()
        path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                        decayMode=23, cut='1.5 < M < 2.0')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='dM',
                        selectLowest=True, numBest=10, outputVariable='preCut_rank')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_BestCandidateHighest(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable='dM',
                                                     bestCandidateCut=20, bestCandidateMode='highest')
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'D0', ['K-', 'pi+'], preCutConfig, mvaConfig, 23), 'D0:42')

        path = basf2.create_path()
        path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True, decayMode=23, cut='1.5 < M < 2.0')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='dM',
                        selectLowest=False, numBest=20, outputVariable='preCut_rank')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_DaughterParticleListContainsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable=None)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'D0', ['K-', 'pi+', None], preCutConfig, mvaConfig, 23), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_MakeFinalStateParticle(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable=None)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'e+', ['e+:FSP'], preCutConfig, mvaConfig, 23), 'e+:42')

        path = basf2.create_path()
        path.add_module('ParticleListManipulator', inputListNames=['e+:FSP'], outputListName='e+:42',
                        cut='1.5 < M < 2.0', writeOut=True)
        path.add_module('VariablesToExtraInfo', particleList='e+:42', variables={'constant(23)': 'decayModeID'})
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_MonitoringWithBestCandidate(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable='dM',
                                                     bestCandidateCut=10, bestCandidateMode='lowest')
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'D0', ['K-', 'pi+'], preCutConfig, mvaConfig, 23), 'D0:42')

        path = basf2.create_path()
        path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                        decayMode=23, cut='1.5 < M < 2.0')
        path.add_module('MCMatcherParticles', listName='D0:42')

        hist_variables = ['mcErrors', 'mcParticleStatus', 'isSignal', 'dM']
        hist_variables_2d = [('dM', mvaConfig.target),
                             ('dM', 'mcErrors'),
                             ('dM', 'mcParticleStatus')]
        path.add_module('VariablesToHistogram', particleList='D0:42',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_MakeParticleList_BeforeRanking_D0:42.root')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='dM',
                        selectLowest=True, numBest=10, outputVariable='preCut_rank')

        hist_variables += ['extraInfo(preCut_rank)']
        hist_variables_2d += [('extraInfo(preCut_rank)', mvaConfig.target),
                              ('extraInfo(preCut_rank)', 'mcErrors'),
                              ('extraInfo(preCut_rank)', 'mcParticleStatus')]
        path.add_module('VariablesToHistogram', particleList='D0:42',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_MakeParticleList_AfterRanking_D0:42.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_MonitoringWithoutBestCandidate(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration,
                                                     userCut='1.5 < M < 2.0', bestCandidateVariable=None)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.MakeParticleList(resource, 'D0', ['K-', 'pi+'], preCutConfig, mvaConfig, 23), 'D0:42')

        path = basf2.create_path()
        path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                        decayMode=23, cut='1.5 < M < 2.0')
        path.add_module('MCMatcherParticles', listName='D0:42')

        hist_variables = ['mcErrors', 'mcParticleStatus', 'isSignal']
        hist_variables_2d = []
        path.add_module('VariablesToHistogram', particleList='D0:42',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_MakeParticleList_BeforeRanking_D0:42.root')

        hist_variables += ['extraInfo(preCut_rank)']
        hist_variables_2d += [('extraInfo(preCut_rank)', mvaConfig.target),
                              ('extraInfo(preCut_rank)', 'mcErrors'),
                              ('extraInfo(preCut_rank)', 'mcParticleStatus')]
        path.add_module('VariablesToHistogram', particleList='D0:42',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_MakeParticleList_AfterRanking_D0:42.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestMatchParticleList(unittest.TestCase):

    def test_Generic(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False, 'ROE': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.MatchParticleList(resource, 'e+:generic', mvaConfig), 'e+:generic')

        path = basf2.create_path()
        path.add_module('MCMatcherParticles', listName='e+:generic')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.condition, ('EventType', '==0'))

    def test_ParticleListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False, 'ROE': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.MatchParticleList(resource, None, mvaConfig), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False, 'ROE': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.MatchParticleList(resource, 'e+:generic', mvaConfig), 'e+:generic')

        path = basf2.create_path()
        path.add_module('MCMatcherParticles', listName='e+:generic')
        path.add_module('VariablesToHistogram', particleList='e+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([]),
                        fileName='Monitor_MatchParticleList_AfterMatch_e+:generic.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.condition, ('EventType', '==0'))


class TestCopyParticleLists(unittest.TestCase):

    def test_CopyParticleLists(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        postCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PostCutConfiguration, value=0.1, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyParticleLists(resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                                        postCutConfig, mvaConfig, ['S1', 'S2', 'S3']), 'D0:42')
        path = basf2.create_path()
        path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:2', 'D0:3'], writeOut=True)
        path.add_module('ParticleSelector', decayString='D0:42', cut='0.1 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        postCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PostCutConfiguration, value=0.1, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyParticleLists(resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                                        postCutConfig, mvaConfig, ['S1', 'S2', 'S3']), 'D0:42')
        path = basf2.create_path()
        hist_variables = ['mcErrors', 'mcParticleStatus', 'isSignal', 'extraInfo(SignalProbability)', 'extraInfo(decayModeID)']
        hist_variables_2d = [('extraInfo(decayModeID)', mvaConfig.target),
                             ('extraInfo(decayModeID)', 'mcErrors'),
                             ('extraInfo(decayModeID)', 'mcParticleStatus')]
        path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:2', 'D0:3'], writeOut=True)
        path.add_module('VariablesToHistogram', particleList='D0:42',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_CopyParticleList_BeforeCut_D0:42.root')
        path.add_module('ParticleSelector', decayString='D0:42', cut='0.1 < extraInfo(SignalProbability)')
        path.add_module('VariablesToHistogram', particleList='D0:42', variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_CopyParticleList_BeforeRanking_D0:42.root')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        hist_variables_2d += [('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                              ('isSignal', 'extraInfo(postCut_rank)'),
                              ('mcErrors', 'extraInfo(postCut_rank)'),
                              ('mcParticleStatus', 'extraInfo(postCut_rank)')]
        path.add_module('VariablesToHistogram', particleList='D0:42',
                        variables=fei.config.variables2binnings(hist_variables + ['extraInfo(postCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_CopyParticleList_AfterRanking_D0:42.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_OneDaughterListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        postCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PostCutConfiguration, value=0.1, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyParticleLists(resource, 'D0', 'generic', ['D0:1', None, 'D0:3'],
                                                        postCutConfig, mvaConfig, ['S1', 'S2', 'S3']), 'D0:42')
        path = basf2.create_path()
        path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:3'], writeOut=True)
        path.add_module('ParticleSelector', decayString='D0:42', cut='0.1 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_OneSignalProbabilityIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        postCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PostCutConfiguration, value=0.1, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyParticleLists(resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                                        postCutConfig, mvaConfig, ['S1', None, 'S3']), 'D0:42')
        path = basf2.create_path()
        path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:3'], writeOut=True)
        path.add_module('ParticleSelector', decayString='D0:42', cut='0.1 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='D0:42', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_AllDaughterListsAreNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        postCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PostCutConfiguration, value=0.1, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyParticleLists(resource, 'D0', 'generic', [None, None, None],
                                                        postCutConfig, mvaConfig, ['S1', 'S2', 'S3']), None)
        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_AllSignalProbabilitiesAreNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        postCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PostCutConfiguration, value=0.1, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyParticleLists(resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                                        postCutConfig, mvaConfig, [None, None, None]), None)
        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestCopyIntoHumanReadableParticleList(unittest.TestCase):

    def test_CopyIntoHumanReadableParticleList(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyIntoHumanReadableParticleList(resource, 'D0', 'human', mvaConfig, 'D0:123'), 'D0:human')
        path = basf2.create_path()
        path.add_module('ParticleListManipulator', outputListName='D0:human', inputListNames=['D0:123'], writeOut=True)
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_ParticleListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyIntoHumanReadableParticleList(resource, 'D0', 'human', mvaConfig, None), None)
        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.CopyIntoHumanReadableParticleList(resource, 'D0', 'human', mvaConfig, 'D0:123'), 'D0:human')
        path = basf2.create_path()
        path.add_module('ParticleListManipulator', outputListName='D0:human', inputListNames=['D0:123'], writeOut=True)
        hist_variables = ['extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'mcParticleStatus', 'isSignal',
                          'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)']
        path.add_module('VariablesToNtuple', particleList='D0:human', variables=hist_variables, treeName='variables',
                        fileName='Monitor_Final_D0:human.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestFitVertex(unittest.TestCase):

    def test_FitVertex(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, vertexCut=0.1)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.FitVertex(resource, 'D0 -> K+ pi-', 'D0:23', mvaConfig, preCutConfig), '42')
        path = basf2.create_path()
        path.add_module('ParticleVertexFitter', listName='D0:23', confidenceLevel=0.1,
                        vertexFitter='kfitter', fitType='vertex')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_ParticleListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, vertexCut=-2)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.FitVertex(resource, 'D0 -> K+ pi-', None, mvaConfig, preCutConfig), None)
        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, vertexCut=-2)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.FitVertex(resource, 'D0 -> K+ pi-', 'D0:23', mvaConfig, preCutConfig), '42')
        path = basf2.create_path()
        path.add_module('VariablesToHistogram', particleList='D0:23',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([]),
                        fileName='Monitor_FitVertex_Before_D0:23.root')
        path.add_module('ParticleVertexFitter', listName='D0:23', confidenceLevel=-2,
                        vertexFitter='kfitter', fitType='vertex')
        hist_variables = ['mcErrors', 'chiProb', 'mcParticleStatus', 'isSignal']
        hist_variables_2d = [('chiProb', 'isSignal'),
                             ('chiProb', 'mcErrors'),
                             ('chiProb', 'mcParticleStatus')]
        path.add_module('VariablesToHistogram', particleList='D0:23',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_FitVertex_After_D0:23.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_WithTwoPi0(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, vertexCut=0.1)
        mvaConfig = unittest.mock.NonCallableMock(target='isSignal')
        self.assertEqual(fei.provider.FitVertex(resource, 'D0 -> K+ pi- pi0 pi0', 'D0:23', mvaConfig, preCutConfig), None)
        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


@contextlib.contextmanager
def temporary_file(filename):
    open(filename, 'a').close()
    yield
    os.remove(filename)


class TestGenerateTrainingData(unittest.TestCase):

    def test_GenerateTrainingData(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mcCounts = {421: {'sum': 100}, 0: {'sum': 100}}
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.GenerateTrainingData(resource, 'D0', 'D0:1', mcCounts, preCutConfig, mvaConfig), None)

        path = basf2.create_path()
        path.add_module('VariablesToNtuple', fileName='D0:1_42.root', treeName='variables', variables=['p', 'pt', 'isSignal'],
                        particleList='D0:1', sampling=('isSignal', {}))
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.halt, True)
        self.assertEqual(resource.condition, ('EventType', '==0'))

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        mcCounts = {421: {'sum': 100}, 0: {'sum': 100}}
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.GenerateTrainingData(resource, 'D0', 'D0:1', mcCounts, preCutConfig, mvaConfig), None)

        path = basf2.create_path()
        hist_variables = ['mcErrors', 'mcParticleStatus', 'p', 'pt', 'isSignal']
        hist_variables_2d = [('p', 'isSignal'), ('pt', 'isSignal')]
        path.add_module('VariablesToHistogram', particleList='D0:1',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_GenerateTrainingData_D0:1.root')
        path.add_module('VariablesToNtuple', fileName='D0:1_42.root', treeName='variables', variables=['p', 'pt', 'isSignal'],
                        particleList='D0:1', sampling=('isSignal', {}))
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.halt, True)
        self.assertEqual(resource.condition, ('EventType', '==0'))

    def test_InverseSamplingRates(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mcCounts = {421: {'sum': 5e9}, 0: {'sum': 1e7}}
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.GenerateTrainingData(resource, 'D0', 'D0:1', mcCounts, preCutConfig, mvaConfig), None)

        path = basf2.create_path()
        path.add_module('VariablesToNtuple', fileName='D0:1_42.root', treeName='variables', variables=['p', 'pt', 'isSignal'],
                        particleList='D0:1', sampling=('isSignal', {0: 11, 1: 2}))
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.halt, True)
        self.assertEqual(resource.condition, ('EventType', '==0'))

    def test_DataFileAlreadyAvailable(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mcCounts = {421: {'sum': 100}, 0: {'sum': 1e7}}
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  target='isSignal', sPlotVariable=None)
        with temporary_file('D0:1_42.root'):
            self.assertEqual(fei.provider.GenerateTrainingData(resource, 'D0', 'D0:1', mcCounts, preCutConfig, mvaConfig),
                             'D0:1_42')

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_ParticleListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mcCounts = {421: {'sum': 100}, 0: {'sum': 1e7}}
        preCutConfig = unittest.mock.NonCallableMock(spec=fei.config.PreCutConfiguration, bestCandidateCut=10)
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.GenerateTrainingData(resource, 'D0', None, mcCounts, preCutConfig, mvaConfig), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


mock_threading_called = False


@contextlib.contextmanager
def mock_threading():
    global mock_threading_called
    mock_threading_called = True
    yield


class TestTrainMultivariateClassifier(unittest.TestCase):

    def setUp(self):
        global mock_threading_called
        fei.provider.MinimumNumberOfMVASamples = 10
        mock_threading_called = False
        self.original_call = subprocess.call

        f = ROOT.TFile('trainingData.root', 'RECREATE')
        f.cd()
        tree = ROOT.TTree('treename', 'title')
        isSignal = np.zeros(1, dtype=float)
        p = np.zeros(1, dtype=float)
        pt = np.zeros(1, dtype=float)
        tree.Branch('isSignal', isSignal, 'isSignal/D')
        tree.Branch('p', p, 'p/D')
        tree.Branch('pt', pt, 'pt/D')
        for i in range(1000):
            isSignal[0] = i % 2
            p[0] = i
            pt[0] = i*2
            tree.Fill()
        f.Write("treename")
        # Upload function does nothing, otherwise unittest will fail due to the invalid weightfile
        basf2_mva.upload = lambda *args: 1

    def tearDown(self):
        global mock_threading_called
        fei.provider.MinimumNumberOfMVASamples = 10
        mock_threading_called = False
        subprocess.call = self.original_call

        if os.path.isfile('trainingData.root'):
            os.remove('trainingData.root')

        # Remove fake upload function again
        # hence getattr will be used which will return the original function
        del basf2_mva.upload

    def test_TrainMultivariateClassifierSuccessfull(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)

        subprocess.call = unittest.mock.create_autospec(self.original_call,
                                                        side_effect=lambda command, shell: open('trainingData.xml',
                                                                                                'w').close())
        fei.provider.MinimumNumberOfMVASamples = 499
        self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, 'trainingData'),
                         'trainingData')
        subprocess.call.assert_called_once_with("basf2_mva_teacher --method 'FastBDT' --target_variable 'isSignal' "
                                                "--treename variables "
                                                "--datafile 'trainingData.root' --signal_class 1 --variables 'p' 'pt' "
                                                "--weightfile 'trainingData.xml' ConfigString > 'trainingData'.log 2>&1"
                                                " && basf2_mva_upload --filename 'trainingData.xml' "
                                                "--identifier 'FEITEST_trainingData'", shell=True)
        os.remove('trainingData.xml')

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, True)

    def test_TrainMultivariateClassifierTooFewSamples(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)

        subprocess.call = unittest.mock.create_autospec(self.original_call)
        fei.provider.MinimumNumberOfMVASamples = 501
        self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, 'trainingData'),
                         None)
        self.assertFalse(subprocess.call.called)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, False)

    def test_TrainMultivariateClassifierROOTFileNotAvailable(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)

        os.remove('trainingData.root')
        subprocess.call = unittest.mock.create_autospec(self.original_call)
        self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, 'trainingData'),
                         None)
        self.assertFalse(subprocess.call.called)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, False)

    def test_TrainMultivariateClassifierTreeNotAvailable(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)

        f = ROOT.TFile('trainingData.root', 'RECREATE')
        del f
        subprocess.call = unittest.mock.create_autospec(self.original_call)
        self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, 'trainingData'),
                         None)
        self.assertFalse(subprocess.call.called)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, False)

    def test_TrainMultivariateClassifierFailed(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)

        subprocess.call = unittest.mock.create_autospec(self.original_call)
        self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, 'trainingData'), None)
        subprocess.call.assert_called_once_with("basf2_mva_teacher --method 'FastBDT' --target_variable 'isSignal' "
                                                "--treename variables "
                                                "--datafile 'trainingData.root' --signal_class 1 --variables 'p' 'pt' "
                                                "--weightfile 'trainingData.xml' ConfigString > 'trainingData'.log 2>&1"
                                                " && basf2_mva_upload --filename 'trainingData.xml' "
                                                "--identifier 'FEITEST_trainingData'", shell=True)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, True)

    def test_AlreadyTrained(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)
        with temporary_file('trainingData.xml'):
            self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, 'trainingData'),
                             'trainingData')

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, False)

    def test_MissingTrainingData(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource,
                                                 env={'externTeacher': 'basf2_mva_teacher'},
                                                 path=basf2.create_path(), hash='42')
        resource.EnableMultiThreading = mock_threading
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', config='ConfigString',
                                                  target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.TrainMultivariateClassifier(resource, 'FEITEST', mvaConfig, None), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(mock_threading_called, False)


class TestSignalProbability(unittest.TestCase):

    def test_SignalProbability(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.SignalProbability(resource, 'FEITEST', 'D0:1', mvaConfig, 'configMVC'), '42')

        path = basf2.create_path()
        path.add_module('MVAExpert', identifier='FEITEST_configMVC', extraInfoName='SignalProbability', listNames=['D0:1'])
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.SignalProbability(resource, 'FEITEST', 'D0:1', mvaConfig, 'configMVC'), '42')

        path = basf2.create_path()
        path.add_module('MVAExpert', identifier='FEITEST_configMVC', extraInfoName='SignalProbability', listNames=['D0:1'])
        hist_variables = ['mcErrors', 'mcParticleStatus', 'extraInfo(SignalProbability)', 'isSignal']
        hist_variables_2d = [('extraInfo(SignalProbability)', 'isSignal'),
                             ('extraInfo(SignalProbability)', 'mcErrors'),
                             ('extraInfo(SignalProbability)', 'mcParticleStatus')]
        path.add_module('VariablesToHistogram', particleList='D0:1',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_SignalProbability_D0:1.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_MissingConfigMVC(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.SignalProbability(resource, 'FEITEST', 'D0:1', mvaConfig, None), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_ParticleListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, variables=['p', 'pt'],
                                                  method='FastBDT', target='isSignal', sPlotVariable=None)
        self.assertEqual(fei.provider.SignalProbability(resource, 'FEITEST', None, mvaConfig, 'configMVC'), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestTagUniqueSignal(unittest.TestCase):

    def test_TagUniqueSignal(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.TagUniqueSignal(resource, 'D0:1', 'signalProbability', mvaConfig), '42')

        path = basf2.create_path()
        path.add_module('TagUniqueSignal', particleList='D0:1', target='isSignal', extraInfoName='uniqueSignal')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_Monitoring(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': True},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.TagUniqueSignal(resource, 'D0:1', 'signalProbability', mvaConfig), '42')

        path = basf2.create_path()
        path.add_module('TagUniqueSignal', particleList='D0:1', target='isSignal', extraInfoName='uniqueSignal')
        hist_variables = ['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)', 'isSignal', 'extraInfo(decayModeID)']
        hist_variables_2d = [('extraInfo(decayModeID)', mvaConfig.target),
                             ('extraInfo(decayModeID)', 'mcErrors'),
                             ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                             ('extraInfo(decayModeID)', 'mcParticleStatus')]
        path.add_module('VariablesToHistogram', particleList='D0:1',
                        variables=fei.config.variables2binnings(hist_variables),
                        variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                        fileName='Monitor_TagUniqueSignal_D0:1.root')
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_ParticleListIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.TagUniqueSignal(resource, None, 'signalProbability', mvaConfig), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)

    def test_SignalProbabilityIsNone(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, env={'monitor': False},
                                                 path=basf2.create_path(), hash='42')
        mvaConfig = unittest.mock.NonCallableMock(spec=fei.config.MVAConfiguration, target='isSignal')
        self.assertEqual(fei.provider.TagUniqueSignal(resource, 'D0:1', None, mvaConfig), None)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestSaveModuleStatistics(unittest.TestCase):

    def test_SaveModuleStatistics(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, path=basf2.create_path(), hash='42', env={'monitor': False})
        self.assertEqual(fei.provider.SaveModuleStatistics(resource), None)

        path = basf2.create_path()
        path.add_module('RootOutput', outputFileName='moduleStatistics_42.root',
                        branchNames=['EventMetaData'], branchNamesPersistent=['ProcessStatistics'],
                        ignoreCommandLineOverride=True)
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.halt, True)

    def test_FileAlreadyAvailable(self):
        # Deactivated
        return
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, path=basf2.create_path(), hash='42', env={'monitor': False})
        with temporary_file('moduleStatistics_42.root'):
            self.assertEqual(fei.provider.SaveModuleStatistics(resource), 'moduleStatistics_42.root')

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestCountMCParticles(unittest.TestCase):

    def tearDown(self):
        if os.path.isfile('mcParticlesCount.root'):
            os.remove('mcParticlesCount.root')

    def test_CountMCParticles(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, path=basf2.create_path(), hash='42')
        self.assertEqual(fei.provider.CountMCParticles(resource, ['e+', 'mu+', 'e-']), None)

        path = basf2.create_path()
        path.add_module('VariablesToHistogram', fileName='mcParticlesCount.root',
                        variables=[('NumberOfMCParticlesInEvent(11)', 100, -0.5, 99.5),
                                   ('NumberOfMCParticlesInEvent(13)', 100, -0.5, 99.5)])
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(resource.halt, True)
        self.assertEqual(resource.condition, ('EventType', '==0'))

    def test_FileAlreadyAvailable(self):
        f = ROOT.TFile('mcParticlesCount.root', 'RECREATE')
        f.cd()
        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo11__bc", "NumberOfMCParticlesInEvent__bo11__bc", 11, -0.5, 10.5)
        for i in range(10):
            hist.Fill(5)
        for i in range(5):
            hist.Fill(4)
            hist.Fill(6)
        for i in range(3):
            hist.Fill(3)
            hist.Fill(7)
        f.Write("NumberOfMCParticlesInEvent__bo11__bc")

        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, path=basf2.create_path(), hash='42')
        mcCounts = {11: {'sum': 130, 'avg': 5.0, 'max': 7, 'min': 3, 'std': 1.1435437497937306}, 0: {'sum': 26}}
        self.assertDictEqual(fei.provider.CountMCParticles(resource, ['e+', 'mu+', 'e-']), mcCounts)

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)


class TestSaveSummary(unittest.TestCase):

    def test_SaveSummary(self):
        resource = unittest.mock.NonCallableMock(spec=fei.dag.Resource, path=basf2.create_path(), hash='42')
        self.assertEqual(fei.provider.SaveSummary(resource, {0: {'sum': 10}},
                                                  {'ParticleCombiner': 4.0, 'ParticleMCMatcher': 2.0, 'VertexFitter': 30.0},
                                                  [fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E']))],
                                                  ['D0:hum'], ['D0:23', 'D0:24', 'D0:25'], ['D0:m23', 'D0:m24', 'D0:m25'],
                                                  ['D0 ==> K- pi+', 'D0 ==> K- pi+ pi0', 'D0 ==> K- pi+ pi0 pi0'],
                                                  ['t23.root', 't24.root', 't25.root']), 'Summary_42.pickle')

        expected = {'mc_counts': {0: {'sum': 10}},
                    'module_statistics': {'ParticleCombiner': 4.0, 'ParticleMCMatcher': 2.0, 'VertexFitter': 30.0},
                    'particles': [fei.config.Particle('D0', fei.config.MVAConfiguration(variables=['E']))],
                    'particle2list': {'D0:generic': 'D0:hum'},
                    'channel2lists': {'D0 ==> K- pi+': ('D0:23', 'D0:m23', 't23.root'),
                                      'D0 ==> K- pi+ pi0': ('D0:24', 'D0:m24', 't24.root'),
                                      'D0 ==> K- pi+ pi0 pi0': ('D0:25', 'D0:m25', 't25.root')}}

        path = basf2.create_path()
        self.assertEqual(resource.path, path)
        self.assertEqual(resource.cache, True)
        self.assertEqual(os.path.isfile('Summary_42.pickle'), True)
        with open('Summary_42.pickle', 'rb') as f:
            data = pickle.load(f)
        self.assertDictEqual(data, expected)
        os.remove('Summary_42.pickle')


if __name__ == '__main__':
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
