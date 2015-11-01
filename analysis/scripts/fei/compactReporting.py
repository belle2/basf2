#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT
from ROOT import Belle2
import variables
import pdg
from fei import *
from B2Tools import format

import pickle
import sys
import math


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


def calcSum(hist):
    return sum(hist.GetXaxis().GetBinCenter(bin + 1) * hist.GetBinContent(bin + 1) for bin in range(hist.GetNbinsX()))


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
        common = 'extraInfo__bouniqueSignal__bc == 1 && Mbc > 5.23 && extraInfo__boSignalProbability__bc > '
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
    ntuple.Project('ROCuniqueBackground', probabilityVar, '!' + targetVar + '  && extraInfo__bouniqueSignal__bc == 1')
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


def getMCCounts(obj):
    root_file = ROOT.TFile(obj['mc_counts'])
    mc_counts = {}
    for key in root_file.GetListOfKeys():
        variable = Belle2.Variable.invertMakeROOTCompatible(key.GetName())
        pdgcode = int(variable[len('NumberOfMCParticlesInEvent('):-len(")")])
        count = calcSum(key.ReadObj())
        mc_counts[pdgcode] = count
    return mc_counts


def getListCounts(obj):
    root_file = ROOT.TFile(obj['list_counts'])
    list_counts = {}
    for key in root_file.GetListOfKeys():
        variable = Belle2.Variable.invertMakeROOTCompatible(key.GetName())
        argument = variable[len('countInList('):-1]
        listname = argument.split(',', 1)[0]
        mode = 'All' if ',' not in argument else 'Signal' if '1' == argument[-1] else 'Background'

        if listname not in list_counts:
            list_counts[listname] = {}
        list_counts[listname][mode] = calcSum(key.ReadObj())
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

    statistics = {}
    for m in stats.getAll():
        modtype = 'Other'
        listname = 'Other'
        for mt in ['ParticleLoader', 'ParticleCombiner', 'ParticleVertexFitter', 'MCMatching', 'TMVAExpert', 'Other']:
            splitted = m.getName().split('_')
            if splitted[0] in mt:
                modtype = mt
                listname = splitted[1]
        if listname not in statistics:
            statistics[listname] = {}
        if modtype not in statistics[listname]:
            statistics[listname][modtype] = 0.0
        statistics[listname][modtype] += m.getTimeSum(m.c_Event) / 1e9
    return statistics


if __name__ == '__main__':

    if len(sys.argv) != 3:
        print("Usage: " + sys.argv[0] + ' SummaryFile.pickle OutputFile.txt')
        sys.exit(1)

    obj = pickle.load(open(sys.argv[1], 'rb'))

    out = open(sys.argv[2], 'w')

    print("Process MC Counts")
    mc_counts = getMCCounts(obj)

    print("Process List Counts")
    list_counts = getListCounts(obj)

    print("Process Module Statistics")

    out.write('\n\n# Module CPU time statistics: \n')
    module_statistics = getModuleStatistics(obj)
    sum_time_seconds = sum(m for s in module_statistics.values() for m in s.values())
    moduleTypes = list(set(m for s in module_statistics.values() for m in s.keys()))
    list_to_channel = {l: c for l, c in zip(obj['mlists'], obj['cnames'])}
    lines = []
    for listname, stat in module_statistics.items():
        if listname not in list_counts:
            continue
        trueCandidates = list_counts[listname]['Signal']
        allCandidates = list_counts[listname]['All']

        time = sum(m for m in stat.values())
        timePerCandidate = format.duration(time / trueCandidates) + ' (' + format.duration(time / allCandidates) + ')'
        timePercent = time / sum_time_seconds * 100 if sum_time_seconds > 0 else 0

        lines.append('{channel}: {time} {timePerCandidate} {timePercent:.2f}\n'.format(
                        time=time,
                        channel=list_to_channel[listname],
                        timePerCandidate=timePerCandidate,
                        timePercent=timePercent
        ))
    out.write(''.join(sorted(lines)))

    out.write('# Efficiencies and Purities for compact-FEI-summary \n')
    print("Process list counts for individual channels")

    out.write('\n\n# Individual channels: \n')
    lines = []
    for listname, channel, mother in zip(obj['mlists'], obj['cnames'], obj['mothers']):
        if listname is None:
            lines.append('{mother:<20} - {channel:<20}: Was ignored'.format(mother=mother, channel=channel))
            continue
        if listname not in list_counts:
            lines.append('{mother:<20} - {channel:<20}: Was ignored'.format(mother=mother, channel=channel))
            continue
        lines.append('{mother:<20} - {channel:<20}: {eff:<20} {pur:<20}\n'.format(
                     mother=mother,
                     channel=channel,
                     eff=list_counts[listname]['Signal'] * 100. / mc_counts[abs(pdg.from_name(mother.split(':', 1)[0]))],
                     pur=list_counts[listname]['Signal'] * 100. / list_counts[listname]['All']))
    out.write(''.join(sorted(lines)))

    print("Process ntuples for all Particles")
    out.write('\n\n# Particles: \n')

    for particle, ntuple_filename in zip(obj['particles'], obj['ntuples']):
        if ntuple_filename is None:
            out.write(particle.identifier + ' was ignored\n')
            continue

        root_file = ROOT.TFile(ntuple_filename)
        ntuple = root_file.Get('variables')

        mc_count = mc_counts[abs(pdg.from_name(particle.name))]
        signal_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 1'))
        bckgrd_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 0'))
        unique_signal_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 1 && extraInfo__bouniqueSignal__bc == 1'))
        unique_bckgrd_count = int(GetEntriesSafe(ntuple, particle.mvaConfig.target + ' == 0 && extraInfo__bouniqueSignal__bc == 1'))

        line = '{part:<20}: {peff:<20} {pueff:<20} | {ppur:<20} {pupur:<20}\n'.format(
            part=particle.identifier,
            peff=signal_count * 100. / mc_count,
            ppur=signal_count * 100. / (signal_count + bckgrd_count),
            pueff=unique_signal_count * 100. / mc_count,
            pupur=unique_signal_count * 100. / (unique_signal_count + unique_bckgrd_count))
        out.write(line)
        makeROCPlotFromNtuple(ntuple,
                              removeJPsiSlash(particle.name) + '_' + particle.label + '_roc.png',
                              mc_count,
                              particle.mvaConfig.target)
        if particle.identifier in ['B+:generic', 'B0:generic']:
            makeMbcPlot(ntuple,
                        removeJPsiSlash(particle.name) + '_' + particle.label + '_money.png',
                        particle.mvaConfig.target)
        if particle.identifier in ['B+:semileptonic', 'B0:semileptonic']:
            makeCosBDLPlot(root_file,
                           removeJPsiSlash(particle.name) + '_' + particle.label + '_money.png',
                           particle.mvaConfig.target)

    print("Process cut configuration")
    out.write('\n\n# Indivual cuts of particles \n')
    for particle, precut, postcut in zip(obj['particles'], obj['pre_cuts'], obj['post_cuts']):
        if precut is None or postcut is None:
            out.write(particle.identifier + ' was ignored\n')
            continue
        out.write(particle.identifier + ' PostCut ' + postcut['cutstring'] + '\n')
        for channel, cut in precut.items():
            if cut is None:
                out.write('\t' + channel + ' was ignored\n')
            else:
                out.write('\t' + channel + ' ' + cut['cutstring'] + '\n')

    print("Finished")
    out.close()
