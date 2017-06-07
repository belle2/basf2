#!/usr/bin/env python
# -*- coding: utf-8 -*-

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


def purity(nSig, nBg):
    if nSig == 0:
        return 0.0
    if nSig + nBg == 0:
        return 0.0
    return nSig / float(nSig + nBg)


def efficiency(nSig, nTrueSig):
    if nSig == 0:
        return 0.0
    if nTrueSig == 0:
        return float('inf')
    return nSig / float(nTrueSig)


def efficiencyError(nSig, nTrueSig):
    """
    for an efficiency eps = nSig/nTrueSig, this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """
    if nTrueSig == 0:
        return float('inf')

    k = float(nSig)
    n = float(nTrueSig)
    variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
    if variance <= 0:
        return 0.0
    return math.sqrt(variance)


def purityError(nSig, nBg):
    nTot = nSig + nBg
    if nTot == 0:
        return 0.0
    return efficiencyError(nSig, nTot)


def removeJPsiSlash(string):
    return string.replace('/', '')


class Statistic(object):

    def __init__(self, nTrueSig, nSig, nBg):
        self.nTrueSig = nTrueSig
        self.nSig = nSig
        self.nBg = nBg

    @property
    def nTotal(self):
        return self.nSig + self.nBg

    @property
    def purity(self):
        if self.nSig == 0:
            return 0.0
        if self.nTotal == 0:
            return 0.0
        return self.nSig / float(self.nTotal)

    @property
    def efficiency(self):
        if self.nSig == 0:
            return 0.0
        if self.nTrueSig == 0:
            return float('inf')
        return self.nSig / float(self.nTrueSig)

    @property
    def purityError(self):
        if self.nTotal == 0:
            return 0.0
        return self.calcStandardDeviation(self.nSig, self.nTotal)

    @property
    def efficiencyError(self):
        """
        for an efficiency eps = self.nSig/self.nTrueSig, this function calculates the
        standard deviation according to http://arxiv.org/abs/physics/0701199 .
        """
        if self.nTrueSig == 0:
            return float('inf')
        return self.calcStandardDeviation(self.nSig, self.nTrueSig)

    def calcStandardDeviation(self, k, n):
        k = float(k)
        n = float(n)
        variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
        if variance <= 0:
            return 0.0
        return math.sqrt(variance)

    def __str__(self):
        o = "nTrueSig {}    nSig {}    nBg {}\n".format(self.nTrueSig, self.nSig, self.nBg)
        o += "Efficiency {:.3f} ({:.3f})\n".format(self.efficiency, self.efficiencyError)
        o += "Purity {:.3f} ({:.3f})\n".format(self.purity, self.purityError)
        return o

    def __add__(self, a):
        return Statistic(self.nTrueSig, self.nSig + a.nSig, self.nBg + a.nBg)

    def __radd__(self, a):
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

        self.channel_time = {}
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

        self.particle_time = 0
        for key, time in statistic.items():
            if particle.identifier in key:
                self.particle_time += time


def MonitorCosBDLPlot(particle, filename):
    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(filename, 'Cosine of Theta between B and Dl system', 1600, 1200)
    canvas.cd()

    if particle.final_ntuple.valid and particle.mc_count['sum'] > 0:
        ntuple = particle.final_ntuple.tree

        color = ROOT.kRed + 4
        first_plot = True
        common = 'extraInfo__bouniqueSignal__bc == 1 && abs(cosThetaBetweenParticleAndTrueB) < 10'
        common += ' && extraInfo__boSignalProbability__bc > '
        for cut in [0.01, 0.1, 0.5]:
            ntuple.SetLineColor(int(color))
            ntuple.SetLineStyle(ROOT.kSolid)
            ntuple.Draw('cosThetaBetweenParticleAndTrueB', common + str(cut), '' if first_plot else 'same')
            first_plot = False

            ntuple.SetLineStyle(ROOT.kDotted)
            ntuple.Draw('cosThetaBetweenParticleAndTrueB',
                        common + str(cut) + ' && !' + particle.particle.mvaConfig.target, 'same')
            color -= 1

        l = canvas.GetListOfPrimitives()
        for i in range(l.GetEntries()):
            hist = l[i]
            if isinstance(hist, ROOT.TH1D):
                hist.GetXaxis().SetRangeUser(-10, 10)
                break

        legend = canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)
        legend.SetFillStyle(0)
    canvas.SaveAs(filename)


def MonitorMbcPlot(particle, filename):
    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(filename, 'Beam constrained Mass', 1600, 1200)
    canvas.cd()

    if particle.final_ntuple.valid and particle.mc_count['sum'] > 0:
        ntuple = particle.final_ntuple.tree

        color = ROOT.kRed + 4
        first_plot = True
        for cut in [0.01, 0.1, 0.5]:
            ntuple.SetLineColor(int(color))
            ntuple.SetLineStyle(ROOT.kSolid)
            common = 'Mbc > 5.23 && extraInfo__boSignalProbability__bc > '
            ntuple.Draw('Mbc', common + str(cut), '' if first_plot else 'same')
            first_plot = False

            ntuple.SetLineStyle(ROOT.kDotted)
            ntuple.Draw('Mbc', common + str(cut) + ' && !' + particle.particle.mvaConfig.target, 'same')
            color -= 1

        l = canvas.GetListOfPrimitives()
        for i in range(l.GetEntries()):
            hist = l[i]
            if isinstance(hist, ROOT.TH1D):
                hist.GetXaxis().SetRangeUser(5.24, 5.29)
                break

        legend = canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)
        legend.SetFillStyle(0)
    canvas.SaveAs(filename)


def MonitorROCPlot(particle, filename):
    ROOT.gROOT.SetBatch(True)
    canvas = ROOT.TCanvas(filename, "ROC curve", 1600, 1200)
    canvas.cd()

    if particle.final_ntuple.valid and particle.mc_count['sum'] > 0:
        ntuple = particle.final_ntuple.tree

        nbins = 100
        import array
        bckgrdHist = ROOT.TH1D('ROCbackground', 'background', nbins, 0.0, 1.0)
        signalHist = ROOT.TH1D('ROCsignal', 'signal', nbins, 0.0, 1.0)
        uniqueBckgrdHist = ROOT.TH1D('ROCuniqueBackground', 'background', nbins, 0.0, 1.0)
        uniqueSignalHist = ROOT.TH1D('ROCuniqueSignal', 'signal', nbins, 0.0, 1.0)

        probabilityVar = 'extraInfo__boSignalProbability__bc'
        ntuple.Project('ROCbackground', probabilityVar, '!' + particle.particle.mvaConfig.target)
        ntuple.Project('ROCsignal', probabilityVar, particle.particle.mvaConfig.target)
        ntuple.Project('ROCuniqueBackground', probabilityVar, '!' + particle.particle.mvaConfig.target)
        ntuple.Project('ROCuniqueSignal', probabilityVar,
                       particle.particle.mvaConfig.target + '  && extraInfo__bouniqueSignal__bc == 1')

        for i, (signal, bckgrd) in enumerate([(signalHist, bckgrdHist)]):  # , (uniqueSignalHist, uniqueBckgrdHist)]):
            x = array.array('d')
            y = array.array('d')
            xerr = array.array('d')
            yerr = array.array('d')

            for cutBin in range(nbins + 1):
                nSignal = signal.Integral(cutBin, nbins + 1)
                nBckgrd = bckgrd.Integral(cutBin, nbins + 1)

                eff = efficiency(nSignal, particle.mc_count['sum'])
                effErr = efficiencyError(nSignal, particle.mc_count['sum'])
                pur = purity(nSignal, nBckgrd)
                purErr = purityError(nSignal, nBckgrd)

                x.append(100 * pur)
                y.append(100 * eff)
                xerr.append(100 * purErr)
                yerr.append(100 * effErr)

            rocgraph = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)
            rocgraph.SetLineColor(ROOT.kBlue - 2 - i)
            rocgraph.SetTitle(';purity (%);efficiency (%)')
            rocgraph.GetXaxis().SetTitleSize(0.05)
            rocgraph.GetXaxis().SetLabelSize(0.05)
            rocgraph.GetYaxis().SetTitleSize(0.05)
            rocgraph.GetYaxis().SetLabelSize(0.05)
            rocgraph.Draw('ALPZ')

    canvas.SaveAs(filename)


def MonitorDiagPlot(particle, filename):
    ROOT.gROOT.SetBatch(True)
    nbins = 100
    probabilityVar = ROOT.Belle2.makeROOTCompatible('extraInfo(SignalProbability)')

    canvas = ROOT.TCanvas(filename, 'Diagonal plot', 1600, 1200)
    canvas.cd()

    if particle.final_ntuple.valid and particle.mc_count['sum'] > 0:
        ntuple = particle.final_ntuple.tree

        bgHist = ROOT.TH1D('background' + probabilityVar, 'background', nbins, 0.0, 1.0)
        signalHist = ROOT.TH1D('signal' + probabilityVar, 'signal', nbins, 0.0, 1.0)

        ntuple.Project('background' + probabilityVar, probabilityVar, '!' + particle.particle.mvaConfig.target)
        ntuple.Project('signal' + probabilityVar, probabilityVar, particle.particle.mvaConfig.target)

        import array
        x = array.array('d')
        y = array.array('d')
        xerr = array.array('d')
        yerr = array.array('d')

        for i in range(1, nbins + 1):  # no under/overflow bins
            nSig = 1.0 * signalHist.GetBinContent(i)
            nBg = 1.0 * bgHist.GetBinContent(i)
            binCenter = signalHist.GetXaxis().GetBinCenter(i)
            x.append(binCenter)
            y.append(purity(nSig, nBg))
            xerr.append(signalHist.GetXaxis().GetBinWidth(i) / 2)
            yerr.append(purityError(nSig, nBg))

        purityPerBin = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)

        plotLabel = ';classifier output;purity per bin'
        purityPerBin.SetTitle(plotLabel)
        purityPerBin.GetXaxis().SetRangeUser(0.0, 1.0)
        purityPerBin.GetYaxis().SetRangeUser(0.0, 1.0)
        purityPerBin.GetXaxis().SetTitleSize(0.05)
        purityPerBin.GetXaxis().SetLabelSize(0.05)
        purityPerBin.GetYaxis().SetTitleSize(0.05)
        purityPerBin.GetYaxis().SetLabelSize(0.05)
        purityPerBin.Draw('APZ')
        diagonal = ROOT.TLine(0.0, 0.0, 1.0, 1.0)
        diagonal.SetLineColor(ROOT.kAzure)
        diagonal.SetLineWidth(2)
        diagonal.Draw()
    canvas.SaveAs(filename)


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
    _shared = None

    def __init__(self):
        if MonitoringBranchingFractions._shared is None:
            decay_file = os.getenv('BELLE2_EXTERNALS_DIR') + '/share/evtgen/DECAY.DEC'
            self.exclusive_branching_fractions = self.loadExclusiveBranchingFractions(decay_file)
            self.inclusive_branching_fractions = self.loadInclusiveBranchingFractions(self.exclusive_branching_fractions)
            MonitoringBranchingFractions._shared = (self.exclusive_branching_fractions, self.inclusive_branching_fractions)
        else:
            self.exclusive_branching_fractions, self.inclusive_branching_fractions = MonitoringBranchingFractions._shared

    def getExclusive(self, particle):
        return self.getBranchingFraction(particle, self.exclusive_branching_fractions)

    def getInclusive(self, particle):
        return self.getBranchingFraction(particle, self.inclusive_branching_fractions)

    def getBranchingFraction(self, particle, branching_fractions):
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
        self.time_per_channel = self.module_statistic.channel_time
        self.time_per_channel_per_module = self.module_statistic.channel_time_per_module
        self.total_time = self.module_statistic.particle_time + sum(self.time_per_channel.values())

        #: Total number of channels
        self.total_number_of_channels = len(self.particle.channels)
        self.reconstructed_number_of_channels = 0

        #: Branching fractions
        self.branching_fractions = MonitoringBranchingFractions()
        # Exclusive branching fractions per channel
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
