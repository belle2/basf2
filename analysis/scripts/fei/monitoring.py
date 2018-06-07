#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thoms Keck 2017

"""
 Contains classes to read in the monitoring output
 and some simple plotting routines.

 This is used by printReporting.py and latexReporting.py
 to create summaries for a FEI training or application.
"""


import ROOT
from ROOT import gSystem
gSystem.Load('libanalysis.so')
from ROOT import Belle2
Belle2.Variable.Manager.Instance()

import pdg

import numpy as np

import os
import math
import functools
import copy
import pdb
import pickle

import basf2_mva_util
from basf2_mva_evaluation import plotting
from generators import get_default_decayfile


def removeJPsiSlash(string):
    """ Remove slashes in a string, which is not allowed for filenames. """
    return string.replace('/', '')


def load_config():
    """ Load the FEI configuration from the Summary.pickle file. """
    if not os.path.isfile('Summary.pickle'):
        raise RuntimeError("""Could not find Summary.pickle!
                              This file is automatically created by the FEI training.
                              But you can also create it yourself using:
                              pickle.dump((particles, configuration), open('Summary.pickle', 'wb'))""")
    return pickle.load(open('Summary.pickle', 'rb'))


class Statistic(object):
    """
    This class provides thhe efficiency, purity and other quantities for a
    given number of true signal candidates, signal candidates and background candidates
    """

    def __init__(self, nTrueSig, nSig, nBg):
        """
        Create a new Statistic object
        @param nTrueSig the number of true signal particles
        @param nSig the number of reconstructed signal candidates
        @param nBg the number of reconstructed background candidates
        """
        #: the number of true signal particles
        self.nTrueSig = nTrueSig
        #: the number of reconstructed signal candidates
        self.nSig = nSig
        #: the number of reconstructed background candidates
        self.nBg = nBg

    @property
    def nTotal(self):
        """ Returns total number of reconstructed candidates. """
        return self.nSig + self.nBg

    @property
    def purity(self):
        """ Returns the purity of the reconstructed candidates. """
        if self.nSig == 0:
            return 0.0
        if self.nTotal == 0:
            return 0.0
        return self.nSig / float(self.nTotal)

    @property
    def efficiency(self):
        """ Returns the efficiency of the reconstructed signal candidates with respect to the number of true signal particles. """
        if self.nSig == 0:
            return 0.0
        if self.nTrueSig == 0:
            return float('inf')
        return self.nSig / float(self.nTrueSig)

    @property
    def purityError(self):
        """ Returns the uncertainty of the purity. """
        if self.nTotal == 0:
            return 0.0
        return self.calcStandardDeviation(self.nSig, self.nTotal)

    @property
    def efficiencyError(self):
        """
        Returns the uncertainty of the efficiency.
        For an efficiency eps = self.nSig/self.nTrueSig, this function calculates the
        standard deviation according to http://arxiv.org/abs/physics/0701199 .
        """
        if self.nTrueSig == 0:
            return float('inf')
        return self.calcStandardDeviation(self.nSig, self.nTrueSig)

    def calcStandardDeviation(self, k, n):
        """ Helper method to calculate the standard deviation for efficiencies. """
        k = float(k)
        n = float(n)
        variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
        if variance <= 0:
            return 0.0
        return math.sqrt(variance)

    def __str__(self):
        """ Returns a string representation of a Statistic object. """
        o = "nTrueSig {}    nSig {}    nBg {}\n".format(self.nTrueSig, self.nSig, self.nBg)
        o += "Efficiency {:.3f} ({:.3f})\n".format(self.efficiency, self.efficiencyError)
        o += "Purity {:.3f} ({:.3f})\n".format(self.purity, self.purityError)
        return o

    def __add__(self, a):
        """ Adds two Statistics objects and returns a new object. """
        return Statistic(self.nTrueSig, self.nSig + a.nSig, self.nBg + a.nBg)

    def __radd__(self, a):
        """
        Returns a new Statistic object if the current one is added to zero.
        Necessary to apply sum-function to Statistic objects.
        """
        if a != 0:
            return NotImplemented
        return Statistic(self.nTrueSig, self.nSig, self.nBg)


class MonitoringHist(object):
    """
    Reads all TH1F and TH2F from a ROOT file
    and puts them into a more accesable format.
    """

    def __init__(self, filename):
        """
        Reads histograms from the given file
        @param filename the name of the ROOT file
        """
        #: Dictionary of bin-contents for each histogram
        self.values = {}
        #: Dictionary of bin-centers for each histogram
        self.centers = {}
        #: Dictionary of number of bins for each histogram
        self.nbins = {}
        #: Indicates if the histograms were successfully read
        self.valid = os.path.isfile(filename)

        if not self.valid:
            return

        f = ROOT.TFile(filename)

        for key in f.GetListOfKeys():
            name = Belle2.invertMakeROOTCompatible(key.GetName())
            hist = key.ReadObj()
            if not (isinstance(hist, ROOT.TH1D) or isinstance(hist, ROOT.TH1F) or
                    isinstance(hist, ROOT.TH2D) or isinstance(hist, ROOT.TH2F)):
                continue
            two_dimensional = isinstance(hist, ROOT.TH2D) or isinstance(hist, ROOT.TH2F)
            if two_dimensional:
                nbins = (hist.GetNbinsX(), hist.GetNbinsY())
                self.centers[name] = np.array([[hist.GetXaxis().GetBinCenter(i) for i in range(nbins[0] + 2)],
                                               [hist.GetYaxis().GetBinCenter(i) for i in range(nbins[1] + 2)]])
                self.values[name] = np.array([[hist.GetBinContent(i, j) for i in range(nbins[0] + 2)] for j in range(nbins[1] + 2)])
                self.nbins[name] = nbins
            else:
                nbins = hist.GetNbinsX()
                self.centers[name] = np.array([hist.GetBinCenter(i) for i in range(nbins + 2)])
                self.values[name] = np.array([hist.GetBinContent(i) for i in range(nbins + 2)])
                self.nbins[name] = nbins

    def sum(self, name):
        """
        Calculates the sum of a given histogram (== sum of all entries)
        @param name key of the histogram
        """
        if name not in self.centers:
            return np.nan
        return np.sum(self.values[name])

    def mean(self, name):
        """
        Calculates the mean of a given histogram
        @param name key of the histogram
        """
        if name not in self.centers:
            return np.nan
        return np.average(self.centers[name], weights=self.values[name])

    def std(self, name):
        """
        Calculates the standard deviation of a given histogram
        @param name key of the histogram
        """
        if name not in self.centers:
            return np.nan
        avg = np.average(self.centers[name], weights=self.values[name])
        return np.sqrt(np.average((self.centers[name] - avg)**2, weights=self.values[name]))

    def min(self, name):
        """
        Calculates the minimum of a given histogram
        @param name key of the histogram
        """
        if name not in self.centers:
            return np.nan
        nonzero = np.nonzero(self.values[name])[0]
        if len(nonzero) == 0:
            return np.nan
        return self.centers[name][nonzero[0]]

    def max(self, name):
        """
        Calculates the maximum of a given histogram
        @param name key of the histogram
        """
        if name not in self.centers:
            return np.nan
        nonzero = np.nonzero(self.values[name])[0]
        if len(nonzero) == 0:
            return np.nan
        return self.centers[name][nonzero[-1]]


class MonitoringNTuple(object):
    """
    Reads the ntuple named variables from a ROOT file
    """
    def __init__(self, filename):
        """
        Reads ntuple from the given file
        @param filename the name of the ROOT file
        """
        #: Indicates if the ntuple were successfully read
        self.valid = os.path.isfile(filename)
        if not self.valid:
            return
        #: Reference to the ROOT file, so it isn't closed
        self.f = ROOT.TFile(filename)
        #: Reference to the tree named variables inside the ROOT file
        self.tree = self.f.variables
        #: Filename so we can use it later
        self.filename = filename


class MonitoringModuleStatistics(object):
    """
    Reads the module statistics for a single particle from the outputted root file
    and puts them into a more accesable format
    """
    def __init__(self, particle):
        """
        Reads the module statistics from the file named Monitor_ModuleStatistics.root
        @param particle the particle for which the statistics are read
        """
        root_file = ROOT.TFile('Monitor_ModuleStatistics.root')
        persistentTree = root_file.Get('persistent')
        persistentTree.GetEntry(0)
        # Clone() needed so we actually own the object (original dies when tfile is deleted)
        stats = persistentTree.ProcessStatistics.Clone()

        # merge statistics from all persistent trees into 'stats'
        numEntries = persistentTree.GetEntriesFast()
        for i in range(1, numEntries):
            persistentTree.GetEntry(i)
            stats.merge(persistentTree.ProcessStatistics)

        # TODO .getTimeSum returns always 0 at the moment ?!
        statistic = {m.getName(): m.getTimeSum(m.c_Event) / 1e9 for m in stats.getAll()}

        #: the time for each channel
        self.channel_time = {}
        #: the time per module
        self.channel_time_per_module = {}
        for channel in particle.channels:
            if channel.label not in self.channel_time:
                self.channel_time[channel.label] = 0.0
                self.channel_time_per_module[channel.label] = {'ParticleCombiner': 0.0,
                                                               'BestCandidateSelection': 0.0,
                                                               'PListCutAndCopy': 0.0,
                                                               'VariablesToExtraInfo': 0.0,
                                                               'MCMatch': 0.0,
                                                               'ParticleSelector': 0.0,
                                                               'MVAExpert': 0.0,
                                                               'ParticleVertexFitter': 0.0,
                                                               'TagUniqueSignal': 0.0,
                                                               'VariablesToHistogram': 0.0,
                                                               'VariablesToNtuple': 0.0}
            for key, time in statistic.items():
                if(channel.decayString in key or channel.name in key):
                    self.channel_time[channel.label] += time
                    for k in self.channel_time_per_module[channel.label]:
                        if k in key:
                            self.channel_time_per_module[channel.label][k] += time

        #: the time per particle
        self.particle_time = 0
        for key, time in statistic.items():
            if particle.identifier in key:
                self.particle_time += time


def MonitorCosBDLPlot(particle, filename):
    """ Creates a CosBDL plot using ROOT. """
    if not particle.final_ntuple.valid:
        return
    df = basf2_mva_util.tree2dict(particle.final_ntuple.tree,
                                  ['extraInfo__bouniqueSignal__bc', 'cosThetaBetweenParticleAndTrueB',
                                   'extraInfo__boSignalProbability__bc', particle.particle.mvaConfig.target],
                                  ['unique', 'cosThetaBDl', 'probability', 'signal'])
    for i, cut in enumerate([0.0, 0.01, 0.05, 0.1, 0.2, 0.5]):
        p = plotting.VerboseDistribution()
        common = (df['unique'] == 1) & (np.abs(df['cosThetaBDl']) < 10) & (df['probability'] >= cut)
        p.add(df, 'cosThetaBDl', common & (df['signal'] == 1), label="Signal")
        p.add(df, 'cosThetaBDl', common & (df['signal'] == 0), label="Background")
        p.finish()
        p.axis.set_title("Cosine of Theta between B and Dl system for signal probability >= {:.2f}".format(cut))
        p.axis.set_xlabel("CosThetaBDl")
        p.save('{}_{}.png'.format(filename, i))


def MonitorMbcPlot(particle, filename):
    """ Creates a Mbc plot using ROOT. """
    if not particle.final_ntuple.valid:
        return
    df = basf2_mva_util.tree2dict(particle.final_ntuple.tree,
                                  ['extraInfo__bouniqueSignal__bc', 'Mbc',
                                   'extraInfo__boSignalProbability__bc', particle.particle.mvaConfig.target],
                                  ['unique', 'Mbc', 'probability', 'signal'])
    for i, cut in enumerate([0.0, 0.01, 0.05, 0.1, 0.2, 0.5]):
        p = plotting.VerboseDistribution()
        common = (df['unique'] == 1) & (df['Mbc'] > 5.23) & (df['probability'] >= cut)
        p.add(df, 'Mbc', common & (df['signal'] == 1), label="Signal")
        p.add(df, 'Mbc', common & (df['signal'] == 0), label="Background")
        p.finish()
        p.axis.set_title("Beam constrained mass for signal probability >= {:.2f}".format(cut))
        p.axis.set_xlabel("Mbc")
        p.save('{}_{}.png'.format(filename, i))


def MonitorROCPlot(particle, filename):
    """ Creates a ROC plot using ROOT. """
    if not particle.final_ntuple.valid:
        return
    df = basf2_mva_util.tree2dict(particle.final_ntuple.tree,
                                  ['extraInfo__bouniqueSignal__bc',
                                   'extraInfo__boSignalProbability__bc', particle.particle.mvaConfig.target],
                                  ['unique', 'probability', 'signal'])
    p = plotting.RejectionOverEfficiency()
    p.add(df, 'probability', df['signal'] == 1, df['signal'] == 0, label='All')
    p.add(df, 'probability', (df['signal'] == 1) & (df['unique'] == 1), (df['signal'] == 0) & (df['unique'] == 1), label='Unique')
    p.finish()
    p.save(filename + '.png')


def MonitorDiagPlot(particle, filename):
    """ Creates a Diagonal plot using ROOT. """
    if not particle.final_ntuple.valid:
        return
    df = basf2_mva_util.tree2dict(particle.final_ntuple.tree,
                                  ['extraInfo__bouniqueSignal__bc',
                                   'extraInfo__boSignalProbability__bc', particle.particle.mvaConfig.target],
                                  ['unique', 'probability', 'signal'])
    p = plotting.Diagonal()
    p.add(df, 'probability', df['signal'] == 1, df['signal'] == 0)
    # p.add(df, 'probability', (df['signal'] == 1) & (df['unique'] == 1), (df['signal'] == 0) & (df['unique'] == 1))
    p.finish()
    p.save(filename + '.png')


def MonitoringMCCount(particle):
    """
    Reads the MC Counts for a given particle from the ROOT file mcParticlesCount.root
    @param particle the particle for which the MC counts are read
    @return dictionary with 'sum', 'std', 'avg', 'max', and 'min'
    """
    root_file = ROOT.TFile('mcParticlesCount.root')

    key = 'NumberOfMCParticlesInEvent({})'.format(abs(pdg.from_name(particle.name)))
    Belle2.Variable.Manager
    key = Belle2.makeROOTCompatible(key)
    hist = root_file.Get(key)

    mc_counts = {'sum': 0, 'std': 0, 'avg': 0, 'min': 0, 'max': 0}
    if hist:
        mc_counts['sum'] = sum(hist.GetXaxis().GetBinCenter(bin + 1) * hist.GetBinContent(bin + 1)
                               for bin in range(hist.GetNbinsX()))
        mc_counts['std'] = hist.GetStdDev()
        mc_counts['avg'] = hist.GetMean()
        mc_counts['max'] = hist.GetXaxis().GetBinCenter(hist.FindLastBinAbove(0.0))
        mc_counts['min'] = hist.GetXaxis().GetBinCenter(hist.FindFirstBinAbove(0.0))
    return mc_counts


class MonitoringBranchingFractions(object):
    """ Class extracts the branching fractions of a decay channel from the DECAY.DEC file. """
    #: is the monitoring shared
    _shared = None

    def __init__(self):
        """
        Create a new MonitoringBranchingFraction object.
        The extracted branching fractions are cached, hence createing more than one object does not do anything.
        """
        if MonitoringBranchingFractions._shared is None:
            decay_file = get_default_decayfile()
            #: exclusive branching fractions
            self.exclusive_branching_fractions = self.loadExclusiveBranchingFractions(decay_file)
            #: inclusive branching fractions
            self.inclusive_branching_fractions = self.loadInclusiveBranchingFractions(self.exclusive_branching_fractions)
            MonitoringBranchingFractions._shared = (self.exclusive_branching_fractions, self.inclusive_branching_fractions)
        else:
            self.exclusive_branching_fractions, self.inclusive_branching_fractions = MonitoringBranchingFractions._shared

    def getExclusive(self, particle):
        """ Returns the exclusive (i.e. without the branching fractions of the daughters) branching fraction of a particle. """
        return self.getBranchingFraction(particle, self.exclusive_branching_fractions)

    def getInclusive(self, particle):
        """ Returns the inclusive (i.e. including all branching fractions of the daughters) branching fraction of a particle. """
        return self.getBranchingFraction(particle, self.inclusive_branching_fractions)

    def getBranchingFraction(self, particle, branching_fractions):
        """ Returns the branching fraction of a particle given a branching_fraction table. """
        result = {c.label: 0.0 for c in particle.channels}
        name = particle.name
        channels = [tuple(sorted(d.split(':')[0] for d in channel.daughters)) for channel in particle.channels]
        if name not in branching_fractions:
            name = pdg.conjugate(name)
            channels = [tuple(pdg.conjugate(d) for d in channel) for channel in channels]
            if name not in branching_fractions:
                return result
        for c, key in zip(particle.channels, channels):
            if key in branching_fractions[name]:
                result[c.label] = branching_fractions[name][key]
        return result

    def loadExclusiveBranchingFractions(self, filename):
        """
        Load branching fraction from MC decay-file.
        """

        def isFloat(element):
            """ Checks if element is a convertible to float"""
            try:
                float(element)
                return True
            except ValueError:
                return False

        def isValidParticle(element):
            """ Checks if element is a valid pdg name for a particle"""
            try:
                pdg.from_name(element)
                return True
            except LookupError:
                return False

        branching_fractions = {'UNKOWN': {}}

        mother = 'UNKOWN'
        with open(filename, 'r') as f:
            for line in f:
                fields = line.split(' ')
                fields = [x for x in fields if x != '']
                if len(fields) < 2 or fields[0][0] == '#':
                    continue
                if fields[0] == 'Decay':
                    mother = fields[1].strip()
                    if not isValidParticle(mother):
                        mother = 'UNKOWN'
                    continue
                if fields[0] == 'Enddecay':
                    mother = 'UNKOWN'
                    continue
                if mother == 'UNKOWN':
                    continue
                fields = fields[:-1]
                if len(fields) < 1 or not isFloat(fields[0]):
                    continue
                while len(fields) > 1:
                    if isValidParticle(fields[-1]):
                        break
                    fields = fields[:-1]
                if len(fields) < 1 or not all(isValidParticle(p) for p in fields[1:]):
                    continue
                neutrinoTag_list = ['nu_e', 'nu_mu', 'nu_tau', 'anti-nu_e', 'anti-nu_mu', 'anti-nu_tau']
                daughters = tuple(sorted(p for p in fields[1:] if p not in neutrinoTag_list))
                if mother not in branching_fractions:
                    branching_fractions[mother] = {}
                if daughters not in branching_fractions[mother]:
                    branching_fractions[mother][daughters] = 0.0
                branching_fractions[mother][daughters] += float(fields[0])

        del branching_fractions['UNKOWN']
        return branching_fractions

    def loadInclusiveBranchingFractions(self, exclusive_branching_fractions):
        """
        Get covered branching fraction of a particle using a recursive algorithm
        and the given exclusive branching_fractions (given as Hashable List)
        @param particle identifier of the particle
        @param branching_fractions
        """
        particles = set(exclusive_branching_fractions.keys())
        particles.update(set(pdg.conjugate(p) for p in particles if p != pdg.conjugate(p)))
        particles = sorted(particles, key=lambda x: pdg.get(x).Mass())
        inclusive_branching_fractions = copy.deepcopy(exclusive_branching_fractions)

        for p in particles:
            if p in inclusive_branching_fractions:
                br = sum(inclusive_branching_fractions[p].values())
            else:
                br = sum(inclusive_branching_fractions[pdg.conjugate(p)].values())
            for p_br in inclusive_branching_fractions.values():
                for c in p_br:
                    for i in range(c.count(p)):
                        p_br[c] *= br
        return inclusive_branching_fractions


class MonitoringParticle(object):
    """
    Monitoring object containing all the monitoring information
    about a single particle
    """
    def __init__(self, particle):
        """
        Read the monitoring information of the given particle
        @param particle the particle for which the information is read
        """
        #: Particle containing its configuration
        self.particle = particle
        #: Dictionary with 'sum', 'std', 'mean', 'min' and 'max' of the MC counts
        self.mc_count = MonitoringMCCount(particle)
        #: Module statistics
        self.module_statistic = MonitoringModuleStatistics(particle)
        #: time per channel
        self.time_per_channel = self.module_statistic.channel_time
        #: time per channel per module
        self.time_per_channel_per_module = self.module_statistic.channel_time_per_module
        #: total time
        self.total_time = self.module_statistic.particle_time + sum(self.time_per_channel.values())

        #: Total number of channels
        self.total_number_of_channels = len(self.particle.channels)
        #: Reconstructed number of channels
        self.reconstructed_number_of_channels = 0

        #: Branching fractions
        self.branching_fractions = MonitoringBranchingFractions()
        #: Exclusive branching fractions per channel
        self.exc_br_per_channel = self.branching_fractions.getExclusive(particle)
        #: Inclusive branching fraction per channel
        self.inc_br_per_channel = self.branching_fractions.getInclusive(particle)

        #: Monitoring histogram in PreReconstruction before the ranking-cut
        self.before_ranking = {}
        #: Monitoring histogram in PreReconstruction after the ranking-cut
        self.after_ranking = {}
        #: Monitoring histogram in PreReconstruction after the vertex fit
        self.after_vertex = {}
        #: Monitoring histogram in PostReconstruction after the mva application
        self.after_classifier = {}
        #: Monitoring histogram for TrainingData Generation only available if Monitoring runs on the training monitoring data
        self.training_data = {}
        #: Dictionary containing whether the channel reconstructed at least one candidate or not
        self.ignored_channels = {}

        for channel in self.particle.channels:
            hist = MonitoringHist('Monitor_PreReconstruction_BeforeRanking_{}.root'.format(channel.label))
            self.before_ranking[channel.label] = self.calculateStatistic(hist, channel.mvaConfig.target)
            hist = MonitoringHist('Monitor_PreReconstruction_AfterRanking_{}.root'.format(channel.label))
            self.after_ranking[channel.label] = self.calculateStatistic(hist, channel.mvaConfig.target)
            hist = MonitoringHist('Monitor_MatchParticleList_AfterVertex_{}.root'.format(channel.label))
            self.after_vertex[channel.label] = self.calculateStatistic(hist, channel.mvaConfig.target)
            hist = MonitoringHist('Monitor_PostReconstruction_AfterMVA_{}.root'.format(channel.label))
            self.after_classifier[channel.label] = self.calculateStatistic(hist, channel.mvaConfig.target)
            if hist.valid and hist.sum(channel.mvaConfig.target) > 0:
                self.reconstructed_number_of_channels += 1
                self.ignored_channels[channel.label] = False
            else:
                self.ignored_channels[channel.label] = True
            hist = MonitoringHist('Monitor_TrainingData_{}.root'.format(channel.label))
            self.training_data[channel.label] = hist

        plist = removeJPsiSlash(particle.identifier)
        hist = MonitoringHist('Monitor_PostReconstruction_BeforePostCut_{}.root'.format(plist))
        #: Monitoring histogram in PostReconstruction before the postcut
        self.before_postcut = self.calculateStatistic(hist, self.particle.mvaConfig.target)
        hist = MonitoringHist('Monitor_PostReconstruction_BeforeRanking_{}.root'.format(plist))
        #: Monitoring histogram in PostReconstruction before the ranking postcut
        self.before_ranking_postcut = self.calculateStatistic(hist, self.particle.mvaConfig.target)
        hist = MonitoringHist('Monitor_PostReconstruction_AfterRanking_{}.root'.format(plist))
        #: Monitoring histogram in PostReconstruction after the ranking postcut
        self.after_ranking_postcut = self.calculateStatistic(hist, self.particle.mvaConfig.target)
        #: Statistic object before unique tagging of signals
        self.before_tag = self.calculateStatistic(hist, self.particle.mvaConfig.target)
        #: Statistic object after unique tagging of signals
        self.after_tag = self.calculateUniqueStatistic(hist)
        #: Reference to the final ntuple
        self.final_ntuple = MonitoringNTuple('Monitor_Final_{}.root'.format(plist))

    def calculateStatistic(self, hist, target):
        """
        Calculate Statistic object where all signal candidates are considered signal
        """
        nTrueSig = self.mc_count['sum']
        if not hist.valid:
            return Statistic(nTrueSig, 0, 0)
        signal_bins = (hist.centers[target] > 0.5)
        bckgrd_bins = ~signal_bins
        nSig = hist.values[target][signal_bins].sum()
        nBg = hist.values[target][bckgrd_bins].sum()
        return Statistic(nTrueSig, nSig, nBg)

    def calculateUniqueStatistic(self, hist):
        """
        Calculate Static object where only unique signal candidates are considered signal
        """
        nTrueSig = self.mc_count['sum']
        if not hist.valid:
            return Statistic(nTrueSig, 0, 0)
        signal_bins = hist.centers['extraInfo(uniqueSignal)'] > 0.5
        bckgrd_bins = hist.centers['extraInfo(uniqueSignal)'] <= 0.5
        nSig = hist.values['extraInfo(uniqueSignal)'][signal_bins].sum()
        nBg = hist.values['extraInfo(uniqueSignal)'][bckgrd_bins].sum()
        return Statistic(nTrueSig, nSig, nBg)
