#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# subprocess has to be
from fei.provider import *

from basf2 import *
import unittest
import unittest.mock
import os
import tempfile
import atexit
import shutil
import contextlib
import IPython
import subprocess
from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration, DecayChannel

# @cond

# Define equality operators for a bunch of pybasf2 classes
import pybasf2
pybasf2.Module.__eq__ = lambda a, b: a.type() == b.type() and\
    all(x == y for x, y in zip(a.available_params(), b.available_params()))
pybasf2.ModuleParamInfo.__eq__ = lambda a, b: a.name == b.name and a.values == b.values
pybasf2.Path.__eq__ = lambda a, b: all(x == y for x, y in zip(a.modules(), b.modules()))


class MockResource(object):

    def __init__(self, hash='42', env=None, needed=False, cache=False, halt=False, condition=None, path=None,
                 usesMultiThreading=False):
        self.hash = hash
        self.env = {} if env is None else env
        self.needed = needed
        self.cache = cache
        self.halt = halt
        self.condition = condition
        self.path = create_path() if path is None else path
        self.usesMultiThreading = usesMultiThreading

    def __eq__(self, a):
        return self.__dict__ == a.__dict__

    def __repr__(self):
        output = str(self.__dict__)
        for m in self.path.modules():
            output += '\n' + str(m) + ' '
            for p in m.available_params():
                output += str(p.name) + ':' + str(p.values) + ' '
        return output

    @contextlib.contextmanager
    def EnableMultiThreading(self):
        self.usesMultiThreading = True
        yield


class TestHashRequirements(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_HashRequirements(self):
        # Returns hash
        self.assertEqual(HashRequirements(self.resource), '42')
        # Enables caching
        self.assertEqual(self.resource, MockResource(cache=True))


class TestPDGConjugate(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_PDGConjugate(self):
        # Returns charge conjugated list
        self.assertEqual(PDGConjugate(self.resource, 'e+:generic'), 'e-:generic')
        # Enables caching
        self.assertEqual(self.resource, MockResource(cache=True))
        # Returns same list for self-conjugated particles
        self.assertEqual(PDGConjugate(self.resource, 'J/psi:generic'), 'J/psi:generic')


class TestLoadGearbox(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_LoadGearbox(self):
        # Returns nothing
        self.assertIs(LoadGearbox(self.resource), None)
        result = MockResource()
        # Adds Gearbox
        result.path.add_module('Gearbox')
        self.assertEqual(self.resource, result)


class TestLoadGeometry(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_LoadGeometry(self):
        # Returns nothing
        self.assertIs(LoadGeometry(self.resource), None)
        result = MockResource()
        # Adds Geometry
        result.path.add_module('Geometry', components=['MagneticField'])
        self.assertEqual(self.resource, result)


class TestMatchParticleList(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource(env={'ROE': False})

    def test_MatchParticleList(self):
        # Returns name of ParticleList
        self.assertEqual(MatchParticleList(self.resource, 'e+:generic', 'isSignal'), 'e+:generic')
        # Enables caching and MC-data condition
        result = MockResource(env={'ROE': False}, cache=True, condition=('EventType', '==0'))
        # Adds MCMatching for given particle List
        result.path.add_module('MCMatcherParticles', listName='e+:generic')
        result.path.add_module('ParticleSelector', decayString='e+:generic', cut='')
        self.assertEqual(self.resource, result)

    def test_MatchParticleListInROE(self):
        # Set environment to ROE
        self.resource.env['ROE'] = 'B+'
        # Returns name of ParticleList
        self.assertEqual(MatchParticleList(self.resource, 'e+:generic', 'isSignal'), 'e+:generic')
        # Enables caching and MC-data condition
        result = MockResource(env={'ROE': 'B+'}, cache=True, condition=('EventType', '==0'))
        # Adds MCMatching for given particle List
        cut = '[eventCached(countInList(B+, isSignalAcceptMissingNeutrino == 1)) > 0 and isSignal == 1] or '
        cut += 'eventCached(countInList(B+, isSignalAcceptMissingNeutrino == 1)) == 0'
        result.path.add_module('MCMatcherParticles', listName='e+:generic')
        result.path.add_module('ParticleSelector', decayString='e+:generic', cut=cut)
        self.assertEqual(self.resource, result)

    def test_MatchParticleListWithNone(self):
        # Returns None if given ParticleList is None
        self.assertEqual(MatchParticleList(self.resource, None, 'isSignal'), None)
        # Enables caching
        result = MockResource(env={'ROE': False}, cache=True)
        self.assertEqual(self.resource, result)


class TestMakeParticleList(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource(env={'ROE': False})

    def test_MakeParticleList(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], {'cutstring': '1.5 < M < 2.0'}, 'p > 3', 23), 'D0:42')
        # Enables caching
        result = MockResource(cache=True, env={'ROE': False})
        # Adds ParticleCombiner for given decay
        result.path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                               maximumNumberOfCandidates=1000,
                               decayMode=23, cut='[p > 3] and [1.5 < M < 2.0]')
        self.assertEqual(self.resource, result)

    def test_MakeParticleListEmptyUserCut(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], {'cutstring': '1.5 < M < 2.0'}, '', 23), 'D0:42')
        # Enables caching
        result = MockResource(cache=True, env={'ROE': False})
        # Adds ParticleCombiner for given decay
        result.path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                               maximumNumberOfCandidates=1000,
                               decayMode=23, cut='1.5 < M < 2.0')
        self.assertEqual(self.resource, result)

    def test_MakeParticleListEmptyPreCut(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], {'cutstring': ''}, 'p > 3', 23), 'D0:42')
        # Enables caching
        result = MockResource(cache=True, env={'ROE': False})
        # Adds ParticleCombiner for given decay
        result.path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                               maximumNumberOfCandidates=1000,
                               decayMode=23, cut='p > 3')
        self.assertEqual(self.resource, result)

    def test_MakeParticleListEmptyPreCutEmptyUserCut(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], {'cutstring': ''}, '', 23), 'D0:42')
        # Enables caching
        result = MockResource(cache=True, env={'ROE': False})
        # Adds ParticleCombiner for given decay
        result.path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                               maximumNumberOfCandidates=1000,
                               decayMode=23, cut='')
        self.assertEqual(self.resource, result)

    def test_MakeParticleListWithNone(self):
        # Returns None if given PreCut is None
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], None, '', 23), None)
        # Enables caching
        result = MockResource(cache=True, env={'ROE': False})
        self.assertEqual(self.resource, result)

    def test_MakeParticleListFSP(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'e+', ['e+'], {'cutstring': ''}, 'eid > 0.2', 23), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': False}, cache=True)
        # Adds ParticleLoader
        result.path.add_module('ParticleListManipulator', inputListNames=['e+'],
                               outputListName='e+:42', cut='eid > 0.2', writeOut=True)
        result.path.add_module('VariablesToExtraInfo', particleList='e+:42', variables={'constant(23)': 'decayModeID'})
        self.assertEqual(self.resource, result)

    def test_MakeParticleListFSPWithoutCut(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'e+', ['e+'], {'cutstring': ''}, '', 23), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': False}, cache=True)
        # Adds ParticleLoader
        result.path.add_module('ParticleListManipulator', inputListNames=['e+'],
                               outputListName='e+:42', cut='', writeOut=True)
        result.path.add_module('VariablesToExtraInfo', particleList='e+:42', variables={'constant(23)': 'decayModeID'})
        self.assertEqual(self.resource, result)

    def test_MakeParticleListFSPInROE(self):
        # Set environment to ROE
        self.resource.env['ROE'] = 'B+'
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'e+', ['e+'], {'cutstring': ''}, 'eid > 0.2', 23), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': 'B+'}, cache=True)
        # Adds ParticleLoader with cut
        result.path.add_module('ParticleListManipulator', inputListNames=['e+'],
                               outputListName='e+:42', cut='[eid > 0.2] and [isInRestOfEvent > 0.5]', writeOut=True)
        result.path.add_module('VariablesToExtraInfo', particleList='e+:42', variables={'constant(23)': 'decayModeID'})
        self.assertEqual(self.resource, result)

    def test_MakeParticleListFSPInROEWithoutCut(self):
        # Set environment to ROE
        self.resource.env['ROE'] = 'B+'
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'e+', ['e+'], {'cutstring': ''}, '', 23), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': 'B+'}, cache=True)
        # Adds ParticleLoader with cut
        result.path.add_module('ParticleListManipulator', inputListNames=['e+'],
                               outputListName='e+:42', cut='isInRestOfEvent > 0.5', writeOut=True)
        result.path.add_module('VariablesToExtraInfo', particleList='e+:42', variables={'constant(23)': 'decayModeID'})
        self.assertEqual(self.resource, result)


class TestCopyParticleLists(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.postCut = {'cutstring': '0.1 < extraInfo(SignalProbability)'}

    def test_CopyParticleLists(self):
        # Returns name of ParticleList
        self.assertEqual(CopyParticleLists(self.resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                           self.postCut, ['S1', 'S2', 'S3']), 'D0:42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleListManipulator for given decay
        result.path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:2', 'D0:3'],
                               writeOut=True, cut=self.postCut['cutstring'])
        self.assertEqual(self.resource, result)

    def test_CopyParticleListsSomeNone(self):
        # Returns name of ParticleList
        self.assertEqual(CopyParticleLists(self.resource, 'D0', 'generic', ['D0:1', None, 'D0:3'],
                                           self.postCut, ['S1', 'S2', 'S3']), 'D0:42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleListManipulator for given decay
        result.path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:3'],
                               writeOut=True, cut=self.postCut['cutstring'])
        self.assertEqual(self.resource, result)

    def test_CopyParticleListsAllNone(self):
        # Returns None if all ParticleLists are None
        self.assertEqual(CopyParticleLists(self.resource, 'D0', 'generic', [None, None, None],
                                           self.postCut, ['S1', 'S2', 'S3']), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CopyParticleListsSomeSignalProbabilitiesNone(self):
        # Returns name of ParticleList
        self.assertEqual(CopyParticleLists(self.resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                           self.postCut, ['S1', None, 'S3']), 'D0:42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleListManipulator for given decay
        result.path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:3'],
                               writeOut=True, cut=self.postCut['cutstring'])
        self.assertEqual(self.resource, result)

    def test_CopyParticleListsAllSignalProbabilitiesNone(self):
        # Returns None if all ParticleLists are None
        self.assertEqual(CopyParticleLists(self.resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                           self.postCut, [None, None, None]), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestCopyIntoHumanReadableParticleList(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_CopyIntoHumanReadableParticleList(self):
        # Returns name of ParticleList
        self.assertEqual(CopyIntoHumanReadableParticleList(self.resource, 'D0', 'human', 'D0:123'), 'D0:human')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleListManipulator for given decay
        result.path.add_module('ParticleListManipulator', outputListName='D0:human', inputListNames=['D0:123'], writeOut=True)
        self.assertEqual(self.resource, result)

    def test_CopyIntoHumanReadableParticleListWithNone(self):
        # Returns None if given ParticleList is None
        self.assertEqual(CopyIntoHumanReadableParticleList(self.resource, 'D0', 'human', None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestFitVertex(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_FitVertex(self):
        # Returns hash
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi-', 'D0:generic -> K+ pi-', -2), '42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleVertexFitter for given ParticleList
        result.path.add_module('ParticleVertexFitter', listName='D0:generic -> K+ pi-', confidenceLevel=-2,
                               vertexFitter='kfitter', fitType='vertex')
        self.assertEqual(self.resource, result)

    def test_FitVertexOtherConfidenceLevel(self):
        # Returns hash
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi-', 'D0:generic -> K+ pi-', 0.001), '42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleVertexFitter for given ParticleList
        result.path.add_module('ParticleVertexFitter', listName='D0:generic -> K+ pi-', confidenceLevel=0.001,
                               vertexFitter='kfitter', fitType='vertex')
        self.assertEqual(self.resource, result)

    def test_FitVertexWithNone(self):
        # Returns None if ParticleList is None
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi-', None, -2), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_FitVertexWithTwoPi0(self):
        # Returns None if ParticleList contains pi0 pi0
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi- pi0 pi0', 'D0:generic -> K+ pi-', -2), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


@contextlib.contextmanager
def temporary_file(filename):
    open(filename, 'a').close()
    yield
    os.remove(filename)


class TestCreatePreCutHistogram(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.preCutConfig = PreCutConfiguration('M', (500, 2, 4), 0.7, 0.01)
        self.userCut = 'M > 0.2'

    def test_CreatePreCutHistogram(self):
        # Returns None if Histogram does not exists
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, self.userCut, ['pi+:1', 'K+:1'], ['bar', 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds PreCutHistMaker for given ParticleList
        result.path.add_module('PreCutHistMaker', fileName='CutHistograms_UniqueChannelName:42.root',
                               decayString='D+:42 ==> pi+:1 K+:1', cut=self.userCut,
                               target='isSignal', variable=self.preCutConfig.variable,
                               maximumNumberOfCandidates=1000,
                               histParams=self.preCutConfig.binning)
        self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramCustomBinning(self):
        # Set Custom binning
        self.preCutConfig = PreCutConfiguration('M', list(range(10)), 0.7, 0.01)
        # Returns None if Histogram does not exists
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, self.userCut, ['pi+:1', 'K+:1'], ['bar', 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds PreCutHistMaker for given ParticleList
        result.path.add_module('PreCutHistMaker', fileName='CutHistograms_UniqueChannelName:42.root',
                               decayString='D+:42 ==> pi+:1 K+:1', cut=self.userCut,
                               target='isSignal', variable=self.preCutConfig.variable,
                               maximumNumberOfCandidates=1000,
                               customBinning=self.preCutConfig.binning)
        self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramWithFile(self):
        with temporary_file('CutHistograms_UniqueChannelName:42.root'):
            # Returns Tuple if Histogram does exists
            self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                                   self.preCutConfig, self.userCut, ['pi+:1', 'K+:1'], ['bar', 'foo']),
                             ('CutHistograms_UniqueChannelName:42.root', 'D+:42'))
            # Enables caching, halt and condition
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramMissingDaughter(self):
        # Returns None if daughter particle is missing
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, self.userCut, [None, 'K+:1'], ['bar', 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramMissingAdditionalDependency(self):
        # Returns None if additional dependency is missing
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, self.userCut, ['pi+:1', 'K+:1'], [None, 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestPreCutDeterminationPerChannel(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_PreCutDeterminationPerChannel(self):
        # Returns value (preCut) stored in given dictionary (preCuts for all channels) under key (channelName)
        self.assertEqual(PreCutDeterminationPerChannel(self.resource, 'key', {'key': 42}), 42)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_PreCutDeterminationPerChannelWithNone(self):
        # Returns None if given PreCuts is None
        self.assertEqual(PreCutDeterminationPerChannel(self.resource, 'key', None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


def MockCalculatePreCuts(preCutConfig, channelNames, preCutHistograms):
    B2INFO('Called MockCalculatePreCuts')
    assert preCutConfig == PreCutConfiguration('M', (500, 2, 4), 0.7, 0.01)
    assert channelNames == ('D0:1 ==> K+ pi-', 'D0:2 ==> K+ pi- pi0', 'D0:3 ==> K+ pi+ pi- pi-')
    assert preCutHistograms == ('Dummy1', 'Dummy2', 'Dummy3')
    return {'D0:1 ==> K+ pi-': {'cutstring': '0.1 < M < 0.2', 'isIgnored': False},
            'D0:2 ==> K+ pi- pi0': {'cutstring': '0.15 < M < 0.18', 'isIgnored': False},
            'D0:3 ==> K+ pi+ pi- pi-': {'cutstring': '', 'isIgnored': True}}


class TestPreCutDetermination(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.preCutConfig = PreCutConfiguration('M', (500, 2, 4), 0.7, 0.01)
        self.channelNames = ['D0:1 ==> K+ pi-', 'D0:2 ==> K+ pi- pi0', 'D0:3 ==> K+ pi+ pi- pi-', 'D0:4 ==> K+ K-']
        self.preCutHistograms = ['Dummy1', 'Dummy2', 'Dummy3', None]

    def test_PreCutDetermination(self):
        from fei import preCutDetermination
        preCutDetermination.CalculatePreCuts = MockCalculatePreCuts
        # Returns value (preCut) stored in given dictionary (preCuts for all channels) under key (channelName)
        self.assertDictEqual(PreCutDetermination(self.resource, self.channelNames, self.preCutConfig, self.preCutHistograms),
                             {'D0:1 ==> K+ pi-': {'cutstring': '0.1 < M < 0.2', 'isIgnored': False},
                              'D0:2 ==> K+ pi- pi0': {'cutstring': '0.15 < M < 0.18', 'isIgnored': False},
                              'D0:3 ==> K+ pi+ pi- pi-': None,
                              'D0:4 ==> K+ K-': None})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_PreCutDeterminationAllNone(self):
        # Returns None if given PreCutHistograms are all None
        self.assertEqual(PreCutDetermination(self.resource, self.channelNames, self.preCutConfig, [None, None, None, None]), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestPostCutDetermination(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_PostCutDetermination(self):
        # Returns PostCut dictionary with cutstring and range
        self.assertDictEqual(PostCutDetermination(self.resource, PostCutConfiguration(0.123)),
                             {'cutstring': '0.123 < extraInfo(SignalProbability)', 'range': (0.123, 1)})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_PostCutDeterminationWithZero(self):
        # Returns None if given PostCutConfig is None
        self.assertEqual(PostCutDetermination(self.resource, PostCutConfiguration(0.0)),
                         {'cutstring': '', 'range': (0.0, 1)})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestCalculateInverseSamplingRate(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_CalculateInverseSamplingRate(self):
        # Returns dictionary with inverse sampling rates
        self.assertDictEqual(CalculateInverseSamplingRate(self.resource, {'nSignal': 42, 'nBackground': 23}),
                             {})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CalculateInverseSamplingRateSignal(self):
        # Returns dictionary with inverse sampling rates
        self.assertDictEqual(CalculateInverseSamplingRate(self.resource, {'nSignal': 42000000, 'nBackground': 23}),
                             {1: 5})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CalculateInverseSamplingRateBackground(self):
        # Returns dictionary with inverse sampling rates
        self.assertDictEqual(CalculateInverseSamplingRate(self.resource, {'nSignal': 42, 'nBackground': 23000000}),
                             {0: 3})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CalculateInverseSamplingRateBoth(self):
        # Returns dictionary with inverse sampling rates
        self.assertDictEqual(CalculateInverseSamplingRate(self.resource, {'nSignal': 42000000, 'nBackground': 23000000}),
                             {0: 3, 1: 5})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CalculateInverseSamplingRateWithNone(self):
        # Returns None if distribution is None
        self.assertEqual(CalculateInverseSamplingRate(self.resource, None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestCalculateNumberOfBins(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_CalculateNumberOfBinsLowStatistic(self):
        # Returns Nbins config string for TMVA
        self.assertEqual(CalculateNumberOfBins(self.resource, {'nSignal': 10000}),
                         'CreateMVAPdfs:NbinsMVAPdf=50:')
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CalculateNumberOfBinsMediumStatistic(self):
        # Returns Nbins config string for TMVA
        self.assertEqual(CalculateNumberOfBins(self.resource, {'nSignal': 1e5 + 1}),
                         'CreateMVAPdfs:NbinsMVAPdf=100:')
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CalculateNumberOfBinsHighStatistic(self):
        # Returns Nbins config string for TMVA
        self.assertEqual(CalculateNumberOfBins(self.resource, {'nSignal': 1e6 + 1}),
                         'CreateMVAPdfs:NbinsMVAPdf=200:')
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestGenerateTrainingData(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model=None)

    def test_GenerateTrainingData(self):
        # Returns None if TrainingData does not exists
        self.assertEqual(GenerateTrainingData(self.resource, 'D0:1', self.mvaConfig, {0: 3}, None),
                         None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds TMVATeacher for given ParticleList
        result.path.add_module('TMVATeacher', prefix='D0:1_42',
                               variables=['p', 'pt'],
                               sample='isSignal',
                               spectators=['isSignal'],
                               listNames=['D0:1'],
                               inverseSamplingRates={0: 3},
                               maxSamples=int(2e7))
        self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataWithAdditionalDependencies(self):
        # Returns None if TrainingData does not exists
        self.assertEqual(GenerateTrainingData(self.resource, 'D0:1', self.mvaConfig, {0: 3}, ['VertexFit1', 'VertexFit2']),
                         None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds TMVATeacher for given ParticleList
        result.path.add_module('TMVATeacher', prefix='D0:1_42',
                               variables=['p', 'pt'],
                               sample='isSignal',
                               spectators=['isSignal'],
                               listNames=['D0:1'],
                               inverseSamplingRates={0: 3},
                               maxSamples=int(2e7))
        self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataWithFile(self):
        with temporary_file('D0:1_42.root'):
            # Returns filename if TrainingData does exists
            self.assertEqual(GenerateTrainingData(self.resource, 'D0:1', self.mvaConfig, {0: 3}, None),
                             'D0:1_42.root')
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataWithUnfullfilledDependencies(self):
        # Returns None if Dependency is not fulfilled
        self.assertEqual(GenerateTrainingData(self.resource, 'D0:1', self.mvaConfig, {0: 3}, ['Vertex1', None]),
                         None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataMissingParticleList(self):
        # Returns None if ParticleList is None
        self.assertEqual(GenerateTrainingData(self.resource, None, self.mvaConfig, {0: 3}, None),
                         None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestGenerateSPlotModel(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.distribution = {'nSignal': 10, 'nBackground': 30, 'variable': 'M',
                             'signalPeak': 1.8, 'signalWidth': 0.1, 'range': (1.4, 1.9)}

    def test_GenerateSPlotModel(self):
        mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model='M')
        # Returns dictionary with sPlotParameters
        self.assertListEqual(GenerateSPlotModel(self.resource, 'Name', mvaConfig, self.distribution),
                             [{'cut': '1.5 < M < 1.9'},
                              {'modelFileName': 'model_Name_42.root',
                               'discriminatingVariables': ['M']}])
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_GenerateSPlotModelRaiseRuntimeError(self):
        mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model=None)
        with self.assertRaises(RuntimeError):
            GenerateSPlotModel(self.resource, 'Name', mvaConfig, self.distribution)

    def test_GenerateSPlotModelMissingDistribution(self):
        mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model='M')
        # Returns None if distribution is None
        self.assertEqual(GenerateSPlotModel(self.resource, 'Name', mvaConfig, None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestGenerateTrainingDataUsingSPlot(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model='M')
        self.sPlotParameters = [{'cut': '1.8 < M < 1.9'},
                                {'modelFileName': 'FileName',
                                 'discriminatingVariables': ['M']}]

    def test_GenerateTrainingDataUsingSPlot(self):
        # Returns None if TrainingData does not exists
        self.assertEqual(GenerateTrainingDataUsingSPlot(self.resource, 'D0:1', self.mvaConfig, self.sPlotParameters,
                                                        None), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds TMVATeacher for given ParticleList
        result.path.add_module('ParticleListManipulator', outputListName='D0:1_tmp',
                               inputListNames='D0:1',
                               cut='1.8 < M < 1.9',
                               writeOut=False)
        result.path.add_module('TMVATeacher', prefix='D0:1_42',
                               variables=['p', 'pt'],
                               listNames=['D0:1_tmp'],
                               spectators=['M'],
                               maxSamples=int(2e7))
        self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataUsingSPlotWithAdditionalDependencies(self):
        # Returns None if TrainingData does not exists
        self.assertEqual(GenerateTrainingDataUsingSPlot(self.resource, 'D0:1', self.mvaConfig, self.sPlotParameters,
                                                        ['VertexFit1', 'VertexFit2']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds TMVATeacher for given ParticleList
        result.path.add_module('ParticleListManipulator', outputListName='D0:1_tmp',
                               inputListNames='D0:1',
                               cut='1.8 < M < 1.9',
                               writeOut=False)
        result.path.add_module('TMVATeacher', prefix='D0:1_42',
                               variables=['p', 'pt'],
                               listNames=['D0:1_tmp'],
                               spectators=['M'],
                               maxSamples=int(2e7))
        self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataUsingSPlotWithFile(self):
        with temporary_file('D0:1_42.root'):
            # Returns filename if TrainingData does exists
            self.assertEqual(GenerateTrainingDataUsingSPlot(self.resource, 'D0:1', self.mvaConfig, self.sPlotParameters, None),
                             'D0:1_42.root')
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataUsingSPlotWithUnfullfilledDependencies(self):
        # Returns None if Dependency is not fulfilled
        self.assertEqual(GenerateTrainingData(self.resource, 'D0:1', self.mvaConfig, self.sPlotParameters, ['Vertex1', None]),
                         None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_GenerateTrainingDataUsingSPlotMissingParticleList(self):
        # Returns None if ParticleList is None
        self.assertEqual(GenerateTrainingData(self.resource, None, self.mvaConfig, self.sPlotParameters, None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


def SubprocessSuccessSideEffect(command, shell):
    open('trainingData_1.config', 'w').close()


class TestTrainMultivariateClassifier(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource(env={'externTeacher': 'externTeacher'})
        self.mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model=None)

    def test_TrainMultivariateClassifierAlreadyTrained(self):
        with temporary_file('trainingData_1.config'):
            # Returns config filename if training was sucessfull
            self.assertEqual(
                TrainMultivariateClassifier(
                    self.resource, self.mvaConfig, 'CreateMVAPdfs:Nbins=100:', 'trainingData.root'),
                'trainingData_1.config')
            # Enables caching
            result = MockResource(cache=True, env={'externTeacher': 'externTeacher'})
            self.assertEqual(self.resource, result)

    def test_TrainMultivariateClassifierMissingTrainingData(self):
        # Returns None if training is not possible due to missing training data
        self.assertEqual(TrainMultivariateClassifier(self.resource, self.mvaConfig,
                                                     'CreateMVAPdfs:Nbins=100:', None), None)
        # Enables caching
        result = MockResource(cache=True, env={'externTeacher': 'externTeacher'})
        self.assertEqual(self.resource, result)

    def test_TrainMultivariateClassifierTrainingFailed(self):
        original_call = subprocess.call
        subprocess.call = unittest.mock.create_autospec(subprocess.call)
        # Returns None if training was did not create correct file
        # with self.assertRaises(RuntimeError):
        self.assertEqual(TrainMultivariateClassifier(self.resource, self.mvaConfig, 'CreateMVAPdfs:Nbins=100:',
                                                     'trainingData.root'), None)
        subprocess.call.assert_called_once_with("externTeacher --methodName 'FastBDT' --methodType 'Plugin'"
                                                " --methodConfig 'CreateMVAPdfs:Nbins=100:TMVAConfigString'"
                                                " --target 'isSignal' --variables 'p' 'pt' --prefix 'trainingData'"
                                                " > 'trainingData'.log 2>&1", shell=True)
        subprocess.call = original_call

    def test_TrainMultivariateClassifierTrainingSuccessfull(self):
        original_call = subprocess.call
        subprocess.call = unittest.mock.create_autospec(subprocess.call,
                                                        side_effect=lambda command, shell: open('trainingData_1.config',
                                                                                                'w').close())
        # Returns config filename if training was did not create correct file
        self.assertEqual(TrainMultivariateClassifier(self.resource, self.mvaConfig, 'CreateMVAPdfs:Nbins=100:',
                                                     'trainingData.root'), 'trainingData_1.config')
        # Enables caching
        result = MockResource(cache=True, usesMultiThreading=True, env={'externTeacher': 'externTeacher'})
        self.assertEqual(self.resource, result)
        subprocess.call.assert_called_once_with("externTeacher --methodName 'FastBDT' --methodType 'Plugin'"
                                                " --methodConfig 'CreateMVAPdfs:Nbins=100:TMVAConfigString'"
                                                " --target 'isSignal' --variables 'p' 'pt' --prefix 'trainingData'"
                                                " > 'trainingData'.log 2>&1", shell=True)
        os.remove('trainingData_1.config')
        subprocess.call = original_call


class TestSignalProbability(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.mvaConfig = MVAConfiguration(name='FastBDT', type='Plugin',
                                          config='TMVAConfigString',
                                          variables=['p', 'pt'],
                                          target='isSignal',
                                          model=None)

    def test_SignalProbability(self):
        # Returns hash if sucessfull
        self.assertEqual(
            SignalProbability(
                self.resource, 'D0:1', self.mvaConfig, 'configMVC_1.config'), '42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds TMVAExpert for given ParticleList
        result.path.add_module('TMVAExpert', prefix='configMVC', method='FastBDT',
                               signalFraction=-1, expertOutputName='SignalProbability',
                               signalClass=1, transformToProbability=True, listNames=['D0:1'])
        self.assertEqual(self.resource, result)

    def test_SignalProbabilityMissingConfigMVC(self):
        # Returns None if training is not possible due to missing MVC weight file
        self.assertEqual(SignalProbability(self.resource, 'D0:1', self.mvaConfig, None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_SignalProbabilityMissingParticleList(self):
        # Returns None if training is not possible due to missing ParticleList
        self.assertEqual(SignalProbability(self.resource, None, self.mvaConfig, 'configMVC_1.config'), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestTagUniqueSignal(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_TagUniqueSignal(self):
        # Returns name of extra Info
        self.assertEqual(TagUniqueSignal(self.resource, 'D0:1', 'signalProbability', 'isSignal'), 'extraInfo(uniqueSignal)')
        # Enables caching
        result = MockResource(cache=True)
        # Adds TagUniqueSignal for given ParticleList
        result.path.add_module('TagUniqueSignal', particleList='D0:1', target='isSignal', extraInfoName='uniqueSignal')
        self.assertEqual(self.resource, result)

    def test_TagUniqueSignalMissingParticleList(self):
        # Returns None if Particle List is missing
        self.assertEqual(TagUniqueSignal(self.resource, None, 'signalProbability', 'isSignal'), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_TagUniqueSignalMissingSignalProbability(self):
        # Returns None if Particle List is missing
        self.assertEqual(TagUniqueSignal(self.resource, 'D0:1', None, 'isSignal'), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestVariablesToNTuple(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_VariablesToNTupleWithFile(self):
        with temporary_file('var_D0:1_42.root'):
            # Returns filename if file already exists
            self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', 'signalProbability', 'isSignal'), 'var_D0:1_42.root')
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)

    def test_VariablesToNTupleMissingParticleList(self):
        # Returns None if particle list is None
        self.assertEqual(VariablesToNTuple(self.resource, None, 'signalProbability', 'isSignal'), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_VariablesToNTupleMissingSignalProbability(self):
        # Returns None if SignalProbability is None
        self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', None, 'isSignal'), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_VariablesToNTuple(self):
        # Returns None if file does not exists
        self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', 'signalProbability', 'isSignal'), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds VariablesToNtuple for given ParticleList
        variables = ['isSignal', 'extraInfo(SignalProbability)', 'Mbc', 'mcErrors',
                     'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)']
        result.path.add_module('VariablesToNtuple', particleList='D0:1', fileName='var_D0:1_42.root',
                               treeName='variables', variables=variables)
        self.assertEqual(self.resource, result)


class TestSaveModuleStatistics(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_SaveModuleStatistics(self):
        # Returns None if file does not exists
        self.assertEqual(SaveModuleStatistics(self.resource), None)
        # Enables caching and halt
        result = MockResource(cache=True, halt=True)
        # Add RooTOutput module for statistics
        result.path.add_module('RootOutput', outputFileName='moduleStatistics_42.root',
                               branchNames=['EventMetaData'], branchNamesPersistent=['ProcessStatistics'],
                               ignoreCommandLineOverride=True)
        self.assertEqual(self.resource, result)

    def test_SaveModuleStatisticsWithFile(self):
        with temporary_file('moduleStatistics_42.root'):
            # Returns filename if file already exists
            self.assertEqual(SaveModuleStatistics(self.resource), 'moduleStatistics_42.root')
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)


class MockTFileAndTH1D(object):

    def __init__(self, filename, *args):
        # If TH1D
        if len(args) > 0:
            assert len(args) == 4
            assert filename == 'allMCParticles'
            assert args[0] == 'allMCParticles'
            assert args[1] == 1
            assert args[2] == 0
            assert args[3] == 100
        # TFile
        else:
            assert filename == 'mcParticlesCount.root'

    def Get(self, branch):
        assert branch == 'mccounts'
        return self

    def GetListOfBranches(self):
        class A(object):

            def __init__(self, pdg):
                self.pdg = pdg

            def GetName(self):
                return "NumberOfMCParticlesInEvent" + str(self.pdg)
        yield A(11)
        yield A(13)

    def GetMaximum(self, branch):
        assert branch == "NumberOfMCParticlesInEvent11" or branch == "NumberOfMCParticlesInEvent13"
        return 99

    def Project(self, name, branch1, branch2):
        assert name == 'allMCParticles'
        assert branch1 == "NumberOfMCParticlesInEvent11" or branch1 == "NumberOfMCParticlesInEvent13"
        assert branch2 == "NumberOfMCParticlesInEvent11" or branch2 == "NumberOfMCParticlesInEvent13"
        assert branch1 == branch2
        MockTFileAndTH1D.current_branch = branch1

    def Integral(self):
        if self.current_branch == 'NumberOfMCParticlesInEvent11':
            return 42
        if self.current_branch == 'NumberOfMCParticlesInEvent13':
            return 23
        return 0

    def GetEntries(self):
        return 65


class TestCountMCParticles(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_CountMCParticles(self):
        # Returns None if file does not exists
        self.assertEqual(CountMCParticles(self.resource, ['e+', 'mu+', 'e-']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Add VariablestoNtuple module for statistics
        result.path.add_module('VariablesToHistogram', fileName='mcParticlesCount.root',
                               variables=[('NumberOfMCParticlesInEvent(11)', 100, -0.5, 99.5),
                                          ('NumberOfMCParticlesInEvent(13)', 100, -0.5, 99.5)])
        self.assertEqual(self.resource, result)

    def test_CountMCParticlesWithFile(self):
        with temporary_file('mcParticlesCount.root'):
            # Returns counts if file already exists
            self.assertEqual(CountMCParticles(self.resource, ['e+', 'mu+', 'e-']), "mcParticlesCount.root")
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)


class TestCountParticleLists(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_CountParticleLists(self):
        # Returns None if file does not exists
        self.assertEqual(CountParticleLists(self.resource, ['isSignal', 'isSignal2'], ['e+:1', 'mu+:2']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Add VariablestoNtuple module for statistics
        result.path.add_module('VariablesToHistogram', fileName='listCounts.root',
                               variables=[('countInList(e+:1)', 1000, -0.5, 999.5),
                                          ('countInList(mu+:2)', 1000, -0.5, 999.5),
                                          ('countInList(e+:1, isSignal == 1)', 1000, -0.5, 999.5),
                                          ('countInList(mu+:2, isSignal2 == 1)', 1000, -0.5, 999.5),
                                          ('countInList(e+:1, isSignal == 0)', 1000, -0.5, 999.5),
                                          ('countInList(mu+:2, isSignal2 == 0)', 1000, -0.5, 999.5)])
        self.assertEqual(self.resource, result)

    def test_CountParticlesListsWithFile(self):
        with temporary_file('listCounts.root'):
            # Returns counts if file already exists
            self.assertEqual(CountParticleLists(self.resource, ['isSignal', 'isSignal2'], ['e+', 'mu+']), "listCounts.root")
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)


if __name__ == '__main__':
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
