#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import pdg

from B2Tools import b2plot
from B2Tools import b2stat
from B2Tools import b2latex
from B2Tools import format

from fei import compactReporting

import os
import re
import sys
import subprocess
import copy
import itertools
import pickle
import numpy

import ROOT
from ROOT import gSystem
gSystem.Load('libanalysis.so')
from ROOT import Belle2

import seaborn
seaborn.set(font_scale=5)
seaborn.set_style('whitegrid')

import pandas
import numpy
from root_pandas import read_root
from root_numpy import hist2array

# workaround to make Belle2.Variables namespace available in root6
import variables


def read_root_scaled(filename, tree_key, limit_average):
    """
    Read root-file into pandas.DataFrame, if file contains more entries than the given limit
    the file is randomly sampled, so that in total the limit is met on average,
    so the returned pandas.DataFrame will have more or less events than this limit (but not much...)!
    @param filename of the ROOT file
    @param tree_key name of the TTree in the ROOT file
    @param limit_average the limit which should hold on average
    """
    import ROOT
    from root_pandas import read_root
    import pandas

    if not os.path.exists(filename):
        raise IOError("File %s does not exists." % filename)

    tfile = ROOT.TFile(filename)
    tree = tfile.Get(tree_key)
    nEntries = tree.GetEntries()
    dataframe = pandas.DataFrame()

    if nEntries == 0:
        B2WARNING("Tree used by read_root_scaled is empty")
        return dataframe, 1.0

    percentage = limit_average / float(nEntries)
    if percentage > 1.0:
        return read_root(filename, tree_key=tree_key), 1.0

    for df in read_root(filename, tree_key=tree_key, chunksize=limit_average):
        dataframe = dataframe.append(df.sample(frac=percentage))

    if len(dataframe) == 0:
        B2WARNING("Dataframe returned by read_root_scaled is empty")
        return dataframe, 1.0
    return dataframe, float(nEntries) / len(dataframe)


def loadNTupleDataFrame(filename):
    """
    Load NTuple data of a Particle into a pandas.DataFrame.
    Columns are named by the original variable names written into the NTuple.
        @param filename name of the ROOT file containing the NTuple
        @return pandas.DataFrame with NTuple data
    """
    df, scale = read_root_scaled(filename, tree_key='variables', limit_average=1000000)

    def rename(name):
        return Belle2.Variable.invertMakeROOTCompatible(name)

    df.columns = list(map(rename, df.columns))
    return df, scale


def loadTMVADataFrame(filename):
    """
    Load TMVA training and test data into a pandas.DataFrame.
    Columns are named by the original variable names used in the training.
        @param filename name of the ROOT file containing the TMVA data
        @return pandas.DataFrame with TMVA data
    """
    train_df, train_scale = read_root_scaled(filename, tree_key='TrainTree', limit_average=1000000)
    test_df, test_scale = read_root_scaled(filename, tree_key='TestTree', limit_average=1000000)

    def rename(name):
        return Belle2.Variable.invertMakeROOTCompatible(name)

    train_df.columns = list(map(rename, train_df.columns))
    test_df.columns = list(map(rename, test_df.columns))
    train_df['__isTrain__'] = True
    test_df['__isTrain__'] = False
    df = train_df.append(test_df, ignore_index=True)
    df['__isSignal__'] = df['className'].map(lambda x: True if x.decode('utf-8') == 'Signal' else False)
    return df, train_scale, test_scale


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


if __name__ == '__main__':

    if len(sys.argv) != 3:
        print("Usage: " + sys.argv[0] + ' SummaryFile.pickle OutputFile.tex')
        sys.exit(1)

    obj = pickle.load(open(sys.argv[1], 'rb'))
    obj_hash = sys.argv[1].replace('Summary_', '').replace('.pickle', '')

    # Load global stuff
    mc_counts = compactReporting.getMCCounts(obj)
    mc_counts_max = compactReporting.getMCCounts(obj, compactReporting.calcMax)
    mc_counts_min = compactReporting.getMCCounts(obj, compactReporting.calcMin)
    mc_counts_mean = compactReporting.getMCCounts(obj, compactReporting.calcMean)
    mc_counts_dev = compactReporting.getMCCounts(obj, compactReporting.calcStdDev)
    print('MC-Counts loaded...')
    list_counts = compactReporting.getListCounts(obj)
    list_counts_max = compactReporting.getListCounts(obj, compactReporting.calcMax)
    list_counts_min = compactReporting.getListCounts(obj, compactReporting.calcMin)
    list_counts_mean = compactReporting.getListCounts(obj, compactReporting.calcMean)
    list_counts_dev = compactReporting.getListCounts(obj, compactReporting.calcStdDev)
    print('List-Counts loaded...')

    module_statistics = compactReporting.getModuleStatistics(obj)
    print('Module-Statistics loaded...')
    coveredBranchingFractions = compactReporting.getCoveredBranchingFractions(obj)
    print('Covered Branching Fractions loaded...')
    detectorData = compactReporting.getDetectorEfficiencies(obj, mc_counts)
    detectorEfficiencies = detectorData['__values__']
    detectorEff_Errors = detectorData['__errors__']
    print('Detector Efficiencies loaded...')
    userCutData = compactReporting.getUserCutEfficiencies(obj, mc_counts)
    userCutEfficiencies = userCutData['__values__']
    userCutEff_Errors = userCutData['__errors__']
    print('UserCut Efficiencies loaded...')
    postCutData = compactReporting.getPostCutEfficiencies(obj, mc_counts, list_counts)
    postCutEfficiencies = postCutData['__values__']
    postCutEff_Errors = postCutData['__errors__']
    print('PostCut Efficiencies loaded...')
    preCutData = compactReporting.getPreCutEfficiencies(obj, mc_counts, list_counts)
    preCutEfficiencies = preCutData['__values__']
    preCutEff_Errors = preCutData['__errors__']
    print('PreCut Efficiencies loaded...')
    tmvaTrainings = compactReporting.getMVARankings(obj)
    print('TMVA Training Data loaded...')

    # Create latex file
    o = b2latex.LatexFile()

    o += b2latex.TitlePage(title='Full Event Interpretation Report',
                           authors=['Thomas Keck', 'Christian Pulvermacher'],
                           abstract=r"""
                           This report contains key performance indicators and control plots of the Full Event Interpretation.
                           The user-, pre-, and post-cuts as well as trained multivariate selection methods are described.
                           Furthermore the resulting purities and efficiencies are stated.
                              """,
                           add_table_of_contents=True).finish()

    o += b2latex.Section("Summary").finish()
    o += b2latex.String(r"""
        For each decay channel of each particle a multivariate selection method is trained after applying
        a fast pre-cut on the candidates. Afterwards, a post-cut is applied on the signal probability calculated by the method.
        This reduces combinatorics in the following stages of the Full
        Event Interpretation. For some particles, in particular the final B mesons, an additional user-cut
        is applied before all other cuts.
        """).finish()

    table = b2latex.LongTable(columnspecs=r'c|r|rrrr|rrrr',
                              caption='Per-particle efficiency and purity before and after the applied user-, pre- and post-cut.',
                              head=r'Particle & Covered '
                                   r' & \multicolumn{4}{c}{Efficiency in \%}  &  \multicolumn{4}{c}{Purity in \%} \\'
                                   r' & BR in \% &  recon. & user-cut & pre-cut & post-cut '
                                   r' & recon. & user-cut & pre-cut & post-cut \\',
                              format_string=r'{name} & {covered:.2f} & {detector_efficiency:.2f} & {user_efficiency:.2f}'
                                            r' & {pre_efficiency:.2f}  & {post_efficiency:.2f}'
                                            r' & {detector_purity:.2f} & {user_purity:.2f} & {pre_purity:.2f} & {post_purity:.2f}')

    for particle in obj['particles']:
        table.add(name=format.decayDescriptor(particle.identifier),
                  covered=coveredBranchingFractions[particle.identifier],
                  detector_efficiency=detectorEfficiencies[particle.identifier]['__total__'][0],
                  user_efficiency=userCutEfficiencies[particle.identifier]['__total__'][0],
                  pre_efficiency=preCutEfficiencies[particle.identifier]['__total__'][0],
                  post_efficiency=postCutEfficiencies[particle.identifier][0],
                  detector_purity=detectorEfficiencies[particle.identifier]['__total__'][1],
                  user_purity=userCutEfficiencies[particle.identifier]['__total__'][1],
                  pre_purity=preCutEfficiencies[particle.identifier]['__total__'][1],
                  post_purity=postCutEfficiencies[particle.identifier][1])
    o += table.finish()

    # If you change the number of colors, than change below \ifnum5 accordingly
    colours = ["orange", "blue", "red", "green", "cyan", "purple"]

    o += b2latex.Section("CPU time per channel").finish()
    colour_list = b2latex.DefineColourList()
    o += colour_list.finish()

    sum_time_seconds = module_statistics[1]
    moduleTypes = ['ParticleCombiner', 'TMVAExpert', 'MCMatching', 'ParticleVertexFitter', 'ParticleLoader', 'Other']
    list_to_channel = {l: c for l, c in zip(obj['mlists'], obj['cnames'])}

    table = b2latex.LongTable(columnspecs=r'lrcrr',
                              caption='Total CPU time spent in event() calls for each channel. Bars show ' +
                                      ', '.join('\\textcolor{%s}{%s}' % (c, m) for c, m in zip(colour_list.colours, moduleTypes)) +
                                      ', in this order. Does not include I/O, initialisation, training, post-cuts etc.',
                              head=r'Decay & CPU time & by module & per (true) candidate & Relative time ',
                              format_string=r'{name} & {time} & {bargraph} & {timePerCandidate} & {timePercent:.2f}\% ')

    statTable = []
    lines = []

    for listname, stat in module_statistics[0].items():
        if listname not in list_counts:
            continue
        trueCandidates = list_counts[listname]['Signal']
        allCandidates = list_counts[listname]['All']

        if trueCandidates == 0 or allCandidates == 0:
            continue

        time = sum(m for m in stat.values())
        timePerCandidate = format.duration(time / trueCandidates) + ' (' + format.duration(time / allCandidates) + ')'
        timePercent = time / sum_time_seconds * 100 if sum_time_seconds > 0 else 0

        percents = tuple(stat[key] / float(time) * 100.0 if key in stat.keys() else 0.0 for key in moduleTypes)

        if len(percents) < 6:
            percents = percents + (0,) * (6 - len(percents))

        table.add(name=format.decayDescriptor(list_to_channel[listname]),
                  bargraph=r'\plotbar{ %g/, %g/, %g/, %g/, %g/, %g/, }' % percents,
                  time=format.duration(time),
                  timePerCandidate=timePerCandidate,
                  timePercent=time / sum_time_seconds * 100 if sum_time_seconds > 0 else 0)
    o += table.finish(tail='Total & & ' + format.duration(sum_time_seconds) + ' & & 100 ')

    id_to_mlist = compactReporting.HashableDict()
    id_to_cnames = compactReporting.HashableDict()
    for particle in obj['particles']:
        id_to_mlist[particle.identifier] = []
        id_to_cnames[particle.identifier] = []
    for m, c, n in zip(obj['mlists'], obj['clists'], obj['cnames']):
        id = n.split(' ==>')[0]
        id_to_mlist[id].append(c)
        id_to_cnames[id].append(n)
    cnames_to_trainings = dict(zip(obj['cnames'], obj['training_data']))
    cnames_to_hist = dict(zip(obj['cnames'], obj['pre_cut_histograms']))

    nEvents = mc_counts['__total__']

    for particle, nTuple, pre_cuts in zip(obj['particles'], obj['ntuples'], obj['pre_cuts']):
        print(particle.identifier)

        filename = createUniqueFilename(particle.identifier, obj_hash, suffix='png')

        o += b2latex.Section(format.decayDescriptor(particle.identifier)).finish()
        o += b2latex.SubSection("Channels").finish()
        string = b2latex.String(r"In the reconstruction of {name} {nChannels} out of {max_nChannels} possible channels were used. "
                                r"Therefore the covered branching fraction in percent is {covBR:.3f}, whereas {max_covBR:.3f} "
                                r"was the upper limit.")

        nChannels = len([t for t in list(userCutEfficiencies[particle.identifier].values()) if t[0] != 0.0]) - 1
        max_nChannels = len(userCutEfficiencies[particle.identifier]) - 1       # -1 because of entry '__total__'
        covBR = userCutEfficiencies[particle.identifier]['__total__'][0]
        max_covBR = coveredBranchingFractions[particle.identifier]

        o += string.finish(name=format.decayDescriptor(particle.identifier),
                           nChannels=nChannels,
                           max_nChannels=max_nChannels,
                           covBR=covBR, max_covBR=max_covBR)

        # TODO Get theoretical branching fraction instead of efficiency!
        itemize = b2latex.Itemize()
        for channelName, values in userCutEfficiencies[particle.identifier].items():
            if channelName is '__total__':
                continue
            niceDecayChannel = format.decayDescriptor(channelName)
            text = r"{name} (BR: {BR:.4f})".format(name=niceDecayChannel, BR=values[0])
            if values[0] == 0.0:
                itemize.add(text + ' \t was ignored')
            else:
                itemize.add(text)
        o += itemize.finish()

        o += b2latex.SubSection("Statistic").finish()
        if particle.userCutConfig.userCut != '':
            string = b2latex.String(r"Quantities (except MC truth) are calculated after applying the user-defined cut {cut}")
            o += string.finish(cut=particle.userCutConfig.userCut)
        o += b2latex.String(r"Quantities (except MC truth) are calculated after applying the automatic pre-cuts.").finish()

        lc_sum = {}
        lc_mean = {}
        lc_min = {}
        lc_max = {}
        lc_dev = {}
        for mother in id_to_mlist[particle.identifier]:
            if mother is None:
                continue
            for tag in ['All', 'Signal', 'Background']:
                lc_sum[tag] = list_counts[mother][tag]
                lc_mean[tag] = list_counts_mean[mother][tag]
                lc_min[tag] = list_counts_min[mother][tag]
                lc_max[tag] = list_counts_max[mother][tag]
                lc_dev[tag] = list_counts_dev[mother][tag]

        table = b2latex.LongTable(columnspecs=r'p{5cm}rrrrr',
                                  caption='Statistical quantities per Monte Carlo event',
                                  head=r'Quantity & Average $\pm$ Error & Deviation & Minimum & Maximum & PostCut',
                                  format_string=r'{quantity} & $({mean:.3f} \pm {error:.3f})$ & ${std:.3f}$ & ${min:.3f}$ &'
                                                r' ${max:.3f}$ & $({meanPostCut:.3f} \pm {errorPostCut:.3f})$')

        particle_pdg = abs(pdg.from_name(particle.identifier.replace(':generic', '').replace(':semileptonic', '')))

        postCut_sig = postCutEfficiencies[particle.identifier][0] * mc_counts[particle_pdg] / 100.0
        postCut_candidates = postCut_sig / postCutEfficiencies[particle.identifier][1] * 100.0
        postCut_bkg = postCut_candidates - postCut_sig

        table.add(quantity='MC-Particles',
                  mean=mc_counts_mean[particle_pdg],
                  error=b2stat.poisson_error(mc_counts[particle_pdg]) / (1.0 * nEvents),
                  std=mc_counts_dev[particle_pdg],
                  min=mc_counts_min[particle_pdg],
                  max=mc_counts_max[particle_pdg],
                  meanPostCut=mc_counts_mean[particle_pdg],
                  errorPostCut=b2stat.poisson_error(mc_counts[particle_pdg]) / nEvents)
        table.add(quantity='Candidates',
                  mean=lc_mean['All'],
                  error=b2stat.poisson_error(lc_sum['All']) / nEvents,
                  std=lc_dev['All'],
                  min=lc_min['All'],
                  max=lc_max['All'],
                  meanPostCut=postCut_candidates / nEvents,
                  errorPostCut=b2stat.poisson_error(postCut_candidates) / nEvents)
        table.add(quantity='Candidates (Signal)',
                  mean=lc_mean['Signal'],
                  error=b2stat.poisson_error(lc_sum['Signal']) / nEvents,
                  std=lc_dev['Signal'],
                  min=lc_min['Signal'],
                  max=lc_max['Signal'],
                  meanPostCut=postCut_sig / nEvents,
                  errorPostCut=b2stat.poisson_error(postCut_sig) / nEvents)
        table.add(quantity='Candidates (Background)',
                  mean=lc_mean['Background'],
                  error=b2stat.poisson_error(lc_sum['Background']) / nEvents,
                  std=lc_dev['Background'],
                  min=lc_min['Background'],
                  max=lc_max['Background'],
                  meanPostCut=postCut_bkg / nEvents,
                  errorPostCut=b2stat.poisson_error(postCut_bkg) / nEvents)

        o += table.finish()

        table = b2latex.LongTable(columnspecs=r'p{5cm}rr',
                                  caption='Efficiencies and Purities in Percent',
                                  head=r'Cut & Efficiency $\pm$ Error & Purity $\pm$ Error',
                                  format_string=r'{cut} & $({efficiency:.2f} \pm {eerr:.2f})$ & $({purity:.2f} \pm {perr:.2f})$')
        table.add(cut='Reconstruction',
                  efficiency=detectorEfficiencies[particle.identifier]['__total__'][0] * 100.0,
                  eerr=detectorEff_Errors[particle.identifier]['__total__'][0] * 100.0,
                  purity=detectorEfficiencies[particle.identifier]['__total__'][1] * 100.0,
                  perr=detectorEff_Errors[particle.identifier]['__total__'][1] * 100.0)
        table.add(cut='Pre',
                  efficiency=preCutEfficiencies[particle.identifier]['__total__'][0],
                  eerr=preCutEff_Errors[particle.identifier]['__total__'][0],
                  purity=preCutEfficiencies[particle.identifier]['__total__'][1],
                  perr=preCutEff_Errors[particle.identifier]['__total__'][1])
        table.add(cut='User',
                  efficiency=userCutEfficiencies[particle.identifier]['__total__'][0] * 100.0,
                  eerr=userCutEff_Errors[particle.identifier]['__total__'][0] * 100.0,
                  purity=userCutEfficiencies[particle.identifier]['__total__'][1] * 100.0,
                  perr=userCutEff_Errors[particle.identifier]['__total__'][1] * 100.0)
        table.add(cut='Post',
                  efficiency=postCutEfficiencies[particle.identifier][0],
                  eerr=postCutEff_Errors[particle.identifier][0],
                  purity=postCutEfficiencies[particle.identifier][1],
                  perr=postCutEff_Errors[particle.identifier][1])

        o += table.finish()

        nTupleData, scale = loadNTupleDataFrame(nTuple)

        o += b2latex.SubSection("Diagonal plot")
        diagonalPlot = 'diag_' + filename
        print(diagonalPlot)
        plot = b2plot.Diagonal()
        plot.set_plot_options({'linestyle': '-', 'lw': 3})
        plot.add(nTupleData, 'extraInfo(SignalProbability)',
                 nTupleData[particle.mvaConfig.target] > 0, nTupleData[particle.mvaConfig.target] == 0)
        plot.finish()
        plot.save(diagonalPlot)
        o += b2latex.Graphics().add(diagonalPlot, width=0.7).finish()

        del nTupleData

        for i, channelName in enumerate(id_to_cnames[particle.identifier]):
            if channelName is not None:
                tmvaRanking = tmvaTrainings[particle.identifier][channelName]
                tmvaTraining = cnames_to_trainings[channelName]

            o += b2latex.SubSection(format.decayDescriptor(channelName)).finish()

            o += b2latex.SubSubSection("PreCut").finish()

            preCutDf = pandas.DataFrame()
            preCutCut = pre_cuts[channelName]

            hist_file_name = cnames_to_hist[channelName][0]
            prefix_list = ['signal', 'all', 'ratio']
            rootfile = ROOT.TFile(hist_file_name)
            for key in rootfile.GetListOfKeys():
                for prefix in prefix_list:
                    if prefix not in key.GetName():
                        continue
                    hist = rootfile.GetKey(key.GetName()).ReadObj()
                    size = hist.GetSize()
                    content_list = []
                    center_list = []
                    for i in range(size-2):
                        i += 1
                        content_list.append(hist.GetBinContent(i))
                        center_list.append(hist.GetBinCenter(i))
                    preCutDf[prefix] = center_list
                    preCutDf[prefix + '_weight'] = content_list

            o += b2latex.String(r"PreCut-Histograms").finish()

            # TODO: Fix box-plots (weights are not supportet, yet, but necessary here)
            # TODO: x-axis-label if preCutCut is None
            signalPlot = 'precut_signal_' + filename
            plot = b2plot.VerboseDistribution()
            plot.set_plot_options({'linestyle': '-', 'lw': 3})
            plot.add(preCutDf, 'signal', weight_column='signal_weight')
            plot.finish()
            plot.axis.set_title("Signal")
            plot.axis.get_yaxis().set_label_text('N')
            if preCutCut is not None:
                plot.axis.get_xaxis().set_label_text(preCutCut['variable'])
                for box_axis in plot.box_axes:
                    box_axis.get_xaxis().set_label_text(preCutCut['variable'])
                low, high = preCutCut['range']
                plot.axis.plot((low, low), (0.0, plot.ymax), color='black')
                plot.axis.plot((high, high), (0.0, plot.ymax), color='black')
            plot.save(signalPlot)

            allPlot = 'precut_all_' + filename
            plot = b2plot.VerboseDistribution()
            plot.set_plot_options({'linestyle': '-', 'lw': 3})
            plot.add(preCutDf, 'all', weight_column='all_weight')
            plot.finish()
            plot.axis.set_title("All")
            plot.axis.get_yaxis().set_label_text('N')
            if preCutCut is not None:
                plot.axis.get_xaxis().set_label_text(preCutCut['variable'])
                for box_axis in plot.box_axes:
                    box_axis.get_xaxis().set_label_text(preCutCut['variable'])
                low, high = preCutCut['range']
                plot.axis.plot((low, low), (0.0, plot.ymax), color='black')
                plot.axis.plot((high, high), (0.0, plot.ymax), color='black')
            plot.save(allPlot)

            ratioPlot = 'precut_ratio_' + filename
            plot = b2plot.VerboseDistribution()
            plot.set_plot_options({'linestyle': '-', 'lw': 3})
            plot.add(preCutDf, 'ratio', weight_column='ratio_weight')
            plot.finish()
            plot.axis.set_title("Ratio")
            plot.axis.get_yaxis().set_label_text('S/B')
            if preCutCut is not None:
                plot.axis.get_xaxis().set_label_text(preCutCut['variable'])
                for box_axis in plot.box_axes:
                    box_axis.get_xaxis().set_label_text(preCutCut['variable'])
                low, high = preCutCut['range']
                plot.axis.plot((low, low), (0.0, plot.ymax), color='black')
                plot.axis.plot((high, high), (0.0, plot.ymax), color='black')
            plot.save(ratioPlot)

            o += b2latex.Graphics().add(signalPlot, 0.49).add(allPlot, 0.49).add(ratioPlot, 0.49).finish()

            # TODO: Fix precut-sting being too long and no linebreak is applied
            if preCutCut is not None:
                o += b2latex.String(r"PreCut-String: \verb+" + preCutCut['cutstring'] + r"+")  # no finish
            else:
                o += b2latex.String(r"The channel was ignored. No PreCut is available, but the histograms are.").finish()
        #    else:
        #        o += b2latex.String(r"The channel was ignored. No PreCut histograms are available.").finish()

            del preCutDf

            o += b2latex.SubSubSection("MC-based MVA")
            if tmvaTraining is None:
                o += b2latex.String(r"""
                        There is no MC-based TMVA training data available for this final state particle.
                        Usually this means there wasn't enough statistics to perform a training,
                        or the training failed due to other reasons (cpu-, memory-, disk-, limitations?)
                        """)
            elif ':V0' in channelName:
                o += b2latex.String(r"""
                        There is no MC-based TMVA training data available for this :V0 particle.
                        """)
            else:
                TMVAFilename = tmvaTraining[:-5] + '_1.root'  # Strip .root of filename
                tmvaData, train_scale, test_scale = loadTMVADataFrame(TMVAFilename)
                tmvaTrainingData = tmvaData[tmvaData['__isTrain__']]
                tmvaTestData = tmvaData[~tmvaData['__isTrain__']]

                logFilename = tmvaTraining[:-5] + '.log'  # Strip .root of filename
                ranking = tmvaRanking

                table = b2latex.LongTable(columnspecs=r'lp{5cm}rrr',
                                          caption='List of variables used in the training',
                                          head=r'No. & Name & Importance & \multicolumn{2}{c}{mean $\pm$ std} \\'
                                               r' & & & Signal & Background ',
                                          format_string=r'{no} & {name} & {v:.2f} & $({ms:.3f} \pm {es:.3f})$'
                                                        r' & $({mb:.3f} \pm {eb:.3f})$')

                variable_list = []
                for number, (n, value) in enumerate(ranking):
                    variable_list.append(n)
                    table.add(no=number+1,
                              name=format.variable(format.string(n)),
                              v=value,
                              ms=tmvaData[tmvaData['__isSignal__']][n].mean(),
                              es=tmvaData[tmvaData['__isSignal__']][n].std(),
                              mb=tmvaData[~tmvaData['__isSignal__']][n].mean(),
                              eb=tmvaData[~tmvaData['__isSignal__']][n].std())
                for n in particle.mvaConfig.variables:
                    if n not in variable_list:
                        table.add(no='---',
                                  name=format.variable(format.string(n)),
                                  v=float('nan'),
                                  ms=float('nan'),
                                  es=float('nan'),
                                  mb=float('nan'),
                                  eb=float('nan'))
                o += table.finish()

                correlationMatrixPlotSignal = 'corMSig_' + filename
                plot = b2plot.CorrelationMatrix()
                plot.add(tmvaData[tmvaData['__isSignal__']], variable_list)
                plot.finish()
                plot.axis.set_title('Correlation Matrix for Signal')
                plot.save(correlationMatrixPlotSignal)

                correlationMatrixPlotBackground = 'corMBkg_' + filename
                plot = b2plot.CorrelationMatrix()
                plot.add(tmvaData[~tmvaData['__isSignal__']], variable_list)
                plot.finish()
                plot.axis.set_title('Correlation Matrix for Background')
                plot.save(correlationMatrixPlotBackground)

                o += b2latex.Graphics().add(correlationMatrixPlotSignal).add(correlationMatrixPlotBackground).finish()

                caption = 'Method {t} / {n} was used with the following configuration '.format(
                                          t=particle.mvaConfig.type,
                                          n=particle.mvaConfig.name)
                caption += format.variable(particle.mvaConfig.config)
                caption += r' and with target variable \emph{' + particle.mvaConfig.target + r'}'

                table = b2latex.LongTable(columnspecs=r'lp{5cm}rr',
                                          caption=caption,
                                          head=r'Name & Signal & Background',
                                          format_string=r'{name} & {signal:.0f} & {background:.0f}')
                table.add(name='Training',
                          signal=numpy.sum(tmvaTrainingData['__isSignal__']) * train_scale,
                          background=numpy.sum(~tmvaTestData['__isSignal__']) * test_scale)
                table.add(name='Test',
                          signal=numpy.sum(tmvaTestData['__isSignal__']) * train_scale,
                          background=numpy.sum(~tmvaTestData['__isSignal__']) * test_scale)
                o += table.finish()

                Con_name = particle.mvaConfig.name

                mvaROCPlt = 'mva_roc_' + filename
                plot = b2plot.RejectionOverEfficiency()
                plot.set_plot_options({'linestyle': '-', 'lw': 3})
                plot.add(tmvaTestData, particle.mvaConfig.name, tmvaTestData['__isSignal__'], ~tmvaTestData['__isSignal__'])
                plot.add(tmvaTrainingData, Con_name, tmvaTrainingData['__isSignal__'], ~tmvaTrainingData['__isSignal__'])
                plot.labels = ["Test", "Training"]
                plot.finish()
                plot.save(mvaROCPlot)

                mvaDiagPlt = 'mva_diag_' + filename
                plot = b2plot.Diagonal()
                plot.set_plot_options({'linestyle': '-', 'lw': 3})
                plot.add(tmvaTestData, particle.mvaConfig.name, tmvaTestData['__isSignal__'], ~tmvaTestData['__isSignal__'])
                plot.add(tmvaTrainingData, Con_name, tmvaTrainingData['__isSignal__'], ~tmvaTrainingData['__isSignal__'])
                plot.labels = ["Test", "Training"]
                plot.finish()
                plot.save(mvaDiagPlot)

                mvaOTrainPlt = 'mva_overtraining_' + filename
                plot = b2plot.Overtraining()
                plot.set_plot_options({'linestyle': '-', 'lw': 3})
                plot.add(tmvaData, 'FastBDT',
                         tmvaData['__isTrain__'], ~tmvaData['__isTrain__'],
                         tmvaData['__isSignal__'], ~tmvaData['__isSignal__'])
                plot.finish()
                plot.save(mvaOvertrainingPlot)

                o += b2latex.Graphics().add(mvaOTrainPlt, width=0.7).add(mvaROCPlt, width=0.7).add(mvaDiagPlt, width=0.7).finish()

        print(particle.identifier + ' done...')

#    out.write('\n\n# Indivual cuts of particles \n')
#    for particle, precut, postcut in zip(obj['particles'], obj['pre_cuts'], obj['post_cuts']):
#        if precut is None or postcut is None:
#            out.write(particle.identifier + ' was ignored\n')
#            continue
#        out.write(particle.identifier + ' PostCut ' + postcut['cutstring'] + '\n')
#        for channel, cut in precut.items():
#            if cut is None:
#                out.write('\t' + channel + ' was ignored\n')
#            else:
#                out.write('\t' + channel + ' ' + cut['cutstring'] + '\n')

    o.save(sys.argv[2], compile=True)
