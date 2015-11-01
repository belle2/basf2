#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import os
import tempfile
import atexit
import shutil
import contextlib
import IPython

# Deactivate this unittests for now
import sys
sys.exit(0)

import numpy
import pandas

from fei import detailedReporting
from ROOT import Belle2
# @cond


class TestLoadPreCutDictionary(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/precuthist.root')

    def test_loadPreCutDictionary(self):
        df = detailedReporting.loadPreCutDictionary(self.filename)

        self.assertEqual(len(df), 5)
        self.assertListEqual(sorted(df.keys()), sorted(['Signal', 'WithoutCut', 'Background', 'All', 'Ratio']))

        # Numbers were tested TBrowser
        self.assertEqual(len(df['WithoutCut'].array), 4)
        self.assertEqual(df['WithoutCut'].array[1], 170312934.0)
        self.assertEqual(df['WithoutCut'].array[2], 4262288.0)
        self.assertEqual(df['WithoutCut'].count(), 174575222)
        self.assertEqual(df['WithoutCut'].min(), 0.0)
        self.assertEqual(df['WithoutCut'].max(), 1.0)


class TestLoadMCCountsDictionary(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/mcParticlesCountHists.root')

    def test_loadMCCountsDictionary(self):
        df = detailedReporting.loadMCCountsDictionary(self.filename)

        self.assertEqual(len(df), 7)
        self.assertListEqual(sorted(df.keys()), sorted(['211', '321', '22', '111', '421', '411', '413']))

        # Numbers were tested using IPython
        # f = ROOT.TFile("analysis/tests/testfiles_detailedReporting/mcParticlesCountHists.root")
        # for i in f.GetListOfKeys():
        #     hist = f.Get(i.GetName())
        #     print i.GetName(), hist.GetMean()*hist.Integral()
        self.assertEqual(df['211'].sum(), 7962435.0)
        self.assertEqual(df['321'].sum(), 1653494.0)
        self.assertEqual(df['22'].sum(), 11256323.0)
        self.assertEqual(df['111'].sum(), 4806187.0)
        self.assertEqual(df['421'].sum(), 1799020.0)
        self.assertEqual(df['411'].sum(), 261671.0)
        self.assertEqual(df['413'].sum(), 246457.0)


class TestLoadListCountsDictionary(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/listCountsHists.root')
        self.lists = ['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467',
                      'K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99',
                      'gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24',
                      'pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9',
                      'D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3',
                      'D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3',
                      'D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf',
                      'D0:c756a1fdbbe75673bf78b71a90e726a11202a50e',
                      'D+:5144c31f84e6c83c673259778908aad82b62844b',
                      'D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35']
        self.keys = [l + '_All' for l in self.lists] + [l + '_Signal' for l in self.lists] + [l + '_Background' for l in self.lists]

    def test_loadListCountsDictionary(self):
        df = detailedReporting.loadListCountsDictionary(self.filename)

        self.assertEqual(len(df), len(self.keys))
        self.assertListEqual(sorted(df.keys()), sorted(self.keys))

        # Numbers were tested using IPython
        # f = ROOT.TFile("analysis/tests/testfiles_detailedReporting/listCountsHists.root")
        # for i in f.GetListOfKeys():
        #     hist = f.Get(i.GetName())
        #     print i.GetName(), hist.GetMean()*hist.Integral()
        self.assertEqual(df['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467_All'].sum(), 9727281.0)
        self.assertEqual(df['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467_Signal'].sum(), 6679404.0)
        self.assertEqual(df['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467_Background'].sum(), 3047877.0)
        self.assertEqual(df['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99_All'].sum(), 9727281.0)
        self.assertEqual(df['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99_Signal'].sum(), 1336094.0)
        self.assertEqual(df['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99_Background'].sum(), 8391187.0)
        self.assertEqual(df['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24_All'].sum(), 18018502.0)
        self.assertEqual(df['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24_Signal'].sum(), 10073484.0)
        self.assertEqual(df['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24_Background'].sum(), 7945018.0)
        self.assertEqual(df['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9_All'].sum(), 54048503.0)
        self.assertEqual(df['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9_Signal'].sum(), 3291243.0)
        self.assertEqual(df['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9_Background'].sum(), 51519938.0)
        self.assertEqual(df['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3_All'].sum(), 186726.0)
        self.assertEqual(df['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3_Signal'].sum(), 44664.0)
        self.assertEqual(df['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3_Background'].sum(), 142062.0)
        self.assertEqual(df['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3_All'].sum(), 1973959.0)
        self.assertEqual(df['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3_Signal'].sum(), 49857.0)
        self.assertEqual(df['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3_Background'].sum(), 1924119.0)
        self.assertEqual(df['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf_All'].sum(), 26093.0)
        self.assertEqual(df['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf_Signal'].sum(), 1533.0)
        self.assertEqual(df['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf_Background'].sum(), 24560.0)
        self.assertEqual(df['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e_All'].sum(), 22832.0)
        self.assertEqual(df['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e_Signal'].sum(), 4028.0)
        self.assertEqual(df['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e_Background'].sum(), 18804.0)
        self.assertEqual(df['D+:5144c31f84e6c83c673259778908aad82b62844b_All'].sum(), 331357.0)
        self.assertEqual(df['D+:5144c31f84e6c83c673259778908aad82b62844b_Signal'].sum(), 10638.0)
        self.assertEqual(df['D+:5144c31f84e6c83c673259778908aad82b62844b_Background'].sum(), 320719.0)
        self.assertEqual(df['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35_All'].sum(), 101730.0)
        self.assertEqual(df['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35_Signal'].sum(), 3236.0)
        self.assertEqual(df['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35_Background'].sum(), 98494.0)


class TestLoadMCCountsDataFrame(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/mcParticlesCount.root')

    def test_loadMCCountsDataFrame(self):
        df = detailedReporting.loadMCCountsDataFrame(self.filename)

        self.assertEqual(len(df), 100)
        self.assertListEqual(sorted(df.columns), sorted(['211', '321', '22', '111', '421', '411', '413']))
        sdf = df.sum()
        # The following numbers were independently checked with:
        # root -l mcParticlesCount.root
        # TH1F h("h", "h", 100, 0, 100);
        # for(int i = 0; i < 7; ++i) {
        #   std::string test = mccounts->GetListOfBranches().At(i).GetName();
        #   cout << test << " = ";
        #   mccounts->Project("h", test.c_str(), test.c_str());
        #   cout << h.Integral() << endl;
        # }
        self.assertEqual(sdf['211'], 791.0)
        self.assertEqual(sdf['321'], 161.0)
        self.assertEqual(sdf['22'], 1134.0)
        self.assertEqual(sdf['111'], 479.0)
        self.assertEqual(sdf['421'], 181.0)
        self.assertEqual(sdf['411'], 26.0)
        self.assertEqual(sdf['413'], 25.0)


class TestLoadListCountsDataFrame(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/listCounts.root')
        self.lists = ['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467',
                      'K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99',
                      'gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24',
                      'pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9',
                      'D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3',
                      'D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3',
                      'D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf',
                      'D0:c756a1fdbbe75673bf78b71a90e726a11202a50e',
                      'D+:5144c31f84e6c83c673259778908aad82b62844b',
                      'D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35']
        self.keys = [l + '_All' for l in self.lists] + [l + '_Signal' for l in self.lists] + [l + '_Background' for l in self.lists]

    def test_loadListCountsDataFrame(self):
        df = detailedReporting.loadListCountsDataFrame(self.filename)

        self.assertEqual(len(df), 100)
        self.assertListEqual(sorted(df.columns), sorted(self.keys))
        sdf = df.sum()
        # The following numbers were independently checked with:
        # root -l listCounts.root
        # TH1F h("h", "h", 100, 0, 100);
        # for(int i = 0; i < 30; ++i) {
        #   std::string test = listcounts->GetListOfBranches().At(i).GetName();
        #   cout << test << " = ";
        #   listcounts->Project("h", test.c_str(), test.c_str());
        #   cout << h.Integral() << endl;
        # }
        self.assertEqual(sdf['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467_All'], 954.0)
        self.assertEqual(sdf['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467_Signal'], 670.0)
        self.assertEqual(sdf['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467_Background'], 284.0)
        self.assertEqual(sdf['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99_All'], 954.0)
        self.assertEqual(sdf['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99_Signal'], 130.0)
        self.assertEqual(sdf['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99_Background'], 824.0)
        self.assertEqual(sdf['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24_All'], 1830.0)
        self.assertEqual(sdf['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24_Signal'], 1002.0)
        self.assertEqual(sdf['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24_Background'], 828.0)
        self.assertEqual(sdf['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9_All'], 6009.0)
        self.assertEqual(sdf['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9_Signal'], 341.0)
        self.assertEqual(sdf['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9_Background'], 5668.0)
        self.assertEqual(sdf['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3_All'], 23.0)
        self.assertEqual(sdf['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3_Signal'], 4.0)
        self.assertEqual(sdf['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3_Background'], 19.0)
        self.assertEqual(sdf['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3_All'], 231.0)
        self.assertEqual(sdf['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3_Signal'], 3.0)
        self.assertEqual(sdf['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3_Background'], 228.0)
        self.assertEqual(sdf['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf_All'], 4.0)
        self.assertEqual(sdf['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf_Signal'], 1.0)
        self.assertEqual(sdf['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf_Background'], 3.0)
        self.assertEqual(sdf['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e_All'], 2.0)
        self.assertEqual(sdf['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e_Signal'], 0.0)
        self.assertEqual(sdf['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e_Background'], 2.0)
        self.assertEqual(sdf['D+:5144c31f84e6c83c673259778908aad82b62844b_All'], 45.0)
        self.assertEqual(sdf['D+:5144c31f84e6c83c673259778908aad82b62844b_Signal'], 0.0)
        self.assertEqual(sdf['D+:5144c31f84e6c83c673259778908aad82b62844b_Background'], 45.0)
        self.assertEqual(sdf['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35_All'], 5.0)
        self.assertEqual(sdf['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35_Signal'], 0.0)
        self.assertEqual(sdf['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35_Background'], 5.0)


class TestLoadModuleStatisticsDataFrame(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/moduleStatistics.root')
        self.lists = ['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467',
                      'K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99',
                      'gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24',
                      'pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9',
                      'D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3',
                      'D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3',
                      'D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf',
                      'D0:c756a1fdbbe75673bf78b71a90e726a11202a50e',
                      'D+:5144c31f84e6c83c673259778908aad82b62844b',
                      'D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35']

    def test_loadModuleStatisticsDataFrame(self):
        df = detailedReporting.loadModuleStatisticsDataFrame(self.filename)

        self.assertEqual(len(df), 141)
        self.assertListEqual(sorted(df.columns), sorted(['name', 'time', 'type', 'listname']))
        self.assertListEqual(sorted(df.type.unique()), sorted(['ParticleLoader', 'ParticleCombiner', 'ParticleVertexFitter',
                                                               'MCMatch', 'TMVAExpert', 'Other']))
        unique = df.listname.unique()
        for l in self.lists + ['Other']:
            self.assertTrue(l in unique)

        sdf = df.groupby(['type'])['time'].sum()
        self.assertAlmostEqual(sdf['ParticleLoader'], 348.587310, delta=0.001)
        self.assertAlmostEqual(sdf['ParticleCombiner'], 608.436537, delta=0.001)
        self.assertAlmostEqual(sdf['ParticleVertexFitter'], 9545.304479, delta=0.001)
        self.assertAlmostEqual(sdf['MCMatch'], 406.493269, delta=0.001)
        self.assertAlmostEqual(sdf['TMVAExpert'], 1036.668055, delta=0.001)
        self.assertAlmostEqual(sdf['Other'], 18281.310939, delta=0.001)

        sdf = df.groupby(['listname'])['time'].sum()
        self.assertAlmostEqual(sdf['pi+:9c603352f1c7f8cde7437866da4fc795b72c9467'], 275.136933, delta=0.001)
        self.assertAlmostEqual(sdf['K+:cbc5cbc816a18f1d5658701bc3322ff9c9791c99'], 318.912922, delta=0.001)
        self.assertAlmostEqual(sdf['gamma:baf4804bf3e3d401ed05cc469497b0fe83377d24'], 317.925720, delta=0.001)
        self.assertAlmostEqual(sdf['pi0:90786ffc748ea3b11ae9dab84cbe4fbeb49443d9'], 6667.722236, delta=0.001)
        self.assertAlmostEqual(sdf['D0:3a61b0b85d0bf1497c5bfed8ab2dc66c957dffc3'], 79.856503, delta=0.001)
        self.assertAlmostEqual(sdf['D0:aa6a0fae9aaf8cdcefb8e22ff96c4c8d62ecb6b3'], 3259.807874, delta=0.001)
        self.assertAlmostEqual(sdf['D0:90a21bf28c01481c3dd3d8847ebacff6ec7e8bbf'], 38.665225, delta=0.001)
        self.assertAlmostEqual(sdf['D0:c756a1fdbbe75673bf78b71a90e726a11202a50e'], 35.042606, delta=0.001)
        self.assertAlmostEqual(sdf['D+:5144c31f84e6c83c673259778908aad82b62844b'], 263.403086, delta=0.001)
        self.assertAlmostEqual(sdf['D*+:b30dfc431cd31b7ffed733dceb0f7f0cb5642d35'], 80.580009, delta=0.001)


class TestReadRootScaled(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/'
                                                   'var_K+:2ec3c4d182fc3c427642f9fc648355f410dd141d_'
                                                   'afacdbbcf37285eb16b17d43afb91f2d536ecfef.root')

    def test_read_root_scaled(self):
        df, scale = detailedReporting.read_root_scaled(self.filename, 'variables', limit_average=25)
        self.assertTrue(20.0 < len(df) < 30.0)
        self.assertTrue(3.8 < scale < 4.2)

        self.assertListEqual(sorted(df.columns), sorted(['isSignal', 'mcErrors', 'extraInfo__boSignalProbability__bc',
                                                         'Mbc', 'cosThetaBetweenParticleAndTrueB']))
        sdf = df.sum()
        self.assertTrue(50 < sdf['isSignal'] * scale < 100)
        self.assertTrue(sdf['cosThetaBetweenParticleAndTrueB'] * scale < -3000)


class TestLoadNTupleDataFrame(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/'
                                                   'var_K+:2ec3c4d182fc3c427642f9fc648355f410dd141d_'
                                                   'afacdbbcf37285eb16b17d43afb91f2d536ecfef.root')

    def test_loadNTupleDataFrame(self):
        df, scale = detailedReporting.loadNTupleDataFrame(self.filename)

        self.assertEqual(scale, 1.0)
        self.assertEqual(len(df), 100)
        self.assertListEqual(sorted(df.columns), sorted(['isSignal', 'mcErrors', 'extraInfo(SignalProbability)',
                                                         'Mbc', 'cosThetaBetweenParticleAndTrueB']))
        sdf = df.sum()
        # The following numbers were independently checked with:
        # root -l mcParticlesCount.root
        # TH1F h("h", "h", 100, -1000000, 10000000);
        # for(int i = 0; i < 7; ++i) {
        #   std::string test = variables->GetListOfBranches().At(i).GetName();
        #   cout << test << " = ";
        #   variables->Project("h", test.c_str(), test.c_str());
        #   cout << h.Integral() << endl;
        # }
        # And afterwards added some more digits given by pandas
        self.assertEqual(sdf['isSignal'], 76.0)
        self.assertAlmostEqual(sdf['extraInfo(SignalProbability)'], 75.2462, delta=0.001)
        self.assertAlmostEqual(sdf['Mbc'], 521.6623, delta=0.001)
        self.assertEqual(sdf['mcErrors'], 2952)
        self.assertAlmostEqual(sdf['cosThetaBetweenParticleAndTrueB'], -6846.8998, delta=0.001)


class TestLoadTMVADataFrame(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/TMVATraining_1.root')

    def test_loadTMVADataFrame(self):
        df, train_scale, test_scale = detailedReporting.loadTMVADataFrame(self.filename)

        self.assertEqual(train_scale, 1.0)
        self.assertEqual(test_scale, 1.0)
        self.assertEqual(len(df), 200)
        self.assertTrue(numpy.all(df.iloc[:100]['__isTrain__']))
        self.assertFalse(numpy.any(df.iloc[100:]['__isTrain__']))
        self.assertListEqual(sorted(df.columns), sorted(['__isSignal__', '__isTrain__', 'className', 'classID', 'M', 'weight',
                                                         'isSignal', 'daughter(0,extraInfo(SignalProbability))', 'FastBDT',
                                                         'prob_FastBDT', 'daughter(1,extraInfo(SignalProbability))',
                                                         'daughterAngle(0,1)', 'Q']))
        sdf = df[df['__isTrain__']].sum()
        # The following numbers were independently checked with TBrowser
        self.assertAlmostEqual(sdf['classID'], 72.0, delta=0.01)
        self.assertAlmostEqual(sdf['M'], 12.35, delta=0.01)
        self.assertAlmostEqual(sdf['daughter(0,extraInfo(SignalProbability))'], 60.20, delta=0.01)
        self.assertAlmostEqual(sdf['daughter(1,extraInfo(SignalProbability))'], 57.73, delta=0.01)
        self.assertAlmostEqual(sdf['daughterAngle(0,1)'], 17.96, delta=0.01)
        self.assertAlmostEqual(sdf['Q'], 12.35, delta=0.01)
        self.assertAlmostEqual(sdf['isSignal'], 28.0, delta=0.01)
        self.assertAlmostEqual(sdf['__isSignal__'], 28.0, delta=0.01)
        self.assertAlmostEqual(sdf['__isTrain__'], 100.0, delta=0.01)
        self.assertAlmostEqual(sdf['weight'], 53.22, delta=0.01)
        self.assertAlmostEqual(sdf['FastBDT'], 39.91, delta=0.01)
        self.assertAlmostEqual(sdf['prob_FastBDT'], 39.41, delta=0.01)

        sdf = df[~df['__isTrain__']].sum()
        # The following numbers were independently checked with TBrowser
        self.assertAlmostEqual(sdf['classID'], 81.0, delta=0.01)
        self.assertAlmostEqual(sdf['M'], 12.49, delta=0.01)
        self.assertAlmostEqual(sdf['daughter(0,extraInfo(SignalProbability))'], 62.52, delta=0.01)
        self.assertAlmostEqual(sdf['daughter(1,extraInfo(SignalProbability))'], 55.93, delta=0.01)
        self.assertAlmostEqual(sdf['daughterAngle(0,1)'], -2.133, delta=0.01)
        self.assertAlmostEqual(sdf['Q'], 12.49, delta=0.01)
        self.assertAlmostEqual(sdf['isSignal'], 19.0, delta=0.01)
        self.assertAlmostEqual(sdf['__isSignal__'], 19.0, delta=0.01)
        self.assertAlmostEqual(sdf['__isTrain__'], 0.0, delta=0.01)
        self.assertAlmostEqual(sdf['weight'], 505.0, delta=0.01)
        self.assertAlmostEqual(sdf['FastBDT'], 39.10, delta=0.01)
        self.assertAlmostEqual(sdf['prob_FastBDT'], 38.40, delta=0.01)


class TestLoadMVARankingDataFrame(unittest.TestCase):

    def setUp(self):
        self.filename = Belle2.FileSystem.findFile('analysis/tests/testfiles_detailedReporting/TMVAlogfile.log')

    def test_loadMVARankingDataFrame(self):
        df = detailedReporting.loadMVARankingDataFrame(self.filename)

        self.assertEqual(len(df), 4)
        self.assertListEqual(sorted(df.columns), sorted(['name', 'importance']))

        self.assertEqual(df.iloc[0]['name'], 'daughter(0,extraInfo(SignalProbability))')
        self.assertAlmostEqual(df.iloc[0]['importance'], 6.51500, delta=0.01)
        self.assertEqual(df.iloc[1]['name'], 'Q')
        self.assertAlmostEqual(df.iloc[1]['importance'], 6.14100, delta=0.01)
        self.assertEqual(df.iloc[2]['name'], 'daughterAngle(0,1)')
        self.assertAlmostEqual(df.iloc[2]['importance'], 0.10970, delta=0.01)
        self.assertEqual(df.iloc[3]['name'], 'daughter(1,extraInfo(SignalProbability))')
        self.assertAlmostEqual(df.iloc[3]['importance'], 0.03015, delta=0.01)


class TestLoadBranchingFractionsDataFrame(unittest.TestCase):

    def test_isFloat(self):
        self.assertTrue(detailedReporting.isFloat("1.0"))
        self.assertTrue(detailedReporting.isFloat("-1.5"))
        self.assertTrue(detailedReporting.isFloat("1e-7"))
        self.assertTrue(detailedReporting.isFloat("50"))
        self.assertFalse(detailedReporting.isFloat("1f10"))
        self.assertFalse(detailedReporting.isFloat("5,6"))
        self.assertFalse(detailedReporting.isFloat("asd"))
        self.assertFalse(detailedReporting.isFloat("1.0f"))
        self.assertTrue(detailedReporting.isFloat("inf"))
        self.assertTrue(detailedReporting.isFloat("-inf"))
        self.assertTrue(detailedReporting.isFloat("nan"))

    def test_isValidParticle(self):
        self.assertTrue(detailedReporting.isValidParticle("e+"))
        self.assertTrue(detailedReporting.isValidParticle("J/psi"))
        self.assertTrue(detailedReporting.isValidParticle("K_S0"))
        self.assertTrue(detailedReporting.isValidParticle("D*+"))
        self.assertTrue(detailedReporting.isValidParticle("D_s*+"))
        self.assertTrue(detailedReporting.isValidParticle("gamma"))
        self.assertFalse(detailedReporting.isValidParticle("D_f+"))
        self.assertFalse(detailedReporting.isValidParticle("WW"))
        self.assertFalse(detailedReporting.isValidParticle("Z'"))
        self.assertFalse(detailedReporting.isValidParticle("gam"))
        self.assertFalse(detailedReporting.isValidParticle("hypergraviton"))
        self.assertFalse(detailedReporting.isValidParticle("Thomas'Particle"))  # TODO Fix this, should be true

    def test_loadBranchingFractionsDataFrame(self):
        df = detailedReporting.loadBranchingFractionsDataFrame()

        self.assertEqual(len(df), len(df.drop_duplicates()))
        self.assertListEqual(sorted(df.columns), sorted(['particle', 'channel', 'fraction']))

        sdf = df.groupby(['particle'])['fraction'].sum()
        for fraction in sdf:
            self.assertAlmostEqual(fraction, 1.0, delta=0.03)

    def test_loadCoveredBranchingFractionsDataFrame(self):
        from fei import default_channels
        particles = default_channels.get_default_channels()
        df = detailedReporting.loadCoveredBranchingFractionsDataFrame(particles, include_daughter_fractions=True)

        self.assertListEqual(sorted(df.columns), sorted(['particle', 'channel', 'channelName', 'fraction']))

        sdf = df.groupby(['particle'])['fraction'].sum()

        # In total we reconstruct 21, if new ones are added, don't forget to add test for
        # the branching fractions below, do NOT just increase this number ;-)
        self.assertEqual(len(sdf), 21)

        # Make covered branching fraction will only rise from this point on!
        # Upper boundary should prevent unreasonable high values to be accepted (due to bugs)
        self.assertTrue(0.0148 < sdf['B+:generic'] < 0.016)
        self.assertTrue(0.0083 < sdf['B0:generic'] < 0.01)
        self.assertTrue(0.0450 < sdf['B+:semileptonic'] < 0.05)
        self.assertTrue(0.0293 < sdf['B0:semileptonic'] < 0.035)
        self.assertTrue(0.1840 < sdf['D*+:generic'] < 0.2)
        self.assertTrue(0.2699 < sdf['D*0:generic'] < 0.3)
        self.assertTrue(0.1908 < sdf['D+:generic'] < 0.22)
        self.assertTrue(0.2719 < sdf['D0:generic'] < 0.3)
        self.assertTrue(0.0512 < sdf['D_s*+:generic'] < 0.06)
        self.assertTrue(0.0513 < sdf['D_s+:generic'] < 0.06)
        self.assertTrue(0.1186 < sdf['J/psi:generic'] < 0.14)
        self.assertTrue(0.9882 < sdf['pi0:generic'] < 1.0)
        self.assertTrue(0.6913 < sdf['K_S0:generic'] < 0.9)

        # Final state particles have always branching fraction 1.0
        self.assertEqual(sdf['e+:generic'], 1.0)
        self.assertEqual(sdf['mu+:generic'], 1.0)
        self.assertEqual(sdf['pi+:generic'], 1.0)
        self.assertEqual(sdf['K+:generic'], 1.0)
        self.assertEqual(sdf['gamma:generic'], 1.0)


if __name__ == '__main__':
    tempdir = tempfile.mkdtemp()
    os.chdir(tempdir)
    # main() never returns, so install exit handler to do our cleanup
    atexit.register(shutil.rmtree, tempdir)
    unittest.main()

# @endcond
