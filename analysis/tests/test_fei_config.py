#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tempfile
import os
import shutil
import atexit
import unittest

import fei.config

# @cond


class TestFeiConfiguration(unittest.TestCase):

    def test_Defaults(self):
        config = fei.config.FeiConfiguration()
        self.assertEqual(config.prefix, 'FEI_TEST')
        self.assertEqual(config.cache, None)
        self.assertEqual(config.monitor, True)
        self.assertEqual(config.legacy, None)
        self.assertEqual(config.externTeacher, 'basf2_mva_teacher')
        self.assertEqual(config.training, False)


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
        channel = fei.config.DecayChannel(name='D0:23', label='D0 ==> K- pi+',
                                          decayString='D0:23 -> K- pi+', daughters=['K-', 'pi+'],
                                          mvaConfig=fei.config.MVAConfiguration(variables=['E']),
                                          preCutConfig=fei.config.PreCutConfiguration(userCut='test'),
                                          decayModeID=23)
        self.assertEqual(channel.name, 'D0:23')
        self.assertEqual(channel.label, 'D0 ==> K- pi+')
        self.assertEqual(channel.decayString, 'D0:23 -> K- pi+')
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
        channels.append(fei.config.DecayChannel(name='D0:generic_0',
                                                label='D0:generic ==> pi-:generic pi+:generic',
                                                decayString='D0:generic_0 -> pi-:generic pi+:generic',
                                                daughters=['pi-:generic', 'pi+:generic'],
                                                mvaConfig=fei.config.MVAConfiguration(variables=['E',
                                                                                                 'daughter(0, p)',
                                                                                                 'daughter(1, p)']),
                                                preCutConfig=fei.config.PreCutConfiguration(userCut='test'),
                                                decayModeID=0))
        channels.append(fei.config.DecayChannel(name='D0:generic_1',
                                                label='D0:generic ==> K-:generic pi+:generic',
                                                decayString='D0:generic_1 -> K-:generic pi+:generic',
                                                daughters=['K-:generic', 'pi+:generic'],
                                                mvaConfig=fei.config.MVAConfiguration(variables=['E',
                                                                                                 'daughter(0, p)',
                                                                                                 'daughter(1, p)',
                                                                                                 'daughterAngle(0, 1)']),
                                                preCutConfig=fei.config.PreCutConfiguration(userCut='alternative'),
                                                decayModeID=1))
        channels.append(fei.config.DecayChannel(name='D0:generic_2',
                                                label='D0:generic ==> K-:specific pi+:generic pi0:generic',
                                                decayString='D0:generic_2 -> K-:specific pi+:generic pi0:generic',
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
        channels.append(fei.config.DecayChannel(name='D0:generic_3',
                                                label='D0:generic ==> K-:generic K+:generic',
                                                decayString='D0:generic_3 -> K-:generic K+:generic',
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


if __name__ == '__main__':
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
