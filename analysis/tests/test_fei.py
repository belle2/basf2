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
import subprocess
import ROOT

import fei
from fei.config import *
from fei import core

import numpy as np

import ROOT
from ROOT import Belle2
import basf2_mva
import pdg

# @cond

# Define equality operators for a bunch of pybasf2 classes
import pybasf2
pybasf2.Module.__eq__ = lambda a, b: a.type() == b.type() and\
    all(x == y for x, y in zip(a.available_params(), b.available_params()))
pybasf2.ModuleParamInfo.__eq__ = lambda a, b: a.name == b.name and a.values == b.values
pybasf2.Path.__eq__ = lambda a, b: all(x == y for x, y in zip(a.modules(), b.modules()))


def print_path(a, b):
    """
    Print the parts of the pathes which are different
    """
    for x, y in zip(a.modules(), b.modules()):
        if x.type() != y.type():
            print(x.type(), y.type())
        for n, m in zip(x.available_params(), y.available_params()):
            if n.name != m.name:
                print(n.name, m.name)
            if n.values != m.values:
                print(n.values, m.values)


def get_small_unittest_channels():
    pion = Particle('pi+',
                    MVAConfiguration(variables=['p', 'dr'],
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='piid',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=['p', 'dr'],
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='Kid',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    D0 = Particle('D0',
                  MVAConfiguration(variables=['M', 'p'],
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateMode='lowest',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    D0.addChannel(['K-', 'pi+'])
    D0.addChannel(['pi-', 'pi+'])

    particles = [pion, kaon, D0]
    return particles


class TestGetStagesFromParticles(unittest.TestCase):
    def test_get_stages_from_particles(self):
        particles = fei.get_unittest_channels()
        stages = fei.core.get_stages_from_particles(particles)
        self.assertEqual(len(stages), 7)
        self.assertEqual(len(stages[0]), 4)
        self.assertEqual(stages[0][0].identifier, 'gamma:generic')
        self.assertEqual(stages[0][1].identifier, 'mu+:generic')
        self.assertEqual(stages[0][2].identifier, 'pi+:generic')
        self.assertEqual(stages[0][3].identifier, 'K+:generic')
        self.assertEqual(len(stages[1]), 1)
        self.assertEqual(stages[1][0].identifier, 'pi0:generic')
        self.assertEqual(len(stages[2]), 0)
        self.assertEqual(len(stages[3]), 2)
        self.assertEqual(stages[3][0].identifier, 'D0:generic')
        self.assertEqual(stages[3][1].identifier, 'D0:semileptonic')
        self.assertEqual(len(stages[4]), 0)
        self.assertEqual(len(stages[5]), 0)
        self.assertEqual(len(stages[6]), 0)


class TestTrainingDataInformation(unittest.TestCase):

    def tearDown(self):
        if os.path.isfile('mcParticlesCount.root'):
            os.remove('mcParticlesCount.root')

    def test_reconstruct(self):
        particles = fei.get_unittest_channels()
        x = fei.core.TrainingDataInformation(particles)

        path = basf2.create_path()
        path.add_module('VariablesToHistogram', fileName='mcParticlesCount.root',
                        variables=[
                            ('NumberOfMCParticlesInEvent(321)', 100, -0.5, 99.5),
                            ('NumberOfMCParticlesInEvent(421)', 100, -0.5, 99.5),
                            ('NumberOfMCParticlesInEvent(13)', 100, -0.5, 99.5),
                            ('NumberOfMCParticlesInEvent(111)', 100, -0.5, 99.5),
                            ('NumberOfMCParticlesInEvent(211)', 100, -0.5, 99.5),
                            ('NumberOfMCParticlesInEvent(22)', 100, -0.5, 99.5)]
                        )
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_available(self):
        particles = fei.get_unittest_channels()
        x = fei.core.TrainingDataInformation(particles)
        self.assertEqual(x.available(), False)
        f = ROOT.TFile('mcParticlesCount.root', 'RECREATE')
        self.assertEqual(x.available(), True)

    def test_get_mc_counts(self):
        f = ROOT.TFile('mcParticlesCount.root', 'RECREATE')
        f.cd()
        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo211__bc", "NumberOfMCParticlesInEvent__bo211__bc", 11, -0.5, 10.5)
        for i in range(10):
            hist.Fill(5)
        for i in range(5):
            hist.Fill(4)
        for i in range(3):
            hist.Fill(3)
        f.Write("NumberOfMCParticlesInEvent__bo211__bc")

        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo321__bc", "NumberOfMCParticlesInEvent__bo321__bc", 11, -0.5, 10.5)
        for i in range(8):
            hist.Fill(4)
        for i in range(5):
            hist.Fill(2)
        for i in range(5):
            hist.Fill(7)
        f.Write("NumberOfMCParticlesInEvent__bo321__bc")

        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo13__bc", "NumberOfMCParticlesInEvent__bo13__bc", 11, -0.5, 10.5)
        for i in range(18):
            hist.Fill(5)
        f.Write("NumberOfMCParticlesInEvent__bo13__bc")

        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo22__bc", "NumberOfMCParticlesInEvent__bo222__bc", 11, -0.5, 10.5)
        for i in range(18):
            hist.Fill(0)
        f.Write("NumberOfMCParticlesInEvent__bo22__bc")

        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo111__bc", "NumberOfMCParticlesInEvent__bo111__bc", 11, -0.5, 10.5)
        for i in range(5):
            hist.Fill(5)
        for i in range(10):
            hist.Fill(4)
        for i in range(3):
            hist.Fill(3)
        f.Write("NumberOfMCParticlesInEvent__bo111__bc")

        hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo421__bc", "NumberOfMCParticlesInEvent__bo421__bc", 11, -0.5, 10.5)
        for i in range(10):
            hist.Fill(9)
        for i in range(5):
            hist.Fill(0)
        for i in range(3):
            hist.Fill(1)
        f.Write("NumberOfMCParticlesInEvent__bo421__bc")

        particles = fei.get_unittest_channels()
        x = fei.core.TrainingDataInformation(particles)

        mcCounts = {
            211: {'max': 5.0, 'min': 3.0, 'sum': 79.0, 'avg': 4.3888888888888893, 'std': 0.7556372504852998},
            321: {'max': 7.0, 'min': 2.0, 'sum': 77.0, 'avg': 4.2777777777777777, 'std': 1.8798804795209585},
            13: {'max': 5.0, 'min': 5.0, 'sum': 90.0, 'avg': 5.0, 'std': 0.0},
            22: {'max': 0.0, 'min': 0.0, 'sum': 0.0, 'avg': 0.0, 'std': 0.0},
            111: {'max': 5.0, 'min': 3.0, 'sum': 74.0, 'avg': 4.1111111111111107, 'std': 0.6573421981221816},
            421: {'max': 9.0, 'min': 0.0, 'sum': 93.0, 'avg': 5.166666666666667, 'std': 4.2979323194092087},
            0: {'sum': 18.0}}
        self.assertDictEqual(x.get_mc_counts(), mcCounts)


class TestFSPLoader(unittest.TestCase):

    def test_belle2_without_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False)
        x = fei.core.FSPLoader(particles, config)

        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                                                ('mu+:FSP', ''), ('gamma:FSP', ''), ('K_S0:V0', ''),
                                                                ('p+:FSP', ''), ('K_L0:FSP', ''), ('Lambda0:FSP', '')],
                        writeOut=True)
        path.add_module('ParticleLoader', decayStringsWithCuts=[('gamma:V0', '')], addDaughters=True, writeOut=True)
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_belle2_with_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=True)
        x = fei.core.FSPLoader(particles, config)

        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                                                ('mu+:FSP', ''), ('gamma:FSP', ''), ('K_S0:V0', ''),
                                                                ('p+:FSP', ''), ('K_L0:FSP', ''), ('Lambda0:FSP', '')],
                        writeOut=True)
        path.add_module('ParticleLoader', decayStringsWithCuts=[('gamma:V0', '')], addDaughters=True, writeOut=True)
        hist_variables = [('NumberOfMCParticlesInEvent({i})'.format(i=pdgcode), 100, -0.5, 99.5)
                          for pdgcode in set([11, 321, 211, 13, 22, 310, 2212, 130, 3122, 111])]
        path.add_module('VariablesToHistogram', particleList='',
                        variables=hist_variables,
                        fileName='Monitor_FSPLoader.root')
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_belle1_without_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False, b2bii=True)
        x = fei.core.FSPLoader(particles, config)

        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                                                ('mu+:FSP', ''),
                                                                ('p+:FSP', ''), ('K_L0:FSP', '')],
                        writeOut=True)
        path.add_module('ParticleListManipulator', outputListName='gamma:FSP', inputListNames=['gamma:mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['gamma:FSP'])
        path.add_module('ParticleListManipulator', outputListName='K_S0:V0', inputListNames=['K_S0:mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['K_S0:V0'])
        path.add_module('ParticleListManipulator', outputListName='pi0:FSP', inputListNames=['pi0:mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['pi0:FSP'])
        path.add_module('ParticleListManipulator', outputListName='gamma:V0', inputListNames=['gamma:v0mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['gamma:V0'])
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_belle1_with_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=True, b2bii=True)
        x = fei.core.FSPLoader(particles, config)

        path = basf2.create_path()
        path.add_module('ParticleLoader', decayStringsWithCuts=[('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                                                ('mu+:FSP', ''),
                                                                ('p+:FSP', ''), ('K_L0:FSP', '')],
                        writeOut=True)
        path.add_module('ParticleListManipulator', outputListName='gamma:FSP', inputListNames=['gamma:mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['gamma:FSP'])
        path.add_module('ParticleListManipulator', outputListName='K_S0:V0', inputListNames=['K_S0:mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['K_S0:V0'])
        path.add_module('ParticleListManipulator', outputListName='pi0:FSP', inputListNames=['pi0:mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['pi0:FSP'])
        path.add_module('ParticleListManipulator', outputListName='gamma:V0', inputListNames=['gamma:v0mdst'], writeOut=True)
        path.add_module('ParticleCopier', inputListNames=['gamma:V0'])
        hist_variables = [('NumberOfMCParticlesInEvent({i})'.format(i=pdgcode), 100, -0.5, 99.5)
                          for pdgcode in set([11, 321, 211, 13, 22, 310, 2212, 130, 3122, 111])]
        path.add_module('VariablesToHistogram', particleList='',
                        variables=hist_variables,
                        fileName='Monitor_FSPLoader.root')
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)


class TestTrainingData(unittest.TestCase):

    def setUp(self):
        self.mc_counts = {
            211: {'sum': 79, 'avg': 4.3888888888888893, 'max': 5, 'min': 3, 'std': 0.75563725048530228},
            321: {'sum': 77, 'avg': 4.2777777777777777, 'max': 7, 'min': 2, 'std': 1.8798804795209592},
            421: {'sum': 93, 'avg': 5.166666666666667, 'max': 9, 'min': 0, 'std': 4.2979323194092087},
            0: {'sum': 18}}

    def test_without_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False)
        x = fei.core.TrainingData(particles, config, self.mc_counts)

        path = basf2.create_path()
        path.add_module('VariablesToNtuple', fileName='pi+:generic ==> pi+:FSP.root', treeName='variables',
                        variables=['p', 'dr', 'isPrimarySignal'],
                        particleList='pi+:generic_0', sampling=('isPrimarySignal', {}))
        path.add_module('VariablesToNtuple', fileName='K+:generic ==> K+:FSP.root', treeName='variables',
                        variables=['p', 'dr', 'isPrimarySignal'],
                        particleList='K+:generic_0', sampling=('isPrimarySignal', {}))
        path.add_module('VariablesToNtuple', fileName='D0:generic ==> K-:generic pi+:generic.root', treeName='variables',
                        variables=['M', 'p', 'isSignal'],
                        particleList='D0:generic_0', sampling=('isSignal', {}))
        path.add_module('VariablesToNtuple', fileName='D0:generic ==> pi-:generic pi+:generic.root', treeName='variables',
                        variables=['M', 'p', 'isSignal'],
                        particleList='D0:generic_1', sampling=('isSignal', {}))
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_with_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=True)
        x = fei.core.TrainingData(particles, config, self.mc_counts)

        path = basf2.create_path()
        path.add_module('VariablesToHistogram', particleList='pi+:generic_0',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'p', 'dr', 'isPrimarySignal']),
                        variables_2d=fei.config.variables2binnings_2d([('p', 'isPrimarySignal'), ('dr', 'isPrimarySignal')]),
                        fileName='Monitor_TrainingData_pi+:generic ==> pi+:FSP.root')
        path.add_module('VariablesToNtuple', fileName='pi+:generic ==> pi+:FSP.root', treeName='variables',
                        variables=['p', 'dr', 'isPrimarySignal'],
                        particleList='pi+:generic_0', sampling=('isPrimarySignal', {}))
        path.add_module('VariablesToHistogram', particleList='K+:generic_0',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'p', 'dr', 'isPrimarySignal']),
                        variables_2d=fei.config.variables2binnings_2d([('p', 'isPrimarySignal'), ('dr', 'isPrimarySignal')]),
                        fileName='Monitor_TrainingData_K+:generic ==> K+:FSP.root')
        path.add_module('VariablesToNtuple', fileName='K+:generic ==> K+:FSP.root', treeName='variables',
                        variables=['p', 'dr', 'isPrimarySignal'],
                        particleList='K+:generic_0', sampling=('isPrimarySignal', {}))
        path.add_module('VariablesToHistogram', particleList='D0:generic_0',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'M', 'p', 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([('M', 'isSignal'), ('p', 'isSignal')]),
                        fileName='Monitor_TrainingData_D0:generic ==> K-:generic pi+:generic.root')
        path.add_module('VariablesToNtuple', fileName='D0:generic ==> K-:generic pi+:generic.root', treeName='variables',
                        variables=['M', 'p', 'isSignal'],
                        particleList='D0:generic_0', sampling=('isSignal', {}))
        path.add_module('VariablesToHistogram', particleList='D0:generic_1',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'M', 'p', 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([('M', 'isSignal'), ('p', 'isSignal')]),
                        fileName='Monitor_TrainingData_D0:generic ==> pi-:generic pi+:generic.root')
        path.add_module('VariablesToNtuple', fileName='D0:generic ==> pi-:generic pi+:generic.root', treeName='variables',
                        variables=['M', 'p', 'isSignal'],
                        particleList='D0:generic_1', sampling=('isSignal', {}))
        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)


class TestPreReconstruction(unittest.TestCase):

    def test_without_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False)
        x = fei.core.PreReconstruction(particles, config)

        path = basf2.create_path()
        path.add_module('ParticleListManipulator', inputListNames=['pi+:FSP'], outputListName='pi+:generic_0',
                        cut='[dr < 2] and [abs(dz) < 4]', writeOut=True)
        path.add_module('VariablesToExtraInfo', particleList='pi+:generic_0', variables={'constant(0)': 'decayModeID'})
        path.add_module('BestCandidateSelection', particleList='pi+:generic_0', variable='piid', selectLowest=False,
                        numBest=20, outputVariable='preCut_rank')

        path.add_module('ParticleListManipulator', inputListNames=['K+:FSP'], outputListName='K+:generic_0',
                        cut='[dr < 2] and [abs(dz) < 4]', writeOut=True)
        path.add_module('VariablesToExtraInfo', particleList='K+:generic_0', variables={'constant(0)': 'decayModeID'})
        path.add_module('BestCandidateSelection', particleList='K+:generic_0', variable='Kid', selectLowest=False,
                        numBest=20, outputVariable='preCut_rank')

        path.add_module('ParticleCombiner', decayString='D0:generic_0 ==> K-:generic pi+:generic', writeOut=True,
                        decayMode=0, cut='1.7 < M < 1.95')
        path.add_module('BestCandidateSelection', particleList='D0:generic_0',
                        variable='abs(dM)', selectLowest=True, numBest=20, outputVariable='preCut_rank')
        path.add_module('ParticleVertexFitter', listName='D0:generic_0', confidenceLevel=-2.0,
                        vertexFitter='kfitter', fitType='vertex')

        path.add_module('ParticleCombiner', decayString='D0:generic_1 ==> pi-:generic pi+:generic', writeOut=True,
                        decayMode=1, cut='1.7 < M < 1.95')
        path.add_module('BestCandidateSelection', particleList='D0:generic_1',
                        variable='abs(dM)', selectLowest=True, numBest=20, outputVariable='preCut_rank')
        path.add_module('ParticleVertexFitter', listName='D0:generic_1', confidenceLevel=-2.0,
                        vertexFitter='kfitter', fitType='vertex')

        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_with_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=True)
        x = fei.core.PreReconstruction(particles, config)

        path = basf2.create_path()
        path.add_module('ParticleListManipulator', inputListNames=['pi+:FSP'], outputListName='pi+:generic_0',
                        cut='[dr < 2] and [abs(dz) < 4]', writeOut=True)
        path.add_module('VariablesToExtraInfo', particleList='pi+:generic_0', variables={'constant(0)': 'decayModeID'})
        path.add_module('MCMatcherParticles', listName='pi+:generic_0')
        path.add_module('VariablesToHistogram', particleList='pi+:generic_0',
                        variables=fei.config.variables2binnings(['piid', 'mcErrors', 'mcParticleStatus', 'isPrimarySignal']),
                        variables_2d=fei.config.variables2binnings_2d([('piid', 'isPrimarySignal'),
                                                                       ('piid', 'mcErrors'),
                                                                       ('piid', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_BeforeRanking_pi+:generic ==> pi+:FSP.root')
        path.add_module('BestCandidateSelection', particleList='pi+:generic_0', variable='piid', selectLowest=False,
                        numBest=20, outputVariable='preCut_rank')
        path.add_module('VariablesToHistogram', particleList='pi+:generic_0',
                        variables=fei.config.variables2binnings(['piid', 'mcErrors', 'mcParticleStatus',
                                                                 'isPrimarySignal', 'extraInfo(preCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('piid', 'isPrimarySignal'),
                                                                       ('piid', 'mcErrors'),
                                                                       ('piid', 'mcParticleStatus'),
                                                                       ('extraInfo(preCut_rank)', 'isPrimarySignal'),
                                                                       ('extraInfo(preCut_rank)', 'mcErrors'),
                                                                       ('extraInfo(preCut_rank)', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterRanking_pi+:generic ==> pi+:FSP.root')
        path.add_module('VariablesToHistogram', particleList='pi+:generic_0',
                        variables=fei.config.variables2binnings(['chiProb', 'mcErrors', 'mcParticleStatus',
                                                                 'isPrimarySignal']),
                        variables_2d=fei.config.variables2binnings_2d([('chiProb', 'isPrimarySignal'),
                                                                       ('chiProb', 'mcErrors'),
                                                                       ('chiProb', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterVertex_pi+:generic ==> pi+:FSP.root')

        path.add_module('ParticleListManipulator', inputListNames=['K+:FSP'], outputListName='K+:generic_0',
                        cut='[dr < 2] and [abs(dz) < 4]', writeOut=True)
        path.add_module('VariablesToExtraInfo', particleList='K+:generic_0', variables={'constant(0)': 'decayModeID'})

        path.add_module('MCMatcherParticles', listName='K+:generic_0')
        path.add_module('VariablesToHistogram', particleList='K+:generic_0',
                        variables=fei.config.variables2binnings(['Kid', 'mcErrors', 'mcParticleStatus', 'isPrimarySignal']),
                        variables_2d=fei.config.variables2binnings_2d([('Kid', 'isPrimarySignal'),
                                                                       ('Kid', 'mcErrors'),
                                                                       ('Kid', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_BeforeRanking_K+:generic ==> K+:FSP.root')
        path.add_module('BestCandidateSelection', particleList='K+:generic_0', variable='Kid', selectLowest=False,
                        numBest=20, outputVariable='preCut_rank')
        path.add_module('VariablesToHistogram', particleList='K+:generic_0',
                        variables=fei.config.variables2binnings(['Kid', 'mcErrors', 'mcParticleStatus',
                                                                 'isPrimarySignal', 'extraInfo(preCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('Kid', 'isPrimarySignal'),
                                                                       ('Kid', 'mcErrors'),
                                                                       ('Kid', 'mcParticleStatus'),
                                                                       ('extraInfo(preCut_rank)', 'isPrimarySignal'),
                                                                       ('extraInfo(preCut_rank)', 'mcErrors'),
                                                                       ('extraInfo(preCut_rank)', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterRanking_K+:generic ==> K+:FSP.root')
        path.add_module('VariablesToHistogram', particleList='K+:generic_0',
                        variables=fei.config.variables2binnings(['chiProb', 'mcErrors', 'mcParticleStatus',
                                                                 'isPrimarySignal']),
                        variables_2d=fei.config.variables2binnings_2d([('chiProb', 'isPrimarySignal'),
                                                                       ('chiProb', 'mcErrors'),
                                                                       ('chiProb', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterVertex_K+:generic ==> K+:FSP.root')

        path.add_module('ParticleCombiner', decayString='D0:generic_0 ==> K-:generic pi+:generic', writeOut=True,
                        decayMode=0, cut='1.7 < M < 1.95')
        path.add_module('MCMatcherParticles', listName='D0:generic_0')
        path.add_module('VariablesToHistogram', particleList='D0:generic_0',
                        variables=fei.config.variables2binnings(['abs(dM)', 'mcErrors', 'mcParticleStatus', 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([('abs(dM)', 'isSignal'),
                                                                       ('abs(dM)', 'mcErrors'),
                                                                       ('abs(dM)', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_BeforeRanking_D0:generic ==> K-:generic pi+:generic.root')
        path.add_module('BestCandidateSelection', particleList='D0:generic_0',
                        variable='abs(dM)', selectLowest=True, numBest=20, outputVariable='preCut_rank')
        path.add_module('VariablesToHistogram', particleList='D0:generic_0',
                        variables=fei.config.variables2binnings(['abs(dM)', 'mcErrors', 'mcParticleStatus',
                                                                 'isSignal', 'extraInfo(preCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('abs(dM)', 'isSignal'),
                                                                       ('abs(dM)', 'mcErrors'),
                                                                       ('abs(dM)', 'mcParticleStatus'),
                                                                       ('extraInfo(preCut_rank)', 'isSignal'),
                                                                       ('extraInfo(preCut_rank)', 'mcErrors'),
                                                                       ('extraInfo(preCut_rank)', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterRanking_D0:generic ==> K-:generic pi+:generic.root')
        path.add_module('ParticleVertexFitter', listName='D0:generic_0', confidenceLevel=-2.0,
                        vertexFitter='kfitter', fitType='vertex')
        path.add_module('VariablesToHistogram', particleList='D0:generic_0',
                        variables=fei.config.variables2binnings(['chiProb', 'mcErrors', 'mcParticleStatus',
                                                                 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([('chiProb', 'isSignal'),
                                                                       ('chiProb', 'mcErrors'),
                                                                       ('chiProb', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterVertex_D0:generic ==> K-:generic pi+:generic.root')

        path.add_module('ParticleCombiner', decayString='D0:generic_1 ==> pi-:generic pi+:generic', writeOut=True,
                        decayMode=1, cut='1.7 < M < 1.95')
        path.add_module('MCMatcherParticles', listName='D0:generic_1')
        path.add_module('VariablesToHistogram', particleList='D0:generic_1',
                        variables=fei.config.variables2binnings(['abs(dM)', 'mcErrors', 'mcParticleStatus', 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([('abs(dM)', 'isSignal'),
                                                                       ('abs(dM)', 'mcErrors'),
                                                                       ('abs(dM)', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_BeforeRanking_D0:generic ==> pi-:generic pi+:generic.root')
        path.add_module('BestCandidateSelection', particleList='D0:generic_1',
                        variable='abs(dM)', selectLowest=True, numBest=20, outputVariable='preCut_rank')
        path.add_module('VariablesToHistogram', particleList='D0:generic_1',
                        variables=fei.config.variables2binnings(['abs(dM)', 'mcErrors', 'mcParticleStatus',
                                                                 'isSignal', 'extraInfo(preCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('abs(dM)', 'isSignal'),
                                                                       ('abs(dM)', 'mcErrors'),
                                                                       ('abs(dM)', 'mcParticleStatus'),
                                                                       ('extraInfo(preCut_rank)', 'isSignal'),
                                                                       ('extraInfo(preCut_rank)', 'mcErrors'),
                                                                       ('extraInfo(preCut_rank)', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterRanking_D0:generic ==> pi-:generic pi+:generic.root')
        path.add_module('ParticleVertexFitter', listName='D0:generic_1', confidenceLevel=-2.0,
                        vertexFitter='kfitter', fitType='vertex')
        path.add_module('VariablesToHistogram', particleList='D0:generic_1',
                        variables=fei.config.variables2binnings(['chiProb', 'mcErrors', 'mcParticleStatus',
                                                                 'isSignal']),
                        variables_2d=fei.config.variables2binnings_2d([('chiProb', 'isSignal'),
                                                                       ('chiProb', 'mcErrors'),
                                                                       ('chiProb', 'mcParticleStatus')]),
                        fileName='Monitor_PreReconstruction_AfterVertex_D0:generic ==> pi-:generic pi+:generic.root')

        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)


class TestPostReconstruction(unittest.TestCase):

    def test_get_missing_channels(self):
        pion = Particle('pi+:unittest', MVAConfiguration(variables=['p', 'dr'], target='isPrimarySignal'))
        pion.addChannel(['pi+:FSP'])
        D0 = Particle('D0:unittest', MVAConfiguration(variables=['M', 'p'], target='isSignal'))
        D0.addChannel(['K-:unittest', 'pi+:unittest'])
        D0.addChannel(['pi-:unittest', 'pi+:unittest'])
        config = fei.config.FeiConfiguration(monitor=False, prefix="UNITTEST")
        x = fei.core.PostReconstruction([pion, D0], config)

        self.assertEqual(x.get_missing_channels(), ['pi+:unittest ==> pi+:FSP', 'D0:unittest ==> K-:unittest pi+:unittest',
                                                    'D0:unittest ==> pi-:unittest pi+:unittest'])
        self.assertEqual(x.available(), False)

        content = """
            <?xml version="1.0" encoding="utf-8"?>
            <method>Trivial</method>
            <weightfile>{channel}.xml</weightfile>
            <treename>tree</treename>
            <target_variable>isSignal</target_variable>
            <weight_variable>__weight__</weight_variable>
            <signal_class>1</signal_class>
            <max_events>0</max_events>
            <number_feature_variables>1</number_feature_variables>
            <variable0>M</variable0>
            <number_spectator_variables>0</number_spectator_variables>
            <number_data_files>1</number_data_files>
            <datafile0>train.root</datafile0>
            <Trivial_version>1</Trivial_version>
            <Trivial_output>0</Trivial_output>
            <signal_fraction>0.066082567</signal_fraction>
            """

        channel = 'D0:unittest ==> K-:unittest pi+:unittest'
        with open(channel + ".xml", "w") as f:
            f.write(content.format(channel=channel))
        basf2_mva.upload(channel + ".xml", 'UNITTEST_' + channel)

        self.assertEqual(x.get_missing_channels(), ['pi+:unittest ==> pi+:FSP',
                                                    'D0:unittest ==> pi-:unittest pi+:unittest'])
        self.assertEqual(x.available(), False)

        channel = 'D0:unittest ==> pi-:unittest pi+:unittest'
        with open(channel + ".xml", "w") as f:
            f.write(content.format(channel=channel))
        basf2_mva.upload(channel + ".xml", 'UNITTEST_' + channel)

        self.assertEqual(x.get_missing_channels(), ['pi+:unittest ==> pi+:FSP'])
        self.assertEqual(x.available(), False)

        channel = 'pi+:unittest ==> pi+:FSP'
        with open(channel + ".xml", "w") as f:
            f.write(content.format(channel=channel))
        basf2_mva.upload(channel + ".xml", 'UNITTEST_' + channel)

        self.assertEqual(x.get_missing_channels(), [])
        self.assertEqual(x.available(), True)

    def tearDown(self):
        if os.path.isfile('pi+:unittest ==> pi+:FSP.xml'):
            os.remove('pi+:unittest ==> pi+:FSP.xml')
        if os.path.isfile('D0:unittest ==> pi-:unittest pi+:unittest.xml'):
            os.remove('D0:unittest ==> pi-:unittest pi+:unittest.xml')
        if os.path.isfile('D0:unittest ==> K-:unittest pi+:unittest.xml'):
            os.remove('D0:unittest ==> K-:unittest pi+:unittest.xml')

    def test_without_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False, prefix='UNITTEST')
        x = fei.core.PostReconstruction(particles, config)

        path = basf2.create_path()

        path.add_module('MVAExpert', identifier='UNITTEST_pi+:generic ==> pi+:FSP', extraInfoName='SignalProbability',
                        listNames=['pi+:generic_0'])
        path.add_module('TagUniqueSignal', particleList='pi+:generic_0', target='isPrimarySignal',
                        extraInfoName='uniqueSignal')
        path.add_module('ParticleListManipulator', outputListName='pi+:generic', inputListNames=['pi+:generic_0'],
                        writeOut=True)
        path.add_module('ParticleSelector', decayString='pi+:generic', cut='0.01 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='pi+:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        path.add_module('MVAExpert', identifier='UNITTEST_K+:generic ==> K+:FSP', extraInfoName='SignalProbability',
                        listNames=['K+:generic_0'])
        path.add_module('TagUniqueSignal', particleList='K+:generic_0', target='isPrimarySignal',
                        extraInfoName='uniqueSignal')
        path.add_module('ParticleListManipulator', outputListName='K+:generic', inputListNames=['K+:generic_0'],
                        writeOut=True)
        path.add_module('ParticleSelector', decayString='K+:generic', cut='0.01 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='K+:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        path.add_module('MVAExpert', identifier='UNITTEST_D0:generic ==> K-:generic pi+:generic',
                        extraInfoName='SignalProbability', listNames=['D0:generic_0'])
        path.add_module('TagUniqueSignal', particleList='D0:generic_0', target='isSignal',
                        extraInfoName='uniqueSignal')

        path.add_module('MVAExpert', identifier='UNITTEST_D0:generic ==> pi-:generic pi+:generic',
                        extraInfoName='SignalProbability', listNames=['D0:generic_1'])
        path.add_module('TagUniqueSignal', particleList='D0:generic_1', target='isSignal',
                        extraInfoName='uniqueSignal')

        path.add_module('ParticleListManipulator', outputListName='D0:generic',
                        inputListNames=['D0:generic_0', 'D0:generic_1'],
                        writeOut=True)
        path.add_module('ParticleSelector', decayString='D0:generic', cut='0.001 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='D0:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_without_monitoring_training_mode(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False, prefix='UNITTEST', training=True)
        x = fei.core.PostReconstruction(particles, config)

        path = basf2.create_path()

        path.add_module('MVAExpert', identifier='pi+:generic ==> pi+:FSP.xml', extraInfoName='SignalProbability',
                        listNames=['pi+:generic_0'])
        path.add_module('TagUniqueSignal', particleList='pi+:generic_0', target='isPrimarySignal',
                        extraInfoName='uniqueSignal')
        path.add_module('ParticleListManipulator', outputListName='pi+:generic', inputListNames=['pi+:generic_0'],
                        writeOut=True)
        path.add_module('ParticleSelector', decayString='pi+:generic', cut='0.01 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='pi+:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        path.add_module('MVAExpert', identifier='K+:generic ==> K+:FSP.xml', extraInfoName='SignalProbability',
                        listNames=['K+:generic_0'])
        path.add_module('TagUniqueSignal', particleList='K+:generic_0', target='isPrimarySignal',
                        extraInfoName='uniqueSignal')
        path.add_module('ParticleListManipulator', outputListName='K+:generic', inputListNames=['K+:generic_0'],
                        writeOut=True)
        path.add_module('ParticleSelector', decayString='K+:generic', cut='0.01 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='K+:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        path.add_module('MVAExpert', identifier='D0:generic ==> K-:generic pi+:generic.xml',
                        extraInfoName='SignalProbability', listNames=['D0:generic_0'])
        path.add_module('TagUniqueSignal', particleList='D0:generic_0', target='isSignal',
                        extraInfoName='uniqueSignal')

        path.add_module('MVAExpert', identifier='D0:generic ==> pi-:generic pi+:generic.xml',
                        extraInfoName='SignalProbability', listNames=['D0:generic_1'])
        path.add_module('TagUniqueSignal', particleList='D0:generic_1', target='isSignal',
                        extraInfoName='uniqueSignal')

        path.add_module('ParticleListManipulator', outputListName='D0:generic',
                        inputListNames=['D0:generic_0', 'D0:generic_1'],
                        writeOut=True)
        path.add_module('ParticleSelector', decayString='D0:generic', cut='0.001 < extraInfo(SignalProbability)')
        path.add_module('BestCandidateSelection', particleList='D0:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')

        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)

    def test_with_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=True, prefix='UNITTEST')
        x = fei.core.PostReconstruction(particles, config)

        path = basf2.create_path()

        path.add_module('MVAExpert', identifier='UNITTEST_pi+:generic ==> pi+:FSP', extraInfoName='SignalProbability',
                        listNames=['pi+:generic_0'])
        path.add_module('TagUniqueSignal', particleList='pi+:generic_0', target='isPrimarySignal',
                        extraInfoName='uniqueSignal')

        path.add_module('VariablesToHistogram', particleList='pi+:generic_0',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isPrimarySignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(SignalProbability)', 'isPrimarySignal'),
                                                                       ('extraInfo(SignalProbability)', 'mcErrors'),
                                                                       ('extraInfo(SignalProbability)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_AfterMVA_pi+:generic ==> pi+:FSP.root')
        path.add_module('ParticleListManipulator', outputListName='pi+:generic', inputListNames=['pi+:generic_0'],
                        writeOut=True)
        path.add_module('VariablesToHistogram', particleList='pi+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isPrimarySignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_BeforePostCut_pi+:generic.root')
        path.add_module('ParticleSelector', decayString='pi+:generic', cut='0.01 < extraInfo(SignalProbability)')
        path.add_module('VariablesToHistogram', particleList='pi+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isPrimarySignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_BeforeRanking_pi+:generic.root')
        path.add_module('BestCandidateSelection', particleList='pi+:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')
        path.add_module('VariablesToHistogram', particleList='pi+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)', 'isPrimarySignal',
                                                                 'extraInfo(decayModeID)', 'extraInfo(postCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                                                                       ('isPrimarySignal', 'extraInfo(postCut_rank)'),
                                                                       ('mcErrors', 'extraInfo(postCut_rank)'),
                                                                       ('mcParticleStatus', 'extraInfo(postCut_rank)')]),
                        fileName='Monitor_PostReconstruction_AfterRanking_pi+:generic.root')
        path.add_module('VariablesToNtuple', fileName='Monitor_Final_pi+:generic.root', treeName='variables',
                        variables=['extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'mcParticleStatus', 'isPrimarySignal',
                                   'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)'],
                        particleList='pi+:generic')

        path.add_module('MVAExpert', identifier='UNITTEST_K+:generic ==> K+:FSP', extraInfoName='SignalProbability',
                        listNames=['K+:generic_0'])
        path.add_module('TagUniqueSignal', particleList='K+:generic_0', target='isPrimarySignal',
                        extraInfoName='uniqueSignal')
        path.add_module('VariablesToHistogram', particleList='K+:generic_0',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isPrimarySignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(SignalProbability)', 'isPrimarySignal'),
                                                                       ('extraInfo(SignalProbability)', 'mcErrors'),
                                                                       ('extraInfo(SignalProbability)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_AfterMVA_K+:generic ==> K+:FSP.root')
        path.add_module('ParticleListManipulator', outputListName='K+:generic', inputListNames=['K+:generic_0'],
                        writeOut=True)
        path.add_module('VariablesToHistogram', particleList='K+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isPrimarySignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_BeforePostCut_K+:generic.root')
        path.add_module('ParticleSelector', decayString='K+:generic', cut='0.01 < extraInfo(SignalProbability)')
        path.add_module('VariablesToHistogram', particleList='K+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isPrimarySignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_BeforeRanking_K+:generic.root')
        path.add_module('BestCandidateSelection', particleList='K+:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')
        path.add_module('VariablesToHistogram', particleList='K+:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)', 'isPrimarySignal',
                                                                 'extraInfo(decayModeID)', 'extraInfo(postCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isPrimarySignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                                                                       ('isPrimarySignal', 'extraInfo(postCut_rank)'),
                                                                       ('mcErrors', 'extraInfo(postCut_rank)'),
                                                                       ('mcParticleStatus', 'extraInfo(postCut_rank)')]),
                        fileName='Monitor_PostReconstruction_AfterRanking_K+:generic.root')
        path.add_module('VariablesToNtuple', fileName='Monitor_Final_K+:generic.root', treeName='variables',
                        variables=['extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'mcParticleStatus', 'isPrimarySignal',
                                   'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)'],
                        particleList='K+:generic')

        path.add_module('MVAExpert', identifier='UNITTEST_D0:generic ==> K-:generic pi+:generic',
                        extraInfoName='SignalProbability', listNames=['D0:generic_0'])
        path.add_module('TagUniqueSignal', particleList='D0:generic_0', target='isSignal',
                        extraInfoName='uniqueSignal')
        path.add_module('VariablesToHistogram', particleList='D0:generic_0',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isSignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(SignalProbability)', 'isSignal'),
                                                                       ('extraInfo(SignalProbability)', 'mcErrors'),
                                                                       ('extraInfo(SignalProbability)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'isSignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_AfterMVA_D0:generic ==> K-:generic pi+:generic.root')

        path.add_module('MVAExpert', identifier='UNITTEST_D0:generic ==> pi-:generic pi+:generic',
                        extraInfoName='SignalProbability', listNames=['D0:generic_1'])
        path.add_module('TagUniqueSignal', particleList='D0:generic_1', target='isSignal',
                        extraInfoName='uniqueSignal')
        path.add_module('VariablesToHistogram', particleList='D0:generic_1',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isSignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(SignalProbability)', 'isSignal'),
                                                                       ('extraInfo(SignalProbability)', 'mcErrors'),
                                                                       ('extraInfo(SignalProbability)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'isSignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_AfterMVA_D0:generic ==> pi-:generic pi+:generic.root')

        path.add_module('ParticleListManipulator', outputListName='D0:generic',
                        inputListNames=['D0:generic_0', 'D0:generic_1'],
                        writeOut=True)
        path.add_module('VariablesToHistogram', particleList='D0:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isSignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isSignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_BeforePostCut_D0:generic.root')
        path.add_module('ParticleSelector', decayString='D0:generic', cut='0.001 < extraInfo(SignalProbability)')
        path.add_module('VariablesToHistogram', particleList='D0:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)',
                                                                 'isSignal', 'extraInfo(decayModeID)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isSignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus')]),
                        fileName='Monitor_PostReconstruction_BeforeRanking_D0:generic.root')
        path.add_module('BestCandidateSelection', particleList='D0:generic', variable='extraInfo(SignalProbability)',
                        selectLowest=False, numBest=10, outputVariable='postCut_rank')
        path.add_module('VariablesToHistogram', particleList='D0:generic',
                        variables=fei.config.variables2binnings(['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)',
                                                                 'extraInfo(SignalProbability)', 'isSignal',
                                                                 'extraInfo(decayModeID)', 'extraInfo(postCut_rank)']),
                        variables_2d=fei.config.variables2binnings_2d([('extraInfo(decayModeID)', 'isSignal'),
                                                                       ('extraInfo(decayModeID)', 'mcErrors'),
                                                                       ('extraInfo(decayModeID)', 'mcParticleStatus'),
                                                                       ('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                                                                       ('isSignal', 'extraInfo(postCut_rank)'),
                                                                       ('mcErrors', 'extraInfo(postCut_rank)'),
                                                                       ('mcParticleStatus', 'extraInfo(postCut_rank)')]),
                        fileName='Monitor_PostReconstruction_AfterRanking_D0:generic.root')
        path.add_module('VariablesToNtuple', fileName='Monitor_Final_D0:generic.root', treeName='variables',
                        variables=['extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'mcParticleStatus', 'isSignal',
                                   'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)'],
                        particleList='D0:generic')

        print_path(path, x.reconstruct())
        self.assertEqual(x.reconstruct(), path)


class TestTeacher(unittest.TestCase):
    def setUp(self):
        fei.core.Teacher.MaximumNumberOfMVASamples = int(1e7)
        fei.core.Teacher.MinimumNumberOfMVASamples = int(10)

        f = ROOT.TFile('pi+:generic ==> pi+:FSP.root', 'RECREATE')
        f.cd()
        tree = ROOT.TTree('variables', 'variables')
        isSignal = np.zeros(1, dtype=float)
        p = np.zeros(1, dtype=float)
        pt = np.zeros(1, dtype=float)
        tree.Branch('isPrimarySignal', isSignal, 'isPrimarySignal/D')
        tree.Branch('p', p, 'p/D')
        tree.Branch('dr', pt, 'dr/D')
        for i in range(1000):
            isSignal[0] = i % 2
            p[0] = i
            pt[0] = i * 2
            tree.Fill()
        f.Write("variables")

        # Broken file
        f = ROOT.TFile('K+:generic ==> K+:FSP.root', 'RECREATE')

        f = ROOT.TFile('D0:generic ==> K-:generic pi+:generic.root', 'RECREATE')
        f.cd()
        tree = ROOT.TTree('variables', 'variables')
        isSignal = np.zeros(1, dtype=float)
        p = np.zeros(1, dtype=float)
        pt = np.zeros(1, dtype=float)
        tree.Branch('isSignal', isSignal, 'isSignal/D')
        tree.Branch('M', p, 'M/D')
        tree.Branch('p', pt, 'p/D')
        # Too few signal events here!
        for i in range(10):
            isSignal[0] = i % 2
            p[0] = i
            pt[0] = i * 2
            tree.Fill()
        f.Write("variables")

        f = ROOT.TFile('D0:generic ==> pi-:generic pi+:generic.root', 'RECREATE')
        f.cd()
        tree = ROOT.TTree('variables', 'variables')
        isSignal = np.zeros(1, dtype=float)
        p = np.zeros(1, dtype=float)
        pt = np.zeros(1, dtype=float)
        tree.Branch('isSignal', isSignal, 'isSignal/D')
        tree.Branch('M', p, 'M/D')
        tree.Branch('p', pt, 'p/D')
        for i in range(1000):
            isSignal[0] = i % 2
            p[0] = i
            pt[0] = i * 2
            tree.Fill()
        f.Write("variables")

    def tearDown(self):
        if os.path.isfile('UNITTEST_TEACHER.xml'):
            os.remove('UNITTEST_TEACHER.xml')
        if os.path.isfile('pi+:generic ==> pi+:FSP.root'):
            os.remove('pi+:generic ==> pi+:FSP.root')
        if os.path.isfile('UNITTEST_pi+:generic ==> pi+:FSP.xml'):
            os.remove('UNITTEST_pi+:generic ==> pi+:FSP.xml')
        if os.path.isfile('K+:generic ==> K+:FSP.root'):
            os.remove('K+:generic ==> K+:FSP.root')
        if os.path.isfile('UNITTEST_K+:generic ==> K+:FSP.xml'):
            os.remove('UNITTEST_K+:generic ==> K+:FSP.xml')
        if os.path.isfile('D0:generic ==> K-:generic pi+:generic.root'):
            os.remove('D0:generic ==> K-:generic pi+:generic.root')
        if os.path.isfile('UNITTEST_D0:generic ==> K-:generic pi+:generic.xml'):
            os.remove('UNITTEST_D0:generic ==> K-:generic pi+:generic.xml')
        if os.path.isfile('D0:generic ==> pi-:generic pi+:generic.root'):
            os.remove('D0:generic ==> pi-:generic pi+:generic.root')
        if os.path.isfile('UNITTEST_D0:generic ==> pi-:generic pi+:generic.xml'):
            os.remove('UNITTEST_D0:generic ==> pi-:generic pi+:generic.xml')

    def test_create_fake_weightfile(self):
        self.assertEqual(os.path.isfile('UNITTEST_pi+:generic ==> pi+:FSP.xml'), False)
        self.assertEqual(basf2_mva.available('UNITTEST_pi+:generic ==> pi+:FSP.xml'), False)
        fei.core.Teacher.create_fake_weightfile('UNITTEST_pi+:generic ==> pi+:FSP')
        self.assertEqual(os.path.isfile('UNITTEST_pi+:generic ==> pi+:FSP.xml'), True)
        self.assertEqual(basf2_mva.available('UNITTEST_pi+:generic ==> pi+:FSP.xml'), True)

    def test_upload(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False, prefix='UNITTEST', externTeacher='basf2_mva_teacher')
        x = fei.core.Teacher(particles, config)
        fei.core.Teacher.create_fake_weightfile('TEACHER')
        self.assertEqual(basf2_mva.available('UNITTEST_TEACHER'), False)
        r = x.upload('TEACHER')
        self.assertEqual(basf2_mva.available('UNITTEST_TEACHER'), True)
        self.assertEqual(r, ('TEACHER.xml', 'UNITTEST_TEACHER'))

    def test_without_monitoring(self):
        particles = get_small_unittest_channels()
        config = fei.config.FeiConfiguration(monitor=False, prefix='UNITTEST', externTeacher='basf2_mva_teacher')
        x = fei.core.Teacher(particles, config)

        self.assertEqual(basf2_mva.available('UNITTEST_pi+:generic ==> pi+:FSP'), False)
        self.assertEqual(basf2_mva.available('UNITTEST_K+:generic ==> K+:FSP'), False)
        self.assertEqual(basf2_mva.available('UNITTEST_D0:generic ==> K-:generic pi+:generic'), False)
        self.assertEqual(basf2_mva.available('UNITTEST_D0:generic ==> pi-:generic pi+:generic'), False)

        x.do_all_trainings()

        self.assertEqual(basf2_mva.available('UNITTEST_pi+:generic ==> pi+:FSP'), True)
        self.assertEqual(basf2_mva.available('UNITTEST_K+:generic ==> K+:FSP'), True)
        self.assertEqual(basf2_mva.available('UNITTEST_D0:generic ==> K-:generic pi+:generic'), True)
        self.assertEqual(basf2_mva.available('UNITTEST_D0:generic ==> pi-:generic pi+:generic'), True)


"""
class TestConvertLegacyTraining(unittest.TestCase):
    pass
"""


class TestGetPath(unittest.TestCase):

    def setUp(self):
        particles = fei.get_unittest_channels()

        f = ROOT.TFile('mcParticlesCount.root', 'RECREATE')
        f.cd()

        for pdgnumber in set([abs(pdg.from_name(particle.name)) for particle in particles]):
            hist = ROOT.TH1F("NumberOfMCParticlesInEvent__bo{}__bc".format(pdgnumber),
                             "NumberOfMCParticlesInEvent__bo{}__bc".format(pdgnumber), 11, -0.5, 10.5)
            for i in range(10):
                hist.Fill(5)
            f.Write("NumberOfMCParticlesInEvent__bo{}__bc".format(pdgnumber))

    def tearDown(self):
        if os.path.isfile('mcParticlesCount.root'):
            os.remove('mcParticlesCount.root')
        if os.path.isfile('Summary.pickle'):
            os.remove('Summary.pickle')

    def test_get_path_default_cache(self):
        particles = fei.get_unittest_channels()
        config = fei.config.FeiConfiguration(training=True)
        x = fei.core.Teacher(particles, config)

        # Should try to create mcParticlesCount
        # -> Returns at stage 0
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 0)

        # Should try to create training data for FSPs
        # -> Returns at stage 1
        config = fei.config.FeiConfiguration(training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 1)

        # No weightfiles were created, hence the algorithm
        # cannot go forward and tries to create the training data again
        config = fei.config.FeiConfiguration(training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 1)

        # We create the FSP weightfiles by hand
        fei.core.Teacher.create_fake_weightfile('pi+:generic ==> pi+:FSP')
        fei.core.Teacher.create_fake_weightfile('K+:generic ==> K+:FSP')
        fei.core.Teacher.create_fake_weightfile('mu+:generic ==> mu+:FSP')
        fei.core.Teacher.create_fake_weightfile('gamma:generic ==> gamma:FSP')
        fei.core.Teacher.create_fake_weightfile('gamma:generic ==> gamma:V0')

        # Should try to create training data for pi0
        # -> Returns at stage 2
        config = fei.config.FeiConfiguration(training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 2)

        # No weightfiles were created, hence the algorithm
        # cannot go forward and tries to create the training data again
        config = fei.config.FeiConfiguration(training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 2)

        # We create the pi0 weightfile by hand
        fei.core.Teacher.create_fake_weightfile('pi0:generic ==> gamma:generic gamma:generic')

        # Should try to create training data for D0
        # -> Returns stage 4 (stage 3 is skipped because it only contains K_S0)
        config = fei.config.FeiConfiguration(training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 4)

        # We create the D0 weightfiles by hand
        fei.core.Teacher.create_fake_weightfile('D0:generic ==> K-:generic pi+:generic')
        fei.core.Teacher.create_fake_weightfile('D0:generic ==> K-:generic pi+:generic pi0:generic')
        fei.core.Teacher.create_fake_weightfile('D0:generic ==> pi-:generic pi+:generic')
        fei.core.Teacher.create_fake_weightfile('D0:semileptonic ==> K-:generic mu+:generic')
        fei.core.Teacher.create_fake_weightfile('D0:semileptonic ==> K-:generic pi0:generic mu+:generic')

        # Unittest channels do not contain anymore stages,
        # -> Returns last stage + 1
        config = fei.config.FeiConfiguration(training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 7)

        # If we start at 0, we should be get the whole path
        # -> Returns last stage + 1
        config = fei.config.FeiConfiguration(cache=0, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 7)

        # If training is False we should get the whole path without
        # checking if weightfiles exist.
        os.remove('pi+:generic ==> pi+:FSP.xml')
        os.remove('K+:generic ==> K+:FSP.xml')
        os.remove('mu+:generic ==> mu+:FSP.xml')
        os.remove('gamma:generic ==> gamma:FSP.xml')
        os.remove('gamma:generic ==> gamma:V0.xml')
        os.remove('pi0:generic ==> gamma:generic gamma:generic.xml')
        os.remove('D0:generic ==> K-:generic pi+:generic.xml')
        os.remove('D0:generic ==> K-:generic pi+:generic pi0:generic.xml')
        os.remove('D0:generic ==> pi-:generic pi+:generic.xml')
        os.remove('D0:semileptonic ==> K-:generic mu+:generic.xml')
        os.remove('D0:semileptonic ==> K-:generic pi0:generic mu+:generic.xml')

        config = fei.config.FeiConfiguration(cache=0, training=False)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 7)

    def test_get_path(self):
        particles = fei.get_unittest_channels()
        config = fei.config.FeiConfiguration(cache=-1, training=True)
        x = fei.core.Teacher(particles, config)

        # Should try to create mcParticlesCount
        # -> Returns at stage 0
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 0)

        # Should try to create training data for FSPs
        # -> Returns at stage 1
        config = fei.config.FeiConfiguration(cache=0, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 1)

        # No weightfiles were created, hence the algorithm
        # cannot go forward and tries to create the training data again
        config = fei.config.FeiConfiguration(cache=1, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 1)

        # We create the FSP weightfiles by hand
        fei.core.Teacher.create_fake_weightfile('pi+:generic ==> pi+:FSP')
        fei.core.Teacher.create_fake_weightfile('K+:generic ==> K+:FSP')
        fei.core.Teacher.create_fake_weightfile('mu+:generic ==> mu+:FSP')
        fei.core.Teacher.create_fake_weightfile('gamma:generic ==> gamma:FSP')
        fei.core.Teacher.create_fake_weightfile('gamma:generic ==> gamma:V0')

        # Should try to create training data for pi0
        # -> Returns at stage 2
        config = fei.config.FeiConfiguration(cache=1, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 2)

        # No weightfiles were created, hence the algorithm
        # cannot go forward and tries to create the training data again
        config = fei.config.FeiConfiguration(cache=2, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 2)

        # No weightfiles were created, hence the algorithm
        # cannot go forward and tries to create the training data again
        # This applies as well if the stage is even bigger
        config = fei.config.FeiConfiguration(cache=4, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 2)

        # We create the pi0 weightfile by hand
        fei.core.Teacher.create_fake_weightfile('pi0:generic ==> gamma:generic gamma:generic')

        # Should try to create training data for D0
        # -> Returns stage 4 (stage 3 is skipped because it only contains K_S0)
        config = fei.config.FeiConfiguration(cache=2, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 4)

        # We create the D0 weightfiles by hand
        fei.core.Teacher.create_fake_weightfile('D0:generic ==> K-:generic pi+:generic')
        fei.core.Teacher.create_fake_weightfile('D0:generic ==> K-:generic pi+:generic pi0:generic')
        fei.core.Teacher.create_fake_weightfile('D0:generic ==> pi-:generic pi+:generic')
        fei.core.Teacher.create_fake_weightfile('D0:semileptonic ==> K-:generic mu+:generic')
        fei.core.Teacher.create_fake_weightfile('D0:semileptonic ==> K-:generic pi0:generic mu+:generic')

        # Unittest channels do not contain anymore stages,
        # -> Returns last stage + 1
        config = fei.config.FeiConfiguration(cache=4, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 7)

        # If we start at 0, we should be get the whole path
        # -> Returns last stage + 1
        config = fei.config.FeiConfiguration(cache=0, training=True)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 7)

        # If training is False we should get the whole path without
        # checking if weightfiles exist.
        os.remove('pi+:generic ==> pi+:FSP.xml')
        os.remove('K+:generic ==> K+:FSP.xml')
        os.remove('mu+:generic ==> mu+:FSP.xml')
        os.remove('gamma:generic ==> gamma:FSP.xml')
        os.remove('gamma:generic ==> gamma:V0.xml')
        os.remove('pi0:generic ==> gamma:generic gamma:generic.xml')
        os.remove('D0:generic ==> K-:generic pi+:generic.xml')
        os.remove('D0:generic ==> K-:generic pi+:generic pi0:generic.xml')
        os.remove('D0:generic ==> pi-:generic pi+:generic.xml')
        os.remove('D0:semileptonic ==> K-:generic mu+:generic.xml')
        os.remove('D0:semileptonic ==> K-:generic pi0:generic mu+:generic.xml')

        config = fei.config.FeiConfiguration(cache=0, training=False)
        feistate = fei.core.get_path(particles, config)
        self.assertEqual(feistate.stage, 7)


if __name__ == '__main__':
    # We have to call basf2_mva once, so that ROOT can load the dictionaries
    # otherwise it will try to load them later after we changed the directory and it will fail to do so
    basf2_mva.loadRootDictionary()
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    basf2.use_local_database(tempdir + '/localdb/dbcache.txt', tempdir + '/localdb/', False, basf2.LogLevel.WARNING)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
