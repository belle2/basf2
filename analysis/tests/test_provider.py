#!/usr/bin/env python
# -*- coding: utf-8 -*-

from fei.provider import *

from basf2 import *
import unittest
import os
import tempfile
import atexit
import shutil
import contextlib
import IPython
import inspect
import ast
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


class TestSelectParticleList(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource(env={'ROE': False})

    def test_SelectParticleList(self):
        # Returns name of ParticleList
        self.assertEqual(SelectParticleList(self.resource, 'e+', 'generic',
                                            PreCutConfiguration(None, None, None, None, 'eid > 0.2')), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': False}, cache=True)
        # Adds ParticleLoader
        result.path.add_module('ParticleLoader', decayStringsWithCuts=[('e+:42', 'eid > 0.2')], writeOut=True)
        self.assertEqual(self.resource, result)

    def test_SelectParticleListWithoutCut(self):
        # Returns name of ParticleList
        self.assertEqual(SelectParticleList(self.resource, 'e+', 'generic', None), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': False}, cache=True)
        # Adds ParticleLoader
        result.path.add_module('ParticleLoader', decayStringsWithCuts=[('e+:42', '')], writeOut=True)
        self.assertEqual(self.resource, result)

    def test_SelectParticleListInROE(self):
        # Set environment to ROE
        self.resource.env['ROE'] = True
        # Returns name of ParticleList
        self.assertEqual(SelectParticleList(self.resource, 'e+', 'generic',
                                            PreCutConfiguration(None, None, None, None, 'eid > 0.2')), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': True}, cache=True)
        # Adds ParticleLoader with cut
        result.path.add_module('ParticleLoader',
                               decayStringsWithCuts=[('e+:42', '[eid > 0.2] and isInRestOfEvent > 0.5')], writeOut=True)
        self.assertEqual(self.resource, result)

    def test_SelectParticleListInROEWithoutCut(self):
        # Set environment to ROE
        self.resource.env['ROE'] = True
        # Returns name of ParticleList
        self.assertEqual(SelectParticleList(self.resource, 'e+', 'generic', None), 'e+:42')
        # Enables caching
        result = MockResource(env={'ROE': True}, cache=True)
        # Adds ParticleLoader with cut
        result.path.add_module('ParticleLoader',
                               decayStringsWithCuts=[('e+:42', 'isInRestOfEvent > 0.5')], writeOut=True)
        self.assertEqual(self.resource, result)


class TestMatchParticleList(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_MatchParticleList(self):
        # Returns name of ParticleList
        self.assertEqual(MatchParticleList(self.resource, 'e+:generic'), 'e+:generic')
        # Enables caching and MC-data condition
        result = MockResource(cache=True, condition=('EventType', '==0'))
        # Adds MCMatching for given particle List
        result.path.add_module('MCMatching', listName='e+:generic')
        self.assertEqual(self.resource, result)

    def test_MatchParticleListWithNone(self):
        # Returns None if given ParticleList is None
        self.assertEqual(MatchParticleList(self.resource, None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestMakeParticleList(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.preCut = {'cutstring': '1.5 < M < 2.0'}

    def test_MakeParticleList(self):
        # Returns name of ParticleList
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], self.preCut, 23), 'D0:42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleCombiner for given decay
        result.path.add_module('ParticleCombiner', decayString='D0:42 ==> K- pi+', writeOut=True,
                               decayMode=23, cut=self.preCut['cutstring'])
        self.assertEqual(self.resource, result)

    def test_MakeParticleListWithNone(self):
        # Returns None if given PreCut is None
        self.assertEqual(MakeParticleList(self.resource, 'D0', ['K-', 'pi+'], None, 23), None)
        # Enables caching
        result = MockResource(cache=True)
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

    def test_CopyParticleListsWithoutPostCut(self):
        # Returns name of ParticleList
        self.assertEqual(CopyParticleLists(self.resource, 'D0', 'generic', ['D0:1', 'D0:2', 'D0:3'],
                                           None, ['S1', 'S2', 'S3']), 'D0:42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleListManipulator for given decay
        result.path.add_module('ParticleListManipulator', outputListName='D0:42', inputListNames=['D0:1', 'D0:2', 'D0:3'],
                               writeOut=True, cut='')
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
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi-', 'D0:generic -> K+ pi-'), '42')
        # Enables caching
        result = MockResource(cache=True)
        # Adds ParticleVertexFitter for given ParticleList
        result.path.add_module('ParticleVertexFitter', listName='D0:generic -> K+ pi-', confidenceLevel=-2,
                               vertexFitter='kfitter', fitType='vertex')
        self.assertEqual(self.resource, result)

    def test_FitVertexWithNone(self):
        # Returns None if ParticleList is None
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi-', None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_FitVertexWithTwoPi0(self):
        # Returns None if ParticleList contains pi0 pi0
        self.assertEqual(FitVertex(self.resource, 'D0 -> K+ pi- pi0 pi0', 'D0:generic -> K+ pi-'), None)
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
        self.preCutConfig = PreCutConfiguration('M', (500, 2, 4), 0.7, 0.01, 'M > 0.2')

    def test_CreatePreCutHistogram(self):
        # Returns None if Histogram does not exists
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds PreCutHistMaker for given ParticleList
        result.path.add_module('PreCutHistMaker', fileName='CutHistograms_UniqueChannelName:42.root',
                               decayString='D+:42 ==> pi+:1 K+:1', cut=self.preCutConfig.userCut,
                               target='isSignal', variable=self.preCutConfig.variable,
                               histParams=self.preCutConfig.binning)
        self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramCustomBinning(self):
        # Set Custom binning
        self.preCutConfig = PreCutConfiguration('M', list(range(10)), 0.7, 0.01, 'M > 0.2')
        # Returns None if Histogram does not exists
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds PreCutHistMaker for given ParticleList
        result.path.add_module('PreCutHistMaker', fileName='CutHistograms_UniqueChannelName:42.root',
                               decayString='D+:42 ==> pi+:1 K+:1', cut=self.preCutConfig.userCut,
                               target='isSignal', variable=self.preCutConfig.variable,
                               customBinning=self.preCutConfig.binning)
        self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramWithFile(self):
        with temporary_file('CutHistograms_UniqueChannelName:42.root'):
            # Returns Tuple if Histogram does exists
            self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                                   self.preCutConfig, ['pi+:1', 'K+:1'], ['bar', 'foo']),
                             ('CutHistograms_UniqueChannelName:42.root', 'D+:42'))
            # Enables caching, halt and condition
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramMissingDaughter(self):
        # Returns None if daughter particle is missing
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, [None, 'K+:1'], ['bar', 'foo']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_CreatePreCutHistogramMissingAdditionalDependency(self):
        # Returns None if additional dependency is missing
        self.assertEqual(CreatePreCutHistogram(self.resource, 'D+', 'UniqueChannelName', 'isSignal',
                                               self.preCutConfig, ['pi+:1', 'K+:1'], [None, 'foo']), None)
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
    assert preCutConfig == PreCutConfiguration('M', (500, 2, 4), 0.7, 0.01, 'p > 0.2')
    assert channelNames == ('D0:1 ==> K+ pi-', 'D0:2 ==> K+ pi- pi0', 'D0:3 ==> K+ pi+ pi- pi-')
    assert preCutHistograms == ('Dummy1', 'Dummy2', 'Dummy3')
    return {'D0:1 ==> K+ pi-': {'cutstring': '0.1 < M < 0.2', 'isIgnored': False},
            'D0:2 ==> K+ pi- pi0': {'cutstring': '0.15 < M < 0.18', 'isIgnored': False},
            'D0:3 ==> K+ pi+ pi- pi-': {'cutstring': '', 'isIgnored': True}}


class TestPreCutDetermination(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.preCutConfig = PreCutConfiguration('M', (500, 2, 4), 0.7, 0.01, 'p > 0.2')
        self.channelNames = ['D0:1 ==> K+ pi-', 'D0:2 ==> K+ pi- pi0', 'D0:3 ==> K+ pi+ pi- pi-', 'D0:4 ==> K+ K-']
        self.preCutHistograms = ['Dummy1', 'Dummy2', 'Dummy3', None]

    def test_PreCutDetermination(self):
        from fei import preCutDetermination
        preCutDetermination.CalculatePreCuts = MockCalculatePreCuts
        # Returns value (preCut) stored in given dictionary (preCuts for all channels) under key (channelName)
        self.assertDictEqual(PreCutDetermination(self.resource, self.channelNames, self.preCutConfig, self.preCutHistograms),
                             {'D0:1 ==> K+ pi-': {'cutstring': '0.1 < M < 0.2 and [p > 0.2]', 'isIgnored': False},
                              'D0:2 ==> K+ pi- pi0': {'cutstring': '0.15 < M < 0.18 and [p > 0.2]', 'isIgnored': False},
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
        self.postCutConfig = PostCutConfiguration(0.123)

    def test_PostCutDetermination(self):
        # Returns PostCut dictionary with cutstring and range
        self.assertDictEqual(PostCutDetermination(self.resource, self.postCutConfig),
                             {'cutstring': '0.123 < extraInfo(SignalProbability)', 'range': (0.123, 1)})
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_PostCutDeterminationWithNone(self):
        # Returns None if given PostCutConfig is None
        self.assertEqual(PostCutDetermination(self.resource, None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class MockTFile(object):

    def __init__(self, filename):
        assert filename == 'e+:generic_42.root'

    def Get(self, branch):
        assert branch == 'distribution'
        return self

    def GetEntries(self, selection):
        assert selection[:-1] == 'isSignal == '
        if selection[-1] == '0':
            return 23
        if selection[-1] == '1':
            return 42
        return 0


# If you're afraid of black magic, ignore the following lines
class InjectMockTFile(ast.NodeTransformer):

    def visit_Attribute(self, node):
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'ROOT':
            return ast.copy_location(ast.Name(id='MockTFile', ctx=node.ctx), node)
        return node


class TestFSPDistribution(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_FSPDistribution(self):
        # Returns None if Histogram does not exists
        self.assertEqual(FSPDistribution(self.resource, 'e+:generic', 'isSignal'), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds VariablesToNtuple for given ParticleList
        result.path.add_module('VariablesToNtuple', particleList='e+:generic', variables=['isSignal'],
                               fileName='e+:generic_42.root', treeName='distribution')
        self.assertEqual(self.resource, result)

    def test_FSPDistributionWithFile(self):
        # I call upon the mighty god of Python!
        exec compile(InjectMockTFile().visit(ast.parse(inspect.getsource(FSPDistribution))), '<string>', 'exec')
        with temporary_file('e+:generic_42.root'):
            # Returns dictionary with nSignal and nBackground if Histogram does exists
            self.assertDictEqual(FSPDistribution(self.resource, 'e+:generic', 'isSignal'),
                                 {'nSignal': 42, 'nBackground': 23})
            # Enables caching, halt and condition
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


class MockTFileForSPlotModel(object):
    def __init__(self, filename, recreate):
        assert recreate == "RECREATE"
        assert filename == "model_Name_42.root"

    def ls(self):
        pass

    def Close(self):
        pass


# If you're afraid of black magic, ignore the following lines
class InjectMockTFileForSPlotModel(ast.NodeTransformer):

    def visit_Attribute(self, node):
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'ROOT':
            return ast.copy_location(ast.Name(id='MockTFileForSPlotModel', ctx=node.ctx), node)
        return node


class TestGenerateSPlotModel(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        self.distribution = {'nSignal': 10, 'nBackground': 30, 'variable': 'M',
                             'signalPeak': 1.8, 'signalWidth': 0.1, 'range': (1.4, 1.9)}

    def test_GenerateSPlotModel(self):
        exec compile(InjectMockTFileForSPlotModel().visit(ast.parse(inspect.getsource(GenerateSPlotModel))), '<string>', 'exec')
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


def MockSubprocessFailCall(command, shell):
    assert shell
    expectation = "externTeacher --methodName 'FastBDT' --methodType 'Plugin'"\
                  " --methodConfig 'CreateMVAPdfs:Nbins=100:TMVAConfigString' --target 'isSignal'"\
                  " --variables 'p' 'pt' --prefix 'trainingData'"\
                  " > 'trainingData'.log 2>&1"
    assert command == expectation, command

# If you're afraid of black magic, ignore the following lines


class InjectMockSubprocessFail(ast.NodeTransformer):

    def visit_Attribute(self, node):
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'subprocess':
            return ast.copy_location(ast.Name(id='MockSubprocessFailCall', ctx=node.ctx), node)
        return node


def MockSubprocessSuccessCall(command, shell):
    assert shell
    expectation = "externTeacher --methodName 'FastBDT' --methodType 'Plugin'"\
                  " --methodConfig 'CreateMVAPdfs:Nbins=100:TMVAConfigString' --target 'isSignal'"\
                  " --variables 'p' 'pt' --prefix 'trainingData'"\
                  " > 'trainingData'.log 2>&1"
    assert command == expectation, command
    open('trainingData_1.config', 'a').close()


# If you're afraid of black magic, ignore the following lines
class InjectMockSubprocessSuccess(ast.NodeTransformer):

    def visit_Attribute(self, node):
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'subprocess':
            return ast.copy_location(ast.Name(id='MockSubprocessSuccessCall', ctx=node.ctx), node)
        return node


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
        # I call upon the mighty god of Python!
        exec compile(InjectMockSubprocessFail().visit(ast.parse(inspect.getsource(TrainMultivariateClassifier))),
                     '<string>', 'exec')
        # Returns None if training was did not create correct file
        with self.assertRaises(RuntimeError):
            TrainMultivariateClassifier(self.resource, self.mvaConfig, 'CreateMVAPdfs:Nbins=100:',
                                        'trainingData.root')

    def test_TrainMultivariateClassifierTrainingSuccessfull(self):
        # I call upon the mighty god of Python!
        exec compile(InjectMockSubprocessSuccess().visit(ast.parse(inspect.getsource(TrainMultivariateClassifier))),
                     '<string>', 'exec')
        # Returns config filename if training was did not create correct file
        self.assertEqual(TrainMultivariateClassifier(self.resource, self.mvaConfig, 'CreateMVAPdfs:Nbins=100:',
                                                     'trainingData.root'), 'trainingData_1.config')
        # Enables caching
        result = MockResource(cache=True, usesMultiThreading=True, env={'externTeacher': 'externTeacher'})
        self.assertEqual(self.resource, result)
        os.remove('trainingData_1.config')


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
            self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', 'signalProbability', 'isSignal', None), 'var_D0:1_42.root')
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)

    def test_VariablesToNTupleMissingParticleList(self):
        # Returns None if particle list is None
        self.assertEqual(VariablesToNTuple(self.resource, None, 'signalProbability', 'isSignal', None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_VariablesToNTupleMissingSignalProbability(self):
        # Returns None if SignalProbability is None
        self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', None, 'isSignal', None), None)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)

    def test_VariablesToNTuple(self):
        # Returns None if file does not exists
        self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', 'signalProbability', 'isSignal', None), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds VariablesToNtuple for given ParticleList
        variables = ['isSignal', 'extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'cosThetaBetweenParticleAndTrueB']
        result.path.add_module('VariablesToNtuple', particleList='D0:1', fileName='var_D0:1_42.root',
                               treeName='variables', variables=variables)
        self.assertEqual(self.resource, result)

    def test_VariablesToNTupleWithExtraVars(self):
        # Returns None if file does not exists
        self.assertEqual(VariablesToNTuple(self.resource, 'D0:1', 'signalProbability', 'isSignal', ['a', 'b']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Adds VariablesToNtuple for given ParticleList
        variables = ['isSignal', 'extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'cosThetaBetweenParticleAndTrueB']
        result.path.add_module('VariablesToNtuple', particleList='D0:1', fileName='var_D0:1_42.root',
                               treeName='variables', variables=variables + ['a', 'b'])
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


# If you're afraid of black magic, ignore the following lines
class InjectMockTFileAndTH1D(ast.NodeTransformer):

    def visit_Attribute(self, node):
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'ROOT':
            return ast.copy_location(ast.Name(id='MockTFileAndTH1D', ctx=node.ctx), node)
        return node


class TestCountMCParticles(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()

    def test_CountMCParticles(self):
        # Returns None if file does not exists
        self.assertEqual(CountMCParticles(self.resource, ['e+', 'mu+']), None)
        # Enables caching, halt and condition
        result = MockResource(cache=True, halt=True, condition=('EventType', '==0'))
        # Add VariablestoNtuple module for statistics
        result.path.add_module('VariablesToNtuple', fileName='mcParticlesCount.root', treeName='mccounts',
                               variables=['NumberOfMCParticlesInEvent(11)', 'NumberOfMCParticlesInEvent(13)'])
        self.assertEqual(self.resource, result)

    def test_CountMCParticlesWithFile(self):
        # I call upon the mighty god of Python!
        exec compile(InjectMockTFileAndTH1D().visit(ast.parse(inspect.getsource(CountMCParticles))), '<string>', 'exec')
        with temporary_file('mcParticlesCount.root'):
            # Returns counts if file already exists
            self.assertDictEqual(CountMCParticles(self.resource, ['e+', 'mu+']), {'11': 42, '13': 23, 'NEvents': 65})
            # Enables caching
            result = MockResource(cache=True)
            self.assertEqual(self.resource, result)


args_list = []
result_list = []


def MockAutomaticReporting(*args):
    global args_list
    global result_list
    x = args_list.pop()
    assert all(a == b for a, b in zip(args, x)), str(list(zip(args, x)))
    return result_list.pop()


class TestWriteAnalysisFileForChannel(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        global args_list
        args_list = []
        global result_list
        result_list = []

    def test_WriteAnalysisFileForChannel(self):
        global args_list
        global result_list
        from fei import automaticReporting
        automaticReporting.createPreCutTexFile = MockAutomaticReporting
        automaticReporting.createMVATexFile = MockAutomaticReporting
        automaticReporting.createTexFile = MockAutomaticReporting

        placeholders = {'particleName': 'D0', 'particleLabel': 'generic', 'channelName': 'D0 -> K+ pi-',
                        'isIgnored': False, 'mvaConfigObject': 'mvaConfig',
                        'texFile': 'D0_D0 -> K+ pi-_42.tex',
                        'mvaSPlotTexFile': 'empty.tex',
                        'mvaTexFile': 'empty.tex'}
        args_list.append([placeholders['texFile'], 'analysis/scripts/fei/templates/ChannelTemplate.tex', placeholders])
        args_list.append([placeholders, 'mvaConfig', 'tmvaTraining', 'postCutConfig', 'postCut'])
        args_list.append([placeholders, 'mvaConfig', 'splotTraining', 'postCutConfig', 'postCut'])
        args_list.append([placeholders, 'preCutHistogram', 'preCutConfig', 'preCut'])
        result_list.append(placeholders)
        result_list.append(placeholders)
        result_list.append(placeholders)
        result_list.append(placeholders)
        # Returns dictionary with placeholders
        self.assertDictEqual(WriteAnalysisFileForChannel(self.resource, 'D0', 'generic', 'D0 -> K+ pi-',
                                                         'preCutConfig', 'preCut', 'preCutHistogram', 'mvaConfig',
                                                         'tmvaTraining', 'splotTraining', 'postCutConfig', 'postCut'), placeholders)
        self.assertEqual(len(args_list), 0)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestWriteAnalysisFileForFSParticle(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        global args_list
        args_list = []
        global result_list
        result_list = []

    def test_WriteAnalysisFileForFSParticle(self):
        global args_list
        global result_list
        from fei import automaticReporting
        automaticReporting.createMVATexFile = MockAutomaticReporting
        automaticReporting.createFSParticleTexFile = MockAutomaticReporting

        placeholders = {'particleName': 'D0', 'particleLabel': 'generic', 'isIgnored': False}
        args_list.append([placeholders, 'nTuple', 'mccounts', 'distribution', 'mvaConfig'])
        args_list.append([placeholders, 'mvaConfig', 'tmvaTraining', 'postCutConfig', 'postCut'])
        result_list.append(placeholders)
        result_list.append(placeholders)
        # Returns dictionary with placeholders
        self.assertDictEqual(WriteAnalysisFileForFSParticle(self.resource, 'D0', 'generic', 'mvaConfig',
                                                            'tmvaTraining', 'postCutConfig', 'postCut',
                                                            'distribution', 'nTuple', 'mccounts'), placeholders)
        self.assertEqual(len(args_list), 0)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestWriteAnalysisFileForCombinedParticle(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        global args_list
        args_list = []
        global result_list
        result_list = []

    def test_WriteAnalysisFileForCombinedParticle(self):
        global args_list
        global result_list
        from fei import automaticReporting
        automaticReporting.createCombinedParticleTexFile = MockAutomaticReporting

        channelPlaceholders = [{'mvaConfigObject': 'mvaConfig'}]
        placeholders = {'channels': channelPlaceholders, 'particleName': 'D0', 'particleLabel': 'generic', 'isIgnored': False}
        args_list.append([placeholders, channelPlaceholders, 'nTuple', 'mccounts', 'mvaConfig'])
        result_list.append(placeholders)
        # Returns dictionary with placeholders
        self.assertDictEqual(WriteAnalysisFileForCombinedParticle(self.resource, 'D0', 'generic', channelPlaceholders,
                                                                  'nTuple', 'mccounts'), placeholders)
        self.assertEqual(len(args_list), 0)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)


class TestWriteCPUTimeSummary(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        global args_list
        args_list = []
        global result_list
        result_list = []

    def test_WriteCPUTimeSummary(self):
        global args_list
        global result_list
        from fei import automaticReporting
        automaticReporting.getModuleStatsFromFile = MockAutomaticReporting
        automaticReporting.createCPUTimeTexFile = MockAutomaticReporting

        placeholders = 'Placeholders'
        args_list.append(['ChannelNames', 'InputLists', 'channelPlaceholders', 'mccounts', 'moduleStatisticsFile', 'stats'])
        args_list.append(['moduleStatisticsFile'])
        result_list.append(placeholders)
        result_list.append('stats')
        # Returns dictionary with placeholders
        self.assertEqual(WriteCPUTimeSummary(self.resource, 'ChannelNames', 'InputLists', 'channelPlaceholders',
                                             'mccounts', 'moduleStatisticsFile'), placeholders)
        self.assertEqual(len(args_list), 0)
        # Enables caching
        result = MockResource()
        self.assertEqual(self.resource, result)


subprocess_call = 0


def MockSummarySubprocessCall(command, shell=False):
    global subprocess_call
    if subprocess_call == 0:
        assert command == 'cp ' + ROOT.Belle2.FileSystem.findFile('analysis/scripts/fei/templates/nordbert.pdf') + ' .'
        assert shell
    elif subprocess_call == 1:
        assert command == 'cp ' + ROOT.Belle2.FileSystem.findFile('analysis/scripts/fei/templates/empty.tex') + ' .'
        assert shell
    elif subprocess_call > 1:
        assert command == ['pdflatex', '-halt-on-error', '-interaction=nonstopmode', 'myTexFile.tex']
        assert shell is False
    subprocess_call += 1
    return 0


class MockSummaryTFile(object):

    def __init__(self, ntuple):
        assert ntuple == "semileptonic.root" or ntuple == "hadronic.root"

    def Get(self, branch):
        assert branch == "variables"
        return self

    def GetEntries(self, selection):
        assert selection == "isSignal && extraInfouniqueSignal" or\
            selection == "isSignalAcceptMissingNeutrino && extraInfouniqueSignal"
        return 42


# If you're afraid of black magic, ignore the following lines
class InjectMockSummary(ast.NodeTransformer):

    def visit_Attribute(self, node):
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'subprocess':
            return ast.copy_location(ast.Name(id='MockSummarySubprocessCall', ctx=node.ctx), node)
        if hasattr(
                node,
                'value') and hasattr(
                node.value,
                'id') and node.value.id == 'ROOT' and not hasattr(
                self,
                'stop_fixing_ROOT'):
            self.stop_fixing_ROOT = True
            return ast.copy_location(ast.Name(id='MockSummaryTFile', ctx=node.ctx), node)
        if hasattr(node, 'value') and hasattr(node.value, 'id') and node.value.id == 'automaticReporting':
            return ast.copy_location(ast.Name(id='MockAutomaticReporting', ctx=node.ctx), node)
        return node


class TestWriteAnalysisFileSummary(unittest.TestCase):

    def setUp(self):
        self.resource = MockResource()
        global args_list
        args_list = []
        global result_list
        result_list = []
        if os.path.isfile("sent_mail"):
            os.remove("sent_mail")

    def test_WriteAnalysisFileSummary(self):
        global args_list
        global result_list
        exec compile(InjectMockSummary().visit(ast.parse(inspect.getsource(WriteAnalysisFileSummary))), '<string>', 'exec')
        placeholders = {'texFile': "myTexFile.tex"}
        finalStateParticlePlaceholders = 'finalStateParticlePlaceholders'
        combinedParticlePlaceholders = [{'particleName': 'B+:semileptonic'}, {'particleName': 'B0:hadronic'}]

        finalParticleNTuples = ['semileptonic.root', None, 'hadronic.root']
        finalParticleTargets = ['isSignalAcceptMissingNeutrino', 'isSignal', 'isSignal']

        filled_finalParticlePlaceholders = [{'particleName': 'B+:semileptonic'}, {'particleName': 'B+:semileptonic'},
                                            {'particleName': 'B0:hadronic'}, {'particleName': 'B0:hadronic'}]
        args_list.append([finalStateParticlePlaceholders, combinedParticlePlaceholders, filled_finalParticlePlaceholders,
                          'cpuTimeSummaryPlaceholders', 'mcCounts', 'particles'])
        args_list.append(['hadronic.root', 'ROC', 'mcCounts', 'isSignal'])
        args_list.append(['hadronic.root', 'Mbc', 'mcCounts', 'isSignal'])
        args_list.append(['semileptonic.root', 'ROC', 'mcCounts', 'isSignalAcceptMissingNeutrino'])
        args_list.append(['semileptonic.root', 'CosBDL', 'mcCounts', 'isSignalAcceptMissingNeutrino'])
        result_list.append(placeholders)
        result_list.append({'particleName': 'B0:hadronic'})
        result_list.append({'particleName': 'B0:hadronic'})
        result_list.append({'particleName': 'B+:semileptonic'})
        result_list.append({'particleName': 'B+:semileptonic'})
        # Returns always None
        self.assertEqual(WriteAnalysisFileSummary(self.resource, finalStateParticlePlaceholders,
                                                  combinedParticlePlaceholders,
                                                  finalParticleNTuples, finalParticleTargets,
                                                  'cpuTimeSummaryPlaceholders',
                                                  'mcCounts',
                                                  'particles'), None)
        self.assertEqual(len(args_list), 0)
        self.assertEqual(combinedParticlePlaceholders[0]['particleNUniqueSignalAfterPostCut'], 42)
        self.assertEqual(combinedParticlePlaceholders[1]['particleNUniqueSignalAfterPostCut'], 42)
        # Enables caching
        result = MockResource(cache=True)
        self.assertEqual(self.resource, result)
        # Check if sent mail lock is created and remove it afterwards
        self.assertTrue(os.path.isfile("sent_mail"))
        os.remove("sent_mail")


if __name__ == '__main__':
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
