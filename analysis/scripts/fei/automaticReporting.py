#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import pdg

from B2Tools import b2plot
from B2Tools import b2stat
from B2Tools import b2latex
from B2Tools import format

from fei import preCutDetermination
import os
import re
import sys
import subprocess
import copy
import itertools
import numpy

from ROOT import gSystem
gSystem.Load('libanalysis.so')
from ROOT import Belle2


def loadMCCountsDataFrame(filename):
    """
    Load MCCounts into a pandas.DataFrame.
    Columns are named by the absolute value of the PDG code of each particle.
        @param filename name of the ROOT file containing the MCCounts
        @return pandas.DataFrame with MCCounts
    """
    from root_pandas import read_root
    df = read_root(filename, 'mccounts', columns='*')

    def rename(name):
        old_name = Belle2.Variable.invertMakeROOTCompatible(name)
        return old_name[len('NumberOfMCParticlesInEvent('):-len(")")]

    df.columns = map(rename, df.columns)
    return df


def loadListCountsDataFrame(filename):
    """
    Load ListCounts into a pandas.DataFrame.
    Columns are named by the MatchedParticleList name plus the suffixes _Signal, _Background, _All.
        @param filename name of the ROOT file containing the ListCounts
        @return pandas.DataFrame with ListCounts
    """
    from root_pandas import read_root
    df = read_root(filename, 'listcounts', columns='*')

    def rename(name):
        old_name = Belle2.Variable.invertMakeROOTCompatible(name)
        if ',' in old_name:
            if old_name[-2] == '1':
                return old_name.split(',')[0][len('countInList('):] + '_Signal'
            if old_name[-2] == '0':
                return old_name.split(',')[0][len('countInList('):] + '_Background'
            raise RuntimeError("Given listCount name is not valid " + old_name)
        else:
            return old_name[len('countInList('):-1] + '_All'

    df.columns = map(rename, df.columns)
    return df


def loadModuleStatisticsDataFrame(filename):
    """
    Load ModuleStatistics into a pandas.DataFrame.
    Columns are named name, time and type.
        @param filename name of the ROOT file containing the ModuleStatistics
        @return pandas.DataFrame with ModuleStatistics
    """
    import ROOT
    tfile = ROOT.TFile(filename)
    persistentTree = tfile.Get('persistent')
    persistentTree.GetEntry(0)
    # Clone() needed so we actually own the object (original dies when tfile is deleted)
    stats = persistentTree.ProcessStatistics.Clone()

    # merge statistics from all persistent trees into 'stats'
    numEntries = persistentTree.GetEntriesFast()
    for i in range(1, numEntries):
        persistentTree.GetEntry(i)
        stats.merge(persistentTree.ProcessStatistics)

    import pandas
    statistics = pandas.DataFrame(columns=['name', 'listname', 'time', 'type'])
    for m in stats.getAll():
        modtype = 'Other'
        listname = 'Other'
        for mt in ['ParticleLoader', 'ParticleCombiner', 'ParticleVertexFitter', 'MCMatching', 'TMVAExpert', 'Other']:
            splitted = m.getName().split('_')
            if splitted[0] in mt:
                modtype = mt
                listname = splitted[1]
        statistics = statistics.append([dict(name=m.getName(),
                                             type=modtype,
                                             listname=listname,
                                             time=m.getTimeSum(m.c_Event) / 1e9)],
                                       ignore_index=True)
    return statistics


def loadNTupleDataFrame(filename):
    """
    Load NTuple data of a Particle into a pandas.DataFrame.
    Columns are named by the original variable names written into the NTuple.
        @param filename name of the ROOT file containing the NTuple
        @return pandas.DataFrame with NTuple data
    """
    from root_pandas import read_root
    df = read_root(filename, 'variables', columns='*')

    def rename(name):
        return Belle2.Variable.invertMakeROOTCompatible(name)

    df.columns = map(rename, df.columns)
    return df


def loadTMVADataFrame(filename):
    """
    Load TMVA training and test data into a pandas.DataFrame.
    Columns are named by the original variable names used in the training.
        @param filename name of the ROOT file containing the TMVA data
        @return pandas.DataFrame with TMVA data
    """
    from root_pandas import read_root
    train_df = read_root(filename, 'TrainTree', columns='*')
    test_df = read_root(filename, 'TestTree', columns='*')

    def rename(name):
        return Belle2.Variable.invertMakeROOTCompatible(name)

    train_df.columns = map(rename, train_df.columns)
    test_df.columns = map(rename, test_df.columns)
    train_df['__isTrain__'] = True
    test_df['__isTrain__'] = False
    df = train_df.append(test_df, ignore_index=True)
    df['__isSignal__'] = df['className'] == "Signal"
    return df


def loadMVARankingDataFrame(logfile):
    """
    Load TMVA variable ranking into a pandas.DataFrame.
    Columns are named name (of the original variable) and importance.
        @param filename name of the log file produced by the TMVA training
        @return pandas.DataFrame with ranking
    """
    import pandas
    ranking = pandas.DataFrame(columns=['name', 'importance'])
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
                ranking = ranking.append([{'name': oldname, 'importance': float(v[3])}], ignore_index=True)
    return ranking


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


def loadBranchingFractionsDataFrame(filename=None):
    """
    TODO In Python >=3.2 we can use functools.lru_cache here for speedup
    Load branching fraction frm MC decay-file.
    Columns are, particle, channel, fraction
        @param filename of the decay file default is $BELLE2_EXTERNALS_DIR/share/evtgen/DECAY.DEC
        @return pandas.DataFrame with the branching fractions
    """
    import pandas
    if filename is None:
        filename = os.getenv('BELLE2_EXTERNALS_DIR') + '/share/evtgen/DECAY.DEC'

    branching_fractions = pandas.DataFrame(columns=['particle', 'channel', 'fraction'])
    mother = 'UNKOWN'
    with open(filename, 'r') as f:
        for line in f:
            fields = line.split(' ')
            fields = filter(lambda x: x != '', fields)
            if len(fields) < 2:
                continue
            if fields[0][0] == '#':
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
            if len(fields) < 1:
                continue
            if not isFloat(fields[0]):
                continue
            while len(fields) > 1:
                if not isValidParticle(fields[-1]):
                    fields = fields[:-1]
                else:
                    break
            if len(fields) < 1:
                continue
            if not all(isValidParticle(p) for p in fields[1:]):
                continue
            daughters = tuple(sorted(p for p in fields[1:]))
            row = [{'particle': mother, 'channel': daughters, 'fraction': float(fields[0])}]
            branching_fractions = branching_fractions.append(row, ignore_index=True)

    # Add some theoretical branching fractions which are not in the DECAY file
    # TODO But these are produced anyway, wtf?
    rows = [{'particle': 'D0', 'channel': tuple(sorted(('K-', 'pi+', 'pi0', 'pi0'))), 'fraction': 0.0},  # UNKOWN
            {'particle': 'anti-D0', 'channel': tuple(sorted(('K+', 'pi-', 'pi0', 'pi0'))), 'fraction': 0.0},  # UNKOWN
            {'particle': 'D_s+', 'channel': tuple(sorted(('K-', 'K_S0', 'pi+', 'pi+'))), 'fraction': 0.0164},  # From PDG
            {'particle': 'D_s+', 'channel': tuple(sorted(('K_S0', 'pi+', 'pi0'))), 'fraction': 0.005},  # Mode D_s->K0 pi- pi0 1%
            {'particle': 'D_s-', 'channel': tuple(sorted(('K+', 'K_S0', 'pi-', 'pi-'))), 'fraction': 0.0164},  # From PDG
            {'particle': 'D_s-', 'channel': tuple(sorted(('K_S0', 'pi-', 'pi0'))), 'fraction': 0.005},  # Mode D_s->K0 pi- pi0 1%
            {'particle': 'B+', 'channel': tuple(sorted(('J/psi', 'K_S0', 'pi+'))), 'fraction': 0.00094},
            {'particle': 'B-', 'channel': tuple(sorted(('J/psi', 'K_S0', 'pi-'))), 'fraction': 0.00094},
            {'particle': 'B0', 'channel': tuple(sorted(('J/psi', 'K_S0', 'pi+', 'pi-'))), 'fraction': 0.001},
            {'particle': 'anti-B0', 'channel': tuple(sorted(('J/psi', 'K_S0', 'pi+', 'pi-'))), 'fraction': 0.001}]
    branching_fractions = branching_fractions.append(rows, ignore_index=True)
    return branching_fractions


def loadCoveredBranchingFractionsDataFrame(particles, filename=None, include_daughter_fractions=True):
    """
    Load covered branching fractions from MC decay-file and given particle definitions.
    Columns are, particle, channel, fraction
        @param particles list of Particle objects
        @param filename of the decay file default is $BELLE2_EXTERNALS_DIR/share/evtgen/DECAY.DEC
        @param include_daughter_fractions if true the branching fraction is multiplied with the branching fraction of the daughters
        @return pandas.DataFrame with the branching fractions
    """
    import pandas
    mc = loadBranchingFractionsDataFrame(filename).groupby(['particle', 'channel'])['fraction'].sum()
    covered = pandas.DataFrame(columns=['particle', 'channel', 'channelName', 'fraction'])
    covered = covered.append([{'particle': 'nu_e:generic', 'channel': (), 'channelName': 'nu_e:generic', 'fraction': 1.0},
                              {'particle': 'nu_mu:generic', 'channel': (), 'channelName': 'nu_mu:generic', 'fraction': 1.0},
                              {'particle': 'nu_tau:generic', 'channel': (), 'channelName': 'nu_tau:generic', 'fraction': 1.0}],
                             ignore_index=True)
    remaining = []
    for particle in particles:
        if particle.isFSP:
            d = [{'particle': particle.identifier, 'channel': tuple(), 'channelName': particle.identifier, 'fraction': 1.0}]
            covered = covered.append(d, ignore_index=True)
        else:
            for channel in particle.channels:
                mychannel = [daughter for daughter in channel.daughters]
                for i, j in [('e+', 'e-'), ('mu+', 'mu-'), ('tau+', 'tau-')]:
                    m = 0
                    for daughter in channel.daughters:
                        if daughter.startswith(i):
                            m += 1
                        if daughter.startswith(j):
                            m -= 1
                    if m > 0:
                        mychannel += ['nu_' + i[:-1] + ':generic'] * m
                    elif m < 0:
                        mychannel += ['anti-nu_' + i[:-1] + ':generic'] * (-m)
                remaining.append((particle.identifier, tuple(sorted(mychannel)), channel.name))

    def conjugate(particle):
        n, l = particle.split(':')
        return pdg.conjugate(n) + ':' + l

    while remaining:
        old_remaining = remaining
        remaining = []
        blacklist = [name for name, _, _ in old_remaining]
        for name, channel, channelName in old_remaining:
            if all(daughter not in blacklist and conjugate(daughter) not in blacklist for daughter in channel):
                rawname = name.split(':')[0]
                rawchannel = tuple(sorted([n.split(':')[0] for n in channel]))
                try:
                    fraction = mc[rawname][rawchannel]
                except KeyError:
                    B2WARNING("Monte Carlo branching fraction for " + rawname +
                              " and channel " + str(channel) + " is not available. Assume 0.")
                    fraction = 0.0
                if include_daughter_fractions:
                    for daughter in channel:
                        mask = (covered.particle == daughter) | (covered.particle == conjugate(daughter))
                        fraction *= covered[mask]['fraction'].sum()
                d = [{'particle': name, 'channel': channel, 'channelName': channelName, 'fraction': fraction}]
                covered = covered.append(d, ignore_index=True)
            else:
                remaining.append((name, channel, channelName))

    return covered


def createUniqueFilename(name, hash, suffix='png'):
    """
    Create a unique filename, which is save to use.
    In particular it removes the symbol / which is introduced by J/Psi
        @param name often the raw_name of the particle or channel
        @param hash often provided by resource.hash
        @param suffix defaults to png
        @return string unique and save to use filename
    """
    filename = '{name}_{hash}.{suffix}'.format(name=name, hash=hash, suffix=suffix)
    return filename.replace('/', '').replace(':', '').replace(' ', '').replace('=', '').replace('>', '')


def createSummary(resource, finalStateSummaries, combinedSummaries, particles, mcCounts, listCounts, moduleStatisticsFile):
    """
    Creates combined summary .tex file for FEIR
        @param resource object
        @param finalStateSummaries list of placeholder dictonaries (for each FSP)
        @param combinedSummaries list of placeholder dictonaries (for each combined particle)
        @param particles List of Particle objects
        @param mcCounts filename containing MCParticle counts
        @param listCounts filename containing List counts
        @param moduleStatisticsFilefile name of the TFile containing actual statistics
        @return dictionary of placeholders used in the .tex file
    """

    o = b2latex.LatexFile()

    o += b2latex.TitlePage(title='Full Event Interpretation Report',
                           authors=['Thomas Keck', 'Christian Pulvermacher'],
                           abstract=r"""
                           This report contains key performance indicators and control plots of the Full Event Interpretation.
                           The user-, pre-, and post-cuts as well as trained multivariate selection methods are described.
                           Furthermore the resulting purities and efficiencies are stated.
                              """,
                           add_table_of_contents=True).finish()

    channelSummaries = sum(map(lambda x: x['channels'], combinedSummaries), [])

    o += b2latex.Section("Summary").finish()
    o += b2latex.String(r"""
        For each final-state particle a multivariate selection method is trained without any previous cuts
        on the candidates. Afterwards, a post cut is applied on the signal probability calculated by the method.
        This reduces combinatorics in the following stages of the Full
        Event Interpretation.
        """).finish()

    table = b2latex.LongTable(columnspecs=r'c|rr|rr',
                              caption='Final-state particle efficiency and purity before and after the applied post-cut.',
                              head=r'Final-state &  \multicolumn{2}{c}{Efficiency in \%}  &  \multicolumn{2}{c}{Purity in \%} \\' +
                                   r' particle    &  detector & post-cut   &  detector & post-cut \\',
                              format_string=r'{name} & {user_efficiency:.2f} & {post_efficiency:.2f}'
                                            r'& {user_purity:.2f} & {post_purity:.2f} ')
    for summary in finalStateSummaries:
        table.add(**summary)
    o += table.finish()

    o += b2latex.String(r"""
        For each decay channel of each intermediate particle a multivariate selection method is trained after applying
        a fast pre-cut on the candidates. Afterwards, a post-cut is applied on the signal probability calculated by the method.
        This reduces combinatorics in the following stages of the Full
        Event Interpretation. For some intermediate particles, in particular the final B mesons, an additional user-cut
        is applied before all other cuts.
        """).finish()

    table = b2latex.LongTable(columnspecs=r'c|r|rrrr|rrrr',
                              caption='Per-particle efficiency and purity before and after the applied user-, pre- and post-cut.',
                              head=r'Particle & Covered BR in \%'
                                   r' & \multicolumn{4}{c}{Efficiency in \%}  &  \multicolumn{4}{c}{Purity in \%} \\'
                                   r' & &  recon. & user-cut & pre-cut & post-cut '
                                   r' & recon. & user-cut & pre-cut & post-cut \\',
                              format_string=r'{name} & {covered:.2f} & {detector_efficiency:.2f} & {user_efficiency:.2f}'
                                            r' & {pre_efficiency:.2f}  & {post_efficiency:.2f}'
                                            r' & {detector_purity:.2f} & {user_purity:.2f} & {pre_purity:.2f} & {post_purity:.2f}')
    for summary in combinedSummaries:
        table.add(**summary)
    o += table.finish()

    # If you change the number of colors, than change below \ifnum5 accordingly
    colours = ["orange", "blue", "red", "green", "cyan", "purple"]

    o += b2latex.Section("CPU time per channel").finish()

    colour_list = b2latex.DefineColourList()
    o += colour_list.finish()

    listCountsData = loadListCountsDataFrame(listCounts)
    stats = loadModuleStatisticsDataFrame(moduleStatisticsFile)
    sum_time_seconds = stats['time'].sum()
    moduleTypes = list(stats.type.unique())
    stats = stats.groupby(['listname', 'type'])['time'].sum()

    statTable = []
    for summary in itertools.chain(finalStateSummaries, channelSummaries):
        name = summary['name']
        plist = summary['list']
        if plist is None:
            continue
        trueCandidates = listCountsData[plist + '_Signal'].sum()
        allCandidates = listCountsData[plist + '_All'].sum()
        statTable.append([name, stats[plist].sum(), stats[plist], trueCandidates, allCandidates])

    table = b2latex.LongTable(columnspecs=r'lrcrr',
                              caption='Total CPU time spent in event() calls for each channel. Bars show ' +
                                      ', '.join('\\textcolor{%s}{%s}' % (c, m) for c, m in zip(colour_list.colours, moduleTypes)) +
                                      ', in this order. Does not include I/O, initalisation, training, post-cuts etc.',
                              head=r'Decay & CPU time & by module & per (true) candidate & Relative time ',
                              format_string=r'{name} & {time} & {bargraph} & {timePerCandidate} & {timePercent:.2f}\% ')
    for name, time, timePerModule, trueCandidates, allCandidates in statTable:
        percents = tuple(timePerModule[key] / time * 100.0 if key in timePerModule else 0.0 for key in moduleTypes)
        timePerCandidate = format.duration(numpy.float64(time) / trueCandidates)
        timePerCandidate += ' (' + format.duration(numpy.float64(time) / allCandidates) + ')'
        table.add(name=name,
                  bargraph=r'\plotbar{ %g/, %g/, %g/, %g/, %g/, %g/, }' % percents,
                  time=format.duration(time),
                  timePerCandidate=timePerCandidate,
                  timePercent=time / sum_time_seconds * 100 if sum_time_seconds > 0 else 0)
    o += table.finish(tail='Total & & ' + format.duration(sum_time_seconds) + ' & & 100 ')

    for ph in finalStateSummaries:
        o += ph['page']
    for ph in combinedSummaries:
        o += ph['page']

    o.save('FEIsummary.tex', compile=True)
    resource.needed = False
    resource.cache = True
    return


def createTMVASection(filename, tmvaTraining, mvaConfig, plotConfig):
    """
    Create TMVA section, with information about used methods,
    overtraining, ROC and diagonal plots.
        @param filename used as suffix for all plots
        @param tmvaTraining data used to create the plots
        @param mvaConfig used to extract config information
        @param plotConfig which defines which plots are included
    """
    o = b2latex.LatexFile()
    o += b2latex.SubSubSection("MC-based MVA")
    if tmvaTraining is None:
        o += b2latex.String(r"""
                There is no MC-based TMVA training data available for this final state particle.
                Usually this means there wasn't enough statistics to perform a training,
                or the training failed due to other reasons (cpu-, memory-, disk-, limitations?)
                """)
    else:
        TMVAFilename = tmvaTraining[:-7] + '.root'  # Strip .config of filename
        tmvaData = loadTMVADataFrame(TMVAFilename)
        tmvaTrainingData = tmvaData[tmvaData['__isTrain__']]
        tmvaTestData = tmvaData[~tmvaData['__isTrain__']]

        logFilename = tmvaTraining[:-9] + '.log'  # Strip _1.config of filename
        ranking = loadMVARankingDataFrame(logFilename)

        table = b2latex.LongTable(columnspecs=r'lp{5cm}rrr',
                                  caption='List of variables used in the training',
                                  head=r'No. & Name & Importance & \multicolumn{2}{c}{mean $\pm$ std} \\'
                                       r' & & & Signal & Background ',
                                  format_string=r'{no} & {name} & {v:.2f} & $({ms:.3f} \pm {es:.3f})$'
                                                r' & $({mb:.3f} \pm {eb:.3f})$')

        for number, (n, value) in ranking.iterrows():
            table.add(no=number+1,
                      name=format.variable(format.string(n)),
                      v=value,
                      ms=tmvaData[tmvaData['__isSignal__']][n].mean(),
                      es=tmvaData[tmvaData['__isSignal__']][n].std(),
                      mb=tmvaData[~tmvaData['__isSignal__']][n].mean(),
                      eb=tmvaData[~tmvaData['__isSignal__']][n].std())
        for n in mvaConfig.variables:
            if n not in ranking.name.unique():
                table.add(no='---',
                          name=format.variable(format.string(n)),
                          v=float('nan'),
                          ms=float('nan'),
                          es=float('nan'),
                          mb=float('nan'),
                          eb=float('nan'))
        o += table.finish()

        if plotConfig.Correlation:
            correlationMatrixPlotSignal = 'corMSig_' + filename
            correlationMatrixPlotBackground = 'corMBkg_' + filename
            plot = b2plot.CorrelationMatrix()
            plot.add(tmvaData[tmvaData['__isSignal__']], ranking.name.unique())
            plot.finish()
            plot.axis.set_title('Correlation Matrix for Signal')
            plot.save(correlationMatrixPlotSignal)

            plot = b2plot.CorrelationMatrix()
            plot.add(tmvaData[~tmvaData['__isSignal__']], ranking.name.unique())
            plot.finish()
            plot.axis.set_title('Correlation Matrix for Background')
            plot.save(correlationMatrixPlotBackground)
            o += b2latex.Graphics().add(correlationMatrixPlotSignal).add(correlationMatrixPlotBackground).finish()

        caption = 'Method ' + mvaConfig.type + '/' + mvaConfig.name + ' was used with the following configuration '
        caption += format.variable(mvaConfig.config) + r'and with target variable \emph{' + mvaConfig.target + r'}'
        table = b2latex.LongTable(columnspecs=r'lp{5cm}rr',
                                  caption=caption,
                                  head=r'Name & Signal & Background',
                                  format_string=r'{name} & {signal} & {background}')
        table.add(name='Training',
                  signal=numpy.sum(tmvaTrainingData['__isSignal__']),
                  background=numpy.sum(~tmvaTestData['__isSignal__']))
        table.add(name='Test',
                  signal=numpy.sum(tmvaTestData['__isSignal__']),
                  background=numpy.sum(~tmvaTestData['__isSignal__']))
        o += table.finish()

        mvaROCPlot = 'mva_roc_' + filename
        plot = b2plot.RejectionOverEfficiency()
        plot.set_plot_options({'linestyle': '-', 'lw': 3})
        plot.add(tmvaTestData, mvaConfig.name, tmvaTestData['__isSignal__'], ~tmvaTestData['__isSignal__'])
        plot.add(tmvaTrainingData, mvaConfig.name, tmvaTrainingData['__isSignal__'], ~tmvaTrainingData['__isSignal__'])
        plot.labels = ["Test", "Training"]
        plot.finish()
        plot.save(mvaROCPlot)

        mvaDiagPlot = 'mva_diag_' + filename
        plot = b2plot.Diagonal()
        plot.set_plot_options({'linestyle': '-', 'lw': 3})
        plot.add(tmvaTestData, mvaConfig.name, tmvaTestData['__isSignal__'], ~tmvaTestData['__isSignal__'])
        plot.add(tmvaTrainingData, mvaConfig.name, tmvaTrainingData['__isSignal__'], ~tmvaTrainingData['__isSignal__'])
        plot.labels = ["Test", "Training"]
        plot.finish()
        plot.save(mvaDiagPlot)

        mvaOvertrainingPlot = 'mva_overtraining_' + filename
        plot = b2plot.Overtraining()
        plot.set_plot_options({'linestyle': '-', 'lw': 3})
        plot.add(tmvaData, 'FastBDT',
                 tmvaData['__isTrain__'], ~tmvaData['__isTrain__'],
                 tmvaData['__isSignal__'], ~tmvaData['__isSignal__'])
        plot.finish()
        plot.save(mvaOvertrainingPlot)
        o += b2latex.Graphics().add(mvaOvertrainingPlot, width=0.7).add(mvaROCPlot, width=0.7).add(mvaDiagPlot, width=0.7).finish()

    return o.finish()


def createDiagonalPlot(filename, nTupleData, mvaConfig):
    """
    Create diagonal plot and returns latex code embedding the plot
        @param filename used as suffix for the plot
        @param nTupleData used to create the diagonal plot
        @param mvaConfig used to define the target variable
    """
    o = b2latex.LatexFile()
    o += b2latex.SubSection("Diagonal plot")
    diagonalPlot = 'diag_' + filename
    plot = b2plot.Diagonal()
    plot.set_plot_options({'linestyle': '-', 'lw': 3})
    plot.add(nTupleData, 'extraInfo(SignalProbability)',
             nTupleData[mvaConfig.target] > 0, nTupleData[mvaConfig.target] == 0)
    plot.finish()
    plot.save(diagonalPlot)
    o += b2latex.Graphics().add(diagonalPlot, width=0.7).finish()
    return o.finish()


def createFSPReport(resource, particleName, particleLabel, matchedList, mvaConfig, userCutConfig, postCutConfig, postCut,
                    plotConfig, tmvaTraining, nTuple, listCounts, mcCounts):
    """
    Creates a pdf document for this final state particle
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param matchedList name of matched particle list
        @param mvaConfig configuration for mva
        @param userCutConfig user cut configuration
        @param postCutConfig post cut configuration
        @param postCut post cut detemined by postCutDetermination
        @param plotConfig plot configuration
        @param tmvaTraining config filename for TMVA training
        @param mcCounts MC-Particle statistics
        @param listCounts particle count in all particle lists
        @param nTuple data with the finished data
        @return dictionary containing latex placeholders of this particle
    """
    import seaborn
    import pandas
    import numpy
    from root_pandas import read_root
    # Set nice searborn settings
    seaborn.set(font_scale=5)
    seaborn.set_style('whitegrid')

    nTupleData = loadNTupleDataFrame(nTuple)

    pdgcode = str(abs(pdg.from_name(particleName)))
    mcCountsData = loadMCCountsDataFrame(mcCounts)
    mcCountsData = mcCountsData[pdgcode]

    listCountsData = loadListCountsDataFrame(listCounts)
    listCountsData = listCountsData[[matchedList + '_Signal', matchedList + '_Background', matchedList + '_All']]
    listCountsData.columns = ["Signal", "Background", "All"]

    nEvents = mcCountsData.count()
    nCandidatesAfterPostCut = float(nTupleData[mvaConfig.target].count())
    nSignalAfterPostCut = float(numpy.sum(nTupleData[mvaConfig.target] > 0))
    nBackgroundAfterPostCut = float(numpy.sum(nTupleData[mvaConfig.target] == 0))

    if(nEvents != listCountsData['All'].count()):
        B2WARNING("Number of Events is different in created ntuples, statistical quantities which are calculated may be wrong.")

    raw_name = particleName
    if particleLabel != '':
        raw_name += ':' + particleLabel
    name = format.decayDescriptor(raw_name)

    o = b2latex.LatexFile()
    o += b2latex.Section(name).finish()

    o += b2latex.SubSection("Statistic").finish()
    if userCutConfig.userCut != '':
        string = b2latex.String(r"Quantities (except MC truth) are calculated after applying the user-defined cut {cut}")
        o += string.finish(cut=userCutConfig.userCut)
    table = b2latex.LongTable(columnspecs=r'p{5cm}rrrrr',
                              caption='Statistical quantities per Monte Carlo event',
                              head=r'Quantity & Average $\pm$ Error & Deviation & Minimum & Maximum & PostCut',
                              format_string=r'{quantity} & $({mean:.3f} \pm {error:.3f})$ & ${std:.3f}$ & ${min:.3f}$ &'
                                            r' ${max:.3f}$ & $({meanPostCut:.3f} \pm {errorPostCut:.3f})$')
    table.add(quantity='MC-Particles',
              mean=mcCountsData.sum() / mcCountsData.count(),
              error=b2stat.poisson_error(mcCountsData.sum()) / nEvents,
              std=mcCountsData.std(),
              min=mcCountsData.min(),
              max=mcCountsData.max(),
              meanPostCut=mcCountsData.sum() / nEvents,
              errorPostCut=b2stat.poisson_error(mcCountsData.sum()) / nEvents)
    table.add(quantity='Candidates',
              mean=listCountsData['All'].sum() / nEvents,
              error=b2stat.poisson_error(listCountsData['All'].sum()) / nEvents,
              std=listCountsData['All'].std(),
              min=listCountsData['All'].min(),
              max=listCountsData['All'].max(),
              meanPostCut=nCandidatesAfterPostCut / nEvents,
              errorPostCut=b2stat.poisson_error(nCandidatesAfterPostCut) / nEvents)
    table.add(quantity='Candidates (Signal)',
              mean=listCountsData['Signal'].sum() / nEvents,
              error=b2stat.poisson_error(listCountsData['Signal'].sum()) / nEvents,
              std=listCountsData['Signal'].std(),
              min=listCountsData['Signal'].min(),
              max=listCountsData['Signal'].max(),
              meanPostCut=nSignalAfterPostCut / nEvents,
              errorPostCut=b2stat.poisson_error(nSignalAfterPostCut) / nEvents)
    table.add(quantity='Candidates (Background)',
              mean=listCountsData['Background'].sum() / nEvents,
              error=b2stat.poisson_error(listCountsData['Background'].sum()) / nEvents,
              std=listCountsData['Background'].std(),
              min=listCountsData['Background'].min(),
              max=listCountsData['Background'].max(),
              meanPostCut=nBackgroundAfterPostCut / nEvents,
              errorPostCut=b2stat.poisson_error(nBackgroundAfterPostCut) / nEvents)
    o += table.finish()

    user_efficiency = listCountsData['Signal'].sum() / mcCountsData.sum() * 100.0
    user_efficiency_error = b2stat.binom_error(listCountsData['Signal'].sum(), mcCountsData.sum()) * 100.0
    user_purity = listCountsData['Signal'].sum() / (listCountsData['All'].sum()) * 100.0
    user_purity_error = b2stat.binom_error(listCountsData['Signal'].sum(), listCountsData['All'].sum()) * 100.0
    post_efficiency = nSignalAfterPostCut / mcCountsData.sum() * 100.0
    post_efficiency_error = b2stat.binom_error(nSignalAfterPostCut, mcCountsData.sum()) * 100.0
    post_purity = nSignalAfterPostCut / nCandidatesAfterPostCut * 100.0
    post_purity_error = b2stat.binom_error(nSignalAfterPostCut, nCandidatesAfterPostCut) * 100.0

    table = b2latex.LongTable(columnspecs=r'p{5cm}rr',
                              caption='Efficiencies and Purities',
                              head=r'Cut & Efficiency $\pm$ Error & Purity $\pm$ Error',
                              format_string=r'{cut} & $({efficiency:.2f} \pm {eerr:.2f})$ & $({purity:.2f} \pm {perr:.2f})$')
    table.add(cut='User',
              efficiency=user_efficiency,
              eerr=user_efficiency_error,
              purity=user_purity,
              perr=user_purity_error)
    table.add(cut='Post',
              efficiency=post_efficiency,
              eerr=post_efficiency_error,
              purity=post_purity,
              perr=post_purity_error)
    o += table.finish()

    if plotConfig.Diagonal:
        o += createDiagonalPlot(createUniqueFilename(raw_name, resource.hash), nTupleData, mvaConfig)

    o += b2latex.SubSection("MVA").finish()
    o += createTMVASection(createUniqueFilename(raw_name, resource.hash), tmvaTraining, mvaConfig, plotConfig)

    resource.needed = False
    resource.cache = True
    return {'name': name, 'list': matchedList, 'page': o.finish(), 'user_efficiency': user_efficiency, 'user_purity': user_purity,
            'post_efficiency': post_efficiency, 'post_purity': post_purity, 'covered': 100.0}


def createParticleReport(resource, particleName, particleLabel, channelNames, matchedLists, mvaConfig, mvaConfigs,
                         userCutConfig, userConfigs, preCutConfig, preCut, preCutHistograms, postCutConfig, postCut,
                         plotConfig, tmvaTrainings, splotTrainings, signalProbabilities, nTuple, listCounts, mcCounts,
                         coveredBranchingFractions):
    """
    Creates a pdf document with the PreCut and Training plots
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
        @param mcCounts
        @param listCounts particle count in all particle lists
        @return dictionary containing latex placeholders of this particle
    """
    import seaborn
    import pandas
    import numpy
    from root_pandas import read_root
    # Set nice searborn settings
    seaborn.set(font_scale=5)
    seaborn.set_style('whitegrid')

    nTupleData = loadNTupleDataFrame(nTuple)

    pdgcode = str(abs(pdg.from_name(particleName)))
    mcCountsData = loadMCCountsDataFrame(mcCounts)
    mcCountsData = mcCountsData[pdgcode]

    listCountsData = loadListCountsDataFrame(listCounts)
    valid_lists = [matchedList for matchedList in matchedLists if matchedList is not None]
    listCountsData['Signal'] = listCountsData[[matchedList + '_Signal' for matchedList in valid_lists]].sum(axis=1)
    listCountsData['Background'] = listCountsData[[matchedList + '_Background' for matchedList in valid_lists]].sum(axis=1)
    listCountsData['All'] = listCountsData[[matchedList + '_All' for matchedList in valid_lists]].sum(axis=1)

    nEvents = mcCountsData.count()
    nCandidatesAfterPostCut = float(nTupleData[mvaConfig.target].count())
    nSignalAfterPostCut = float(numpy.sum(nTupleData[mvaConfig.target] > 0))
    nBackgroundAfterPostCut = float(numpy.sum(nTupleData[mvaConfig.target] == 0))

    if(nEvents != listCountsData['All'].count()):
        B2WARNING("Number of Events is different in created ntuples, statistical quantities which are calculated may be wrong.")

    raw_name = particleName
    if particleLabel != '':
        raw_name += ':' + particleLabel
    name = format.decayDescriptor(raw_name)

    o = b2latex.LatexFile()

    o += b2latex.Section(name).finish()

    o += b2latex.SubSection("Channels").finish()
    string = b2latex.String(r"In the reconstruction of {name} {nChannels} out of {max_nChannels} possible channels were used. "
                            r"Therefore the covered branching fraction in percent is {covBR}, whereas {max_covBR} was the upper"
                            r" limit.")

    df = coveredBranchingFractions.groupby(['channelName'])['fraction'].sum()
    nChannels = len([t for t in signalProbabilities if t is not None])
    max_nChannels = len(signalProbabilities)
    covBR = 0
    max_covBR = 0
    for channelName, s in zip(channelNames, signalProbabilities):
        if s is not None:
            covBR += df[channelName]
        max_covBR += df[channelName]
    o += string.finish(name=name, nChannels=nChannels, max_nChannels=max_nChannels, covBR=covBR, max_covBR=max_covBR)

    itemize = b2latex.Itemize()
    for channelName, s in zip(channelNames, signalProbabilities):
        niceDecayChannel = format.decayDescriptor(channelName)
        s = r"{name} BR: {BR:.2f}".format(name=niceDecayChannel, BR=df[channelName])
        if s is None:
            itemize.add(s + ' \t was ignored')
        else:
            itemize.add(s)
    o += itemize.finish()

    o += b2latex.SubSection("Statistic").finish()
    if userCutConfig.userCut != '':
        string = b2latex.String(r"Quantities (except MC truth) are calculated after applying the user-defined {cut}")
        o += string.finish(cut=userCutConfig.userCut)
    o += b2latex.String(r"Quantities (except MC truth) are calculated after applying the automatic pre-cuts.").finish()
    table = b2latex.LongTable(columnspecs=r'p{5cm}rrrrr',
                              caption='Statistical quantities per Monte Carlo event',
                              head=r'Quantity & Average $\pm$ Error & Deviation & Minimum & Maximum & PostCut',
                              format_string=r'{quantity} & $({mean:.3f} \pm {error:.3f})$ & ${std:.3f}$ & ${min:.3f}$ &'
                                            r' ${max:.3f}$ & $({meanPostCut:.3f} \pm {errorPostCut:.3f})$')
    table.add(quantity='MC-Particles',
              mean=mcCountsData.sum() / mcCountsData.count(),
              error=b2stat.poisson_error(mcCountsData.sum()) / nEvents,
              std=mcCountsData.std(),
              min=mcCountsData.min(),
              max=mcCountsData.max(),
              meanPostCut=mcCountsData.sum() / nEvents,
              errorPostCut=b2stat.poisson_error(mcCountsData.sum()) / nEvents)
    table.add(quantity='Candidates',
              mean=listCountsData['All'].sum() / nEvents,
              error=b2stat.poisson_error(listCountsData['All'].sum()) / nEvents,
              std=listCountsData['All'].std(),
              min=listCountsData['All'].min(),
              max=listCountsData['All'].max(),
              meanPostCut=nCandidatesAfterPostCut / nEvents,
              errorPostCut=b2stat.poisson_error(nCandidatesAfterPostCut) / nEvents)
    table.add(quantity='Candidates (Signal)',
              mean=listCountsData['Signal'].sum() / nEvents,
              error=b2stat.poisson_error(listCountsData['Signal'].sum()) / nEvents,
              std=listCountsData['Signal'].std(),
              min=listCountsData['Signal'].min(),
              max=listCountsData['Signal'].max(),
              meanPostCut=nSignalAfterPostCut / nEvents,
              errorPostCut=b2stat.poisson_error(nSignalAfterPostCut) / nEvents)
    table.add(quantity='Candidates (Background)',
              mean=listCountsData['Background'].sum() / nEvents,
              error=b2stat.poisson_error(listCountsData['Background'].sum()) / nEvents,
              std=listCountsData['Background'].std(),
              min=listCountsData['Background'].min(),
              max=listCountsData['Background'].max(),
              meanPostCut=nBackgroundAfterPostCut / nEvents,
              errorPostCut=b2stat.poisson_error(nBackgroundAfterPostCut) / nEvents)

    o += table.finish()

    user_efficiency = listCountsData['Signal'].sum() / mcCountsData.sum() * 100.0
    user_efficiency_error = b2stat.binom_error(listCountsData['Signal'].sum(), mcCountsData.sum()) * 100.0
    user_purity = listCountsData['Signal'].sum() / (listCountsData['All'].sum()) * 100.0
    user_purity_error = b2stat.binom_error(listCountsData['Signal'].sum(), listCountsData['All'].sum()) * 100.0
    post_efficiency = nSignalAfterPostCut / mcCountsData.sum() * 100.0
    post_efficiency_error = b2stat.binom_error(nSignalAfterPostCut, mcCountsData.sum()) * 100.0
    post_purity = nSignalAfterPostCut / nCandidatesAfterPostCut * 100.0
    post_purity_error = b2stat.binom_error(nSignalAfterPostCut, nCandidatesAfterPostCut) * 100.0

    table = b2latex.LongTable(columnspecs=r'p{5cm}rr',
                              caption='Efficiencies and Purities',
                              head=r'Cut & Efficiency $\pm$ Error & Purity $\pm$ Error',
                              format_string=r'{cut} & $({efficiency:.2f} \pm {eerr:.2f})$ & $({purity:.2f} \pm {perr:.2f})$')
    table.add(cut='User',
              efficiency=user_efficiency,
              eerr=user_efficiency_error,
              purity=user_purity,
              perr=user_purity_error)
    table.add(cut='Post',
              efficiency=post_efficiency,
              eerr=post_efficiency_error,
              purity=post_purity,
              perr=post_purity_error)

    o += table.finish()

    if plotConfig.Diagonal:
        o += createDiagonalPlot(createUniqueFilename(raw_name, resource.hash), nTupleData, mvaConfig)

    channels = []
    for i, channelName in enumerate(channelNames):
        tmvaTraining = tmvaTrainings[i]
        plist = matchedLists[i]
        channels.append(dict(name=format.decayDescriptor(channelName), list=plist))

        o += b2latex.SubSection(format.decayDescriptor(channelName)).finish()

        o += b2latex.SubSubSection("MC-based MVA").finish()
        o += createTMVASection(createUniqueFilename(channelName, resource.hash), tmvaTraining, mvaConfigs[i], plotConfig)

    resource.needed = False
    resource.cache = True
    return {'name': name, 'page': o.finish(), 'user_efficiency': user_efficiency, 'user_purity': user_purity,
            'pre_efficiency': 0, 'pre_purity': 0, 'covered': covBR,
            'post_efficiency': post_efficiency, 'post_purity': post_purity,
            'detector_efficiency': 0, 'detector_purity': 0, 'channels': channels}


def createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut):
    """
    Creates tex file for the PreCuts of this channel. Adds necessary items to the placeholder dictionary
    and returns the modified dictionary.
    @param placeholders dictionary with values for every placeholder in the latex-template
    @param preCutHistogram preCutHistogram (filename, histogram postfix)
    @param preCutConfig configuration for pre cut
    @param preCut used preCuts for this channel
    """
    if preCutHistogram is None:
        hash = dagFramework.create_hash([placeholders])
        placeholders['preCutTexFile'] = removeJPsiSlash(
            '{name}_preCut_{hash}.tex'.format(
                name=placeholders['particleName'],
                hash=hash))
        placeholders['preCutTemplateFile'] = 'analysis/scripts/fei/templates/MissingPreCutTemplate.tex'
        placeholders['isIgnored'] = True
        placeholders['channelPurity'] = 0
        placeholders['channelNSignal'] = 0
        placeholders['channelNBackground'] = 0
        placeholders['channelNSignalAfterPreCut'] = 0
        placeholders['channelNBackgroundAfterPreCut'] = 0
        placeholders['channelNSignalAfterUserCut'] = 0
        placeholders['channelNBackgroundAfterUserCut'] = 0
        placeholders['channelPurityAfterPreCut'] = 0
        placeholders['channelEfficiencyAfterPreCut'] = 0
        placeholders['channelPurityAfterUserCut'] = 0
        placeholders['channelEfficiencyAfterUserCut'] = 0

        if 'ignoreReason' not in placeholders:
            placeholders['ignoreReason'] = """This channel was ignored because one or more daughter particles were ignored."""
    else:
        placeholders['preCutROOTFile'] = preCutHistogram[0]
        rootfile = ROOT.TFile(placeholders['preCutROOTFile'])
        # Calculate purity and efficiency for this channel
        without_userCut_hist = rootfile.GetListOfKeys().At(0).ReadObj()
        signal_hist = rootfile.GetListOfKeys().At(1).ReadObj()
        background_hist = rootfile.GetListOfKeys().At(3).ReadObj()

        placeholders['channelNSignal'] = without_userCut_hist.GetBinContent(2)
        placeholders['channelNBackground'] = without_userCut_hist.GetBinContent(1) - without_userCut_hist.GetBinContent(2)
        placeholders['channelPurity'] = '{:.5f}'.format(purity(placeholders['channelNSignal'], placeholders['channelNBackground']))

        placeholders['channelNSignalAfterUserCut'] = preCutDetermination.GetNumberOfEvents(signal_hist)
        placeholders['channelNBackgroundAfterUserCut'] = preCutDetermination.GetNumberOfEvents(background_hist)
        placeholders['channelAfterUserPurity'] = '{:.5f}'.format(
            purity(
                placeholders['channelNSignalAfterUserCut'],
                placeholders['channelNBackgroundAfterUserCut']))

        if preCut is not None:
            lc, uc = preCut['range']
            placeholders['preCutRange'] = '({:.2f},'.format(lc) + ' {:.2f}'.format(uc) + ')'
            placeholders['channelNSignalAfterPreCut'] = preCutDetermination.GetNumberOfEventsInRange(signal_hist, (lc, uc))
            placeholders['channelNBackgroundAfterPreCut'] = preCutDetermination.GetNumberOfEventsInRange(background_hist, (lc, uc))
            placeholders['channelPurityAfterPreCut'] = '{:.5f}'.format(
                purity(
                    placeholders['channelNSignalAfterPreCut'],
                    placeholders['channelNBackgroundAfterPreCut']))
            placeholders['channelEfficiencyAfterPreCut'] = '{:.5f}'.format(
                placeholders['channelNSignalAfterPreCut'] /
                placeholders['channelNSignal'])
        else:
            placeholders['preCutRange'] = '---'
            placeholders['channelNSignalAfterPreCut'] = 0
            placeholders['channelNBackgroundAfterPreCut'] = 0
            placeholders['channelPurityAfterPreCut'] = 0
            placeholders['channelEfficiencyAfterPreCut'] = 0

        placeholders['preCutVariable'] = preCutConfig.variable
        placeholders['preCutEfficiency'] = '{:.5f}'.format(preCutConfig.efficiency)
        placeholders['preCutPurity'] = '{:.5f}'.format(preCutConfig.purity)

        hash = dagFramework.create_hash([placeholders])

        ROOT.gROOT.SetBatch(True)

        placeholders['preCutAllPlot'] = removeJPsiSlash(
            '{name}_preCut_{hash}_all.png'.format(
                name=placeholders['particleName'],
                hash=hash))
        if not os.path.isfile(placeholders['preCutAllPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutAllPlot'], 'all', preCut, preCutConfig)

        placeholders['preCutSignalPlot'] = removeJPsiSlash(
            '{name}_preCut_{hash}_signal.png'.format(
                name=placeholders['particleName'],
                hash=hash))
        if not os.path.isfile(placeholders['preCutSignalPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutSignalPlot'], 'signal', preCut, preCutConfig)

        placeholders['preCutBackgroundPlot'] = removeJPsiSlash(
            '{name}_preCut_{hash}_background.png'.format(
                name=placeholders['particleName'],
                hash=hash))
        if not os.path.isfile(placeholders['preCutBackgroundPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutBackgroundPlot'], 'bckgrd', preCut, preCutConfig)

        placeholders['preCutRatioPlot'] = removeJPsiSlash(
            '{name}_preCut_{hash}_ratio.png'.format(
                name=placeholders['particleName'],
                hash=hash))
        if not os.path.isfile(placeholders['preCutRatioPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutRatioPlot'], 'ratio', preCut, preCutConfig)

        placeholders['preCutTexFile'] = removeJPsiSlash(
            '{name}_preCut_{hash}.tex'.format(
                name=placeholders['particleName'],
                hash=hash))
        placeholders['preCutTemplateFile'] = 'analysis/scripts/fei/templates/PreCutTemplate.tex'

    if not os.path.isfile(placeholders['preCutTexFile']):
        createTexFile(placeholders['preCutTexFile'], placeholders['preCutTemplateFile'], placeholders)

    return placeholders


def sendMail():
    import smtplib
    from email.MIMEMultipart import MIMEMultipart
    from email.MIMEBase import MIMEBase
    from email.MIMEText import MIMEText
    from email.Utils import COMMASPACE, formatdate
    from email import Encoders
    import random

    fromMail = 'nordbert@kit.edu'
    toMails = ['t.keck@online.de', 'christian.pulvermacher@kit.edu']

    ClicheBegin = ['Hello', 'Congratulations!', 'Whats up?', 'Dear Sir/Madam', 'Howdy!']
    Person = ['I\'m', 'Nordbert is']
    StateOfMind = ['happy', 'cheerful', 'joyful', 'delighted', 'lucky']
    VerbAnnounce = ['announce', 'declare', 'make public', 'make known', 'report', 'publicize', 'broadcast', 'publish']
    PossesivePronoun = ['our', 'my', 'your']
    Adjective = [
        'awesome',
        'revolutionary',
        'gorgeous',
        'beautiful',
        'spectacular',
        'splendid',
        'superb',
        'wonderful',
        'impressive',
        'amazing',
        'stunning',
        'breathtaking',
        'incredible']
    VerbFinished = ('has', ['finished', 'completed', 'terminated', 'concluded'])
    Motivation = [
        ('Keep on your', Adjective, 'work!'), (Person, 'so proud of you!'), ([
            'Enjoy this', 'Relax a'], 'moment and have a', [
            'cookie', '\bn ice', 'bath', 'day off'], '\b.')]
    ClicheEnd = ['Best regards', 'Yours sincerely', 'Kind regards', 'Yours faithfully', 'See you', 'cu']

    def generate_sentence(term):
        if isinstance(term, str):
            return term
        if isinstance(term, list):
            return generate_sentence(term[random.randint(0, len(term) - 1)])
        if isinstance(term, tuple):
            return " ".join([generate_sentence(subterm) for subterm in term])
        raise RuntimeError('Invalid type received in sentence generator')

    sentence = (ClicheBegin,
                '\n\n',
                Person,
                StateOfMind,
                'to',
                VerbAnnounce,
                'that',
                PossesivePronoun,
                Adjective,
                'Full Event Interpretation',
                VerbFinished,
                '\b.\n\nThat\'s',
                Adjective,
                '\b!\n',
                Motivation,
                '\n\n',
                ClicheEnd,
                '\b,\n',
                ['',
                 ('the',
                  [Adjective,
                   StateOfMind])],
                'Nordbert')

    text = MIMEText(re.sub('.\b', '', generate_sentence(sentence)))

    filename = 'FEIsummary.pdf'
    pdf = MIMEBase('application', "octet-stream")
    pdf.set_payload(open(filename, "rb").read())
    Encoders.encode_base64(pdf)
    pdf.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(filename))

    msg = MIMEMultipart()
    msg['From'] = fromMail
    msg['To'] = COMMASPACE.join(toMails)
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = 'Congratulations!'
    msg.attach(text)
    msg.attach(pdf)

    server = smtplib.SMTP('smtp.kit.edu')
    # server.login(fromMail, 'Not necessary for kit.edu :-)')
    server.sendmail(fromMail, toMails, msg.as_string())
    server.quit()
