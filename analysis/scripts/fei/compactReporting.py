#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT
from ROOT import Belle2
import variables
import pdg
from fei import *
from B2Tools import format
from B2Tools import b2stat

import pickle
import os
import sys
import math
import functools
import pprint


def removeJPsiSlash(string):
    return string.replace('/', '')


def GetEntriesSafe(tree, selection):
    """
    Replacement for TTree::GetEntries(const char* selection) that
    doesn't stop after a 1 billion entries or so.
    """
    s = ROOT.TSelectorEntries(selection)
    tree.Process(s, "", 1000000000000000)
    tree.SetNotify(0)
    return s.GetSelectedRows()


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


def makeCosBDLPlot(ntuple, outputFileName, targetVar):
    """
    Using the TNTuple in 'fileName', save CosThetaBDL plot in 'outputFileName'.
    Shows effect of different cuts on SignalProbability, plus signal distribution.
    """
    ROOT.gROOT.SetBatch(True)
    plotTitle = 'CosThetaBDL plot'
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(outputFileName, plotTitle, 600, 400)
    canvas.cd()

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
        ntuple.Draw('cosThetaBetweenParticleAndTrueB', common + str(cut) + ' && !' + targetVar, 'same')
        color -= 1

    l = canvas.GetListOfPrimitives()
    for i in range(l.GetEntries()):
        hist = l[i]
        if isinstance(hist, ROOT.TH1D):
            hist.GetXaxis().SetRangeUser(-10, 10)
            break

    legend = canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)
    legend.SetFillStyle(0)

    canvas.SaveAs(outputFileName)


def makeMbcPlot(ntuple, outputFileName, targetVar):
    """
    Using the TNTuple in 'fileName', save M_bc plot in 'outputFileName'.
    Shows effect of different cuts on SignalProbability, plus signal distribution.
    """
    ROOT.gROOT.SetBatch(True)
    plotTitle = 'Mbc plot'
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(outputFileName, plotTitle, 600, 400)
    canvas.cd()

    color = ROOT.kRed + 4
    first_plot = True
    for cut in [0.01, 0.1, 0.5]:
        ntuple.SetLineColor(int(color))
        ntuple.SetLineStyle(ROOT.kSolid)
        common = 'Mbc > 5.23 && extraInfo__boSignalProbability__bc > '
        ntuple.Draw('Mbc', common + str(cut), '' if first_plot else 'same')
        first_plot = False

        ntuple.SetLineStyle(ROOT.kDotted)
        ntuple.Draw('Mbc', common + str(cut) + ' && !' + targetVar, 'same')
        color -= 1

    l = canvas.GetListOfPrimitives()
    for i in range(l.GetEntries()):
        hist = l[i]
        if isinstance(hist, ROOT.TH1D):
            hist.GetXaxis().SetRangeUser(5.24, 5.29)
            break

    legend = canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)
    legend.SetFillStyle(0)
    canvas.SaveAs(outputFileName)


def makeROCPlotFromNtuple(ntuple, outputFileName, nTrueSignal, targetVar):
    """
    Using the TNTuple in 'fileName', save an efficiency over purity plot in 'outputFileName'.

    @param nTrueSignal number of true signal particles in the sample.
    """
    ROOT.gROOT.SetBatch(True)
    plotTitle = 'ROC curve'
    canvas = ROOT.TCanvas(outputFileName, plotTitle, 600, 400)
    canvas.cd()

    nbins = 100
    import array

    bckgrdHist = ROOT.TH1D('ROCbackground', 'background', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('ROCsignal', 'signal', nbins, 0.0, 1.0)
    uniqueBckgrdHist = ROOT.TH1D('ROCuniqueBackground', 'background', nbins, 0.0, 1.0)
    uniqueSignalHist = ROOT.TH1D('ROCuniqueSignal', 'signal', nbins, 0.0, 1.0)

    probabilityVar = 'extraInfo__boSignalProbability__bc'
    ntuple.Project('ROCbackground', probabilityVar, '!' + targetVar)
    ntuple.Project('ROCsignal', probabilityVar, targetVar)
    ntuple.Project('ROCuniqueBackground', probabilityVar, '!' + targetVar)
    ntuple.Project('ROCuniqueSignal', probabilityVar, targetVar + '  && extraInfo__bouniqueSignal__bc == 1')

    for i, (signal, bckgrd) in enumerate([(signalHist, bckgrdHist), (uniqueSignalHist, uniqueBckgrdHist)]):
        x = array.array('d')
        y = array.array('d')
        xerr = array.array('d')
        yerr = array.array('d')

        for cutBin in range(nbins + 1):
            nSignal = signal.Integral(cutBin, nbins + 1)
            nBckgrd = bckgrd.Integral(cutBin, nbins + 1)

            efficiency = nSignal / nTrueSignal
            efficiencyErr = efficiencyError(nSignal, nTrueSignal)
            try:
                purity = nSignal / (nSignal + nBckgrd)
                purityErr = efficiencyError(nSignal, nSignal + nBckgrd)
            except ZeroDivisionError:
                purity = 0
                purityErr = 0

            x.append(100 * purity)
            y.append(100 * efficiency)
            xerr.append(100 * purityErr)
            yerr.append(100 * efficiencyErr)

        rocgraph = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)
        rocgraph.SetLineColor(ROOT.kBlue - 2 - i)
        rocgraph.SetTitle(';purity (%);efficiency (%)')
        rocgraph.GetXaxis().SetTitleSize(0.05)
        rocgraph.GetXaxis().SetLabelSize(0.05)
        rocgraph.GetYaxis().SetTitleSize(0.05)
        rocgraph.GetYaxis().SetLabelSize(0.05)
        rocgraph.Draw('ALPZ')

    canvas.SaveAs(outputFileName)


def createParticlePlots(particle, ntuple_filename, mc_counts):
    if ntuple_filename is None:
        return
    root_file = ROOT.TFile(ntuple_filename)
    ntuple = root_file.Get('variables')
    mc_count = mc_counts[abs(pdg.from_name(particle.name))]
    makeROCPlotFromNtuple(ntuple,
                          removeJPsiSlash(particle.name) + '_' + particle.label + '_roc.png',
                          mc_count,
                          particle.mvaConfig.target)
    if particle.identifier in ['B+:generic', 'B0:generic']:
        makeMbcPlot(ntuple,
                    removeJPsiSlash(particle.name) + '_' + particle.label + '_money.png',
                    particle.mvaConfig.target)
    if particle.identifier in ['B+:semileptonic', 'B0:semileptonic']:
        makeCosBDLPlot(ntuple,
                       removeJPsiSlash(particle.name) + '_' + particle.label + '_money.png',
                       particle.mvaConfig.target)


def getListCounts(obj, func=calcSum):
    """
    Calculate given function for all ParticleLists
    @param obj object dumped by Summary Provider of FEI
    @param func used to reduce ROOT histogram to a number
    @return dictionary(listname, dictionary(mode, value))
    """
    root_file = ROOT.TFile(obj['list_counts'])
    list_counts = {}
    for key in root_file.GetListOfKeys():
        variable = Belle2.Variable.invertMakeROOTCompatible(key.GetName())
        argument = variable[len('countInList('):-1]
        listname = argument.split(',', 1)[0]
        mode = 'All' if ',' not in argument else 'Signal' if '1' == argument[-1] else 'Background'

        if listname not in list_counts:
            list_counts[listname] = {}
        list_counts[listname][mode] = func(key.ReadObj())
    return list_counts


def getModuleStatistics(obj):
    root_file = ROOT.TFile(obj['module_statistics'])
    persistentTree = root_file.Get('persistent')
    persistentTree.GetEntry(0)
    # Clone() needed so we actually own the object (original dies when tfile is deleted)
    stats = persistentTree.ProcessStatistics.Clone()

    # merge statistics from all persistent trees into 'stats'
    numEntries = persistentTree.GetEntriesFast()
    for i in range(1, numEntries):
        persistentTree.GetEntry(i)
        stats.merge(persistentTree.ProcessStatistics)

    total_sum = 0.0
    statistics = {}

    mt = ['ParticleLoader', 'ParticleCombiner', 'ParticleVertexFitter', 'MCMatch', 'TMVAExpert', 'Other']

    for m in stats.getAll():
        modtype = 'Other'
        listname = 'Other'
        splitted = m.getName().split('_')
        if len(splitted) == 1:
            continue
        if splitted[0] in mt:
            modtype = splitted[0]
            if modtype == 'MCMatch':
                modtype = 'MCMatching'
        if 'applyCuts' in m.getName():
            listname = m.getName().split('_', 2)[2]
        else:
            listname = m.getName().split('_', 1)[1].split(' ==>')[0]
        if listname not in statistics:
            statistics[listname] = {}
        if modtype not in statistics[listname]:
            statistics[listname][modtype] = 0.0
        statistics[listname][modtype] += m.getTimeSum(m.c_Event) / 1e9
        total_sum += m.getTimeSum(m.c_Event) / 1e9

    return statistics, total_sum


def getMVARankings(obj):
    """
    Load TMVA variable rankings.
        @param obj object dumped by Summary provider of FEI
        @return dictionary(mother, dictionary(channel, list of tuple(variable, importance))
    """
    result = {}
    for channel, mother, filename in zip(obj['cnames'], obj['mothers'], obj['training_data']):
        if mother not in result:
            result[mother] = {}
        if filename is None:
            result[mother][channel] = []
            continue
        logfile = filename[:-5] + '.log'
        ranking = []
        ranking_mode = 0
        with open(logfile, 'r') as f:
            for line in f:
                if 'Variable Importance' in line:
                    ranking_mode = 1
                elif ranking_mode == 1:
                    ranking_mode = 2
                elif ranking_mode == 2 and '-------' in line:
                    ranking_mode = 0
                elif ranking_mode == 2:
                    v = line.split(':')
                    if int(v[1]) - 1 != len(ranking):
                        B2WARNING("Error during read out of TMVA ranking from " + logfile)
                    oldname = Belle2.Variable.invertMakeROOTCompatible(v[2].strip())
                    ranking.append((oldname, float(v[3])))
        result[mother][channel] = ranking
    return result


def getDetectorEfficiencies(obj, mc_counts):
    result = {}
    result['__values__'] = {}
    result['__errors__'] = {}
    for channel, mother, (filename, key) in zip(obj['cnames'], obj['mothers'], obj['pre_cut_histograms']):
        if filename is None:
            efficiency, purity = 0.0, 0.0
            signal, bckgrd = 0.0, 0.0
        else:
            mc_count = mc_counts[abs(pdg.from_name(mother.split(':')[0]))]
            f = ROOT.TFile(filename)
            withoutCut = f.GetKey('withoutCut' + key).ReadObj()
            signal = withoutCut.GetBinContent(2)
            bckgrd = withoutCut.GetBinContent(3)  # changed here 2 to 3... TODO: test if right!
            efficiency = signal / mc_count
            purity = signal / (signal + bckgrd) if signal > 0 else 0.0
            eff_error = b2stat.binom_error(signal * 1.0, mc_count)
            pur_error = b2stat.binom_error(signal * 1.0, signal + bckgrd)

        if mother not in result['__values__']:
            result['__values__'][mother] = {'__total__': (0.0, 0.0)}
            result['__errors__'][mother] = {}
        result['__values__'][mother][channel] = (efficiency, purity)
        result['__errors__'][mother][channel] = (eff_error, pur_error)
        old_sig = result['__values__'][mother]['__total__'][0]
        old_bkg = result['__values__'][mother]['__total__'][1]
        result['__values__'][mother]['__total__'] = (old_sig + signal, old_bkg + bckgrd)

    for mother in result['__values__']:
        total = mc_counts[abs(pdg.from_name(mother.split(':', 1)[0]))]
        signal, bckgrd = result['__values__'][mother]['__total__']
        result['__values__'][mother]['__total__'] = (signal / total, signal / (signal + bckgrd) if signal > 0 else 0.0)
        total_eff_error = b2stat.binom_error(signal * 1.0, total)
        total_pur_error = b2stat.binom_error(signal * 1.0, signal + bckgrd)
        result['__errors__'][mother] = {'__total__': (total_eff_error, total_pur_error)}
    return result


def getUserCutEfficiencies(obj, mc_counts):
    result = {}
    result['__values__'] = {}
    result['__errors__'] = {}
    for channel, mother, (filename, key) in zip(obj['cnames'], obj['mothers'], obj['pre_cut_histograms']):
        if filename is None:
            efficiency, purity = 0.0, 0.0
            signal, bckgrd = 0.0, 0.0
        else:
            mc_count = mc_counts[abs(pdg.from_name(mother.split(':')[0]))]
            f = ROOT.TFile(filename)
            signal_hist = f.GetKey('signal' + key).ReadObj()
            all_hist = f.GetKey('all' + key).ReadObj()
            signal = calcSum(signal_hist)
            all = calcSum(all_hist)
            efficiency = signal / mc_count
            purity = signal / all if all > 0 else 0.0
            eff_error = b2stat.binom_error(signal * 1.0, mc_count)
            pur_error = b2stat.binom_error(signal * 1.0, all)

        if mother not in result['__values__']:
            result['__values__'][mother] = {'__total__': (0.0, 0.0)}
            result['__errors__'][mother] = {}
        result['__values__'][mother][channel] = (efficiency, purity)
        result['__errors__'][mother][channel] = (eff_error, pur_error)
        old_sig = result['__values__'][mother]['__total__'][0]
        old_all = result['__values__'][mother]['__total__'][1]
        result['__values__'][mother]['__total__'] = (old_sig + signal, old_all + all)

    for mother in result['__values__']:
        total = mc_counts[abs(pdg.from_name(mother.split(':', 1)[0]))]
        signal, all = result['__values__'][mother]['__total__']
        result['__values__'][mother]['__total__'] = (signal / total, signal / all if all > 0 else 0.0)
        total_eff_error = b2stat.binom_error(signal * 1.0, total)
        total_pur_error = b2stat.binom_error(signal * 1.0, all)
        result['__errors__'][mother] = {'__total__': (total_eff_error, total_pur_error)}
    return result


def getPostCutEfficiencies(obj, mc_counts, list_counts):
    result = {}
    result['__values__'] = {}
    result['__errors__'] = {}
    for particle, ntuple_filename in zip(obj['particles'], obj['ntuples']):
        if ntuple_filename is None:
            efficiency, purity, unique_efficiency, unique_purity = (0.0, 0.0, 0.0, 0.0)
        else:
            root_file = ROOT.TFile(ntuple_filename)
            ntuple = root_file.Get('variables')

            mc_count = mc_counts[abs(pdg.from_name(particle.name))]
            signal_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 1'))
            bckgrd_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 0'))
            efficiency = signal_count * 100. / mc_count
            eff_error = b2stat.binom_error(signal_count * 1.0, mc_count) * 100.0
            purity = signal_count * 100. / (signal_count + bckgrd_count)
            pur_error = b2stat.binom_error(signal_count * 1.0, signal_count * 1.0 + bckgrd_count) * 100.0

            # TODO Background can also be counted twice! uniqueBackground Tag?
            unique_signal_count = int(GetEntriesSafe(ntuple,
                                                     particle.mvaConfig.target + ' == 1 && extraInfo__bouniqueSignal__bc == 1'))
            unique_bckgrd_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 0'))
            unique_efficiency = unique_signal_count * 100. / mc_count
            unique_eff_error = b2stat.binom_error(unique_signal_count, mc_count) * 100.0
            unique_purity = unique_signal_count * 100. / (unique_signal_count + unique_bckgrd_count)
            unique_pur_error = b2stat.binom_error(unique_signal_count, unique_signal_count + unique_bckgrd_count) * 100.0

        result['__values__'][particle.identifier] = (efficiency, purity, unique_efficiency, unique_purity)
        result['__errors__'][particle.identifier] = (eff_error, pur_error, unique_eff_error, unique_pur_error)
    return result


def getPreCutEfficiencies(obj, mc_counts, list_counts):
    result = {}
    result['__values__'] = {}
    result['__errors__'] = {}
    for listname, channel, mother in zip(obj['mlists'], obj['cnames'], obj['mothers']):
        if listname is None or listname not in list_counts:
            efficiency, purity = (0.0, 0.0)
            signal, all = 0, 0
        else:
            total = mc_counts[abs(pdg.from_name(mother.split(':', 1)[0]))]
            signal = list_counts[listname]['Signal']
            all = list_counts[listname]['All']
            efficiency = signal * 100. / total
            purity = signal * 100. / all if all > 0 else 0.0
            eff_error = b2stat.binom_error(signal * 1.0, total) * 100.0
            pur_error = b2stat.binom_error(signal * 1.0, all) * 100.0

        if mother not in result['__values__']:
            result['__values__'][mother] = {'__total__': (0.0, 0.0)}
            result['__errors__'][mother] = {}
        result['__values__'][mother][channel] = (efficiency, purity)
        result['__errors__'][mother][channel] = (eff_error, pur_error)
        old_sig = result['__values__'][mother]['__total__'][0]
        old_all = result['__values__'][mother]['__total__'][1]
        result['__values__'][mother]['__total__'] = (old_sig + signal, old_all + all)

    for mother in result['__values__']:
        total = mc_counts[abs(pdg.from_name(mother.split(':', 1)[0]))]
        signal, all = result['__values__'][mother]['__total__']
        result['__values__'][mother]['__total__'] = (signal / total, signal / all if all > 0 else 0.0)
        total_eff_error = b2stat.binom_error(signal * 1.0, total) * 100.0
        total_pur_error = b2stat.binom_error(signal * 1.0, all) * 100.0
        result['__errors__'][mother] = {'__total__': (total_eff_error, total_pur_error)}
    return result


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


class HashableDict(dict):
    """
    An hashable version of a python dict,
    Since we use this only as a trick so the lru cache of getCoveredBranchingFraction works,
    we return a dummy hash (always 1) and not a real hash.
    """

    def __hash__(self):
        return 1


@functools.lru_cache(maxsize=3)
def loadBranchingFractions(filename=None):
    """
    Load branching fraction from MC decay-file.
    Columns are, particle, channel, fraction
        @param filename of the decay file default is $BELLE2_EXTERNALS_DIR/share/evtgen/DECAY.DEC
    """
    if filename is None:
        filename = os.getenv('BELLE2_EXTERNALS_DIR') + '/share/evtgen/DECAY.DEC'

    # Add some theoretical branching fractions which are not in the DECAY file
    # TODO But these are produced anyway, wtf?
    branching_fractions = {'D0': {tuple(sorted(('K-', 'pi+', 'pi0', 'pi0'))): 0.0},  # UNKOWN
                           'anti-D0': {tuple(sorted(('K+', 'pi-', 'pi0', 'pi0'))): 0.0},  # UNKOWN
                           'D_s+': {tuple(sorted(('K-', 'K_S0', 'pi+', 'pi+'))): 0.0164},  # From PDG
                           'D_s+': {tuple(sorted(('K_S0', 'pi+', 'pi0'))): 0.005},  # Mode D_s->K0 pi- pi0 1%
                           'D_s-': {tuple(sorted(('K+', 'K_S0', 'pi-', 'pi-'))): 0.0164},  # From PDG
                           'D_s-': {tuple(sorted(('K_S0', 'pi-', 'pi0'))): 0.005},  # Mode D_s->K0 pi- pi0 1%
                           'B+': {tuple(sorted(('J/psi', 'K_S0', 'pi+'))): 0.00094},
                           'B-': {tuple(sorted(('J/psi', 'K_S0', 'pi-'))): 0.00094},
                           'B0': {tuple(sorted(('J/psi', 'K_S0', 'pi+', 'pi-'))): 0.001},
                           'anti-B0': {tuple(sorted(('J/psi', 'K_S0', 'pi+', 'pi-'))): 0.001},
                           'K_S0': {('K_S0',): 0.692}}
    branching_fractions = HashableDict(branching_fractions)
    mother = 'UNKOWN'
    with open(filename, 'r') as f:
        for line in f:
            fields = line.split(' ')
            fields = [x for x in fields if x != '']
            if len(fields) < 2 or fields[0][0] == '#':
                continue
            if fields[0] == 'Decay':
                mother = fields[1].strip()
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
                branching_fractions[mother] = HashableDict()
            if daughters not in branching_fractions[mother]:
                branching_fractions[mother][daughters] = 0.0
            branching_fractions[mother][daughters] += float(fields[0])

    return branching_fractions


def conjugate(particle):
    p = particle.split(':')
    if len(p) > 1:
        return pdg.conjugate(p[0]) + ':' + p[1]
    else:
        return pdg.conjugate(p[0])


@functools.lru_cache(maxsize=1000)
def getCoveredBranchingFraction(particle, branching_fractions):
    """
    Get covered branching fraction of a particle using a recursive algorithm
    and the given exclusive branching_fractions (given as Hashable List)
    @param particle identifier of the particle
    @param branching_fractions
    """
    if particle not in branching_fractions:
        if conjugate(particle) not in branching_fractions:
            return 1.0
        else:
            particle = conjugate(particle)

    total_covered = 0.0
    for channel, fraction in branching_fractions[particle].items():
        covered = fraction
        for daughter in channel:
            if particle.split(':')[0] == daughter.split(':')[0]:
                continue
            covered *= getCoveredBranchingFraction(daughter, branching_fractions)
        total_covered += covered
    return total_covered


def getCoveredBranchingFractions(obj):
    """
    Load covered branching fractions from MC decay-file and given particle definitions.
    Columns are, particle, channel, fraction
        @param particles list of Particle objects
    """
    particles = obj['particles']
    mc_branching_fractions = loadBranchingFractions()
    branching_fractions = HashableDict()
    for particle in particles:
        branching_fractions[particle.identifier] = {}
        for channel in particle.channels:
            fraction = 0.0
            daughters = tuple(daughter.split(':')[0] for daughter in channel.daughters)
            if particle.name in mc_branching_fractions:
                if tuple(sorted(daughters)) in mc_branching_fractions[particle.name]:
                    fraction = mc_branching_fractions[particle.name][tuple(sorted(daughters))]
            elif pdg.conjugate(particle.name) in mc_branching_fractions:
                if tuple(sorted(map(pdg.conjugate, daughters))) in mc_branching_fractions[pdg.conjugate(particle.name)]:
                    fraction = mc_branching_fractions[pdg.conjugate(particle.name)][tuple(sorted(map(pdg.conjugate, daughters)))]
            else:
                fraction = 1.0
            if any(daughter.endswith(':V0') for daughter in channel.daughters):
                fraction = 0.0
            if fraction == 0.0:
                print('WARNING: Branching fraction for {p} {ch} is zero'.format(
                    p=particle.identifier,
                    ch=str(tuple(sorted(channel.daughters)))))
            branching_fractions[particle.identifier][tuple(sorted(channel.daughters))] = fraction

    covered = {}
    for particle in particles:
        covered[particle.identifier] = getCoveredBranchingFraction(particle.identifier, branching_fractions)
    return covered


def pretty_format(obj, indent=0):
    """
    Pretty-prints the object passed in.
    """
    return pprint.pformat(obj)


if __name__ == '__main__':

    if len(sys.argv) != 2:
        print("Usage: " + sys.argv[0] + ' SummaryFile.pickle')
        sys.exit(1)

    obj = pickle.load(open(sys.argv[1], 'rb'))

    mc_counts = getMCCounts(obj)
    print('MC Counts')
    print(pretty_format(mc_counts))

    coveredBranchingFractions = getCoveredBranchingFractions(obj)
    print('Covered Branching Fractions')
    print(pretty_format(coveredBranchingFractions))

    module_statistics, total_sum = getModuleStatistics(obj)
    print('Module statistics')
    print(pretty_format(module_statistics))
    print('Total time spent (all modules): {}'.format(total_sum))
    print('Total time spent (only monitored modules): {}'.format(sum(m for s in module_statistics.values() for m in s.values())))

    rankings = getMVARankings(obj)
    print('MVA Ranking')
    print(pretty_format(rankings))

    detectorEfficiencies = getDetectorEfficiencies(obj, mc_counts)['__values__']
    print('DetectorEfficiencies')
    print(pretty_format(detectorEfficiencies))

    userCutEfficiencies = getUserCutEfficiencies(obj, mc_counts)['__values__']
    print('UserCutEfficiencies')
    print(pretty_format(userCutEfficiencies))

    list_counts = getListCounts(obj)
    preCutEfficiencies = getPreCutEfficiencies(obj, mc_counts, list_counts)['__values__']
    print('PreCutEfficiencies')
    print(pretty_format(preCutEfficiencies))

    postCutEfficiencies = getPostCutEfficiencies(obj, mc_counts, list_counts)['__values__']
    print('PostCutEfficiencies')
    print(pretty_format(postCutEfficiencies))

    for particle, ntuple_filename in zip(obj['particles'], obj['ntuples']):
        createParticlePlots(particle, ntuple_filename, mc_counts)
