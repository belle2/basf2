#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT
from ROOT import Belle2

from fei import monitoring

from B2Tools import b2latex
from B2Tools import format

import pickle
import sys


if __name__ == '__main__':

    if len(sys.argv) != 3:
        print("Usage: " + sys.argv[0] + ' SummaryFile.pickle summary.tex')
        sys.exit(1)

    obj = pickle.load(open(sys.argv[1], 'rb'))
    monitoringParticle = []
    for particle in obj['particles']:
        monitoringParticle.append(monitoring.MonitoringParticle(particle, obj))

    # Create latex file
    o = b2latex.LatexFile()

    o += b2latex.TitlePage(title='Full Event Interpretation Report',
                           authors=['Thomas Keck', 'Christian Pulvermacher'],
                           abstract=r"""
                           This report contains key performance indicators and control plots of the Full Event Interpretation.
                           The pre-, and post-cuts as well as trained multivariate selection methods are described.
                           Furthermore the resulting purities and efficiencies are stated.
                              """,
                           add_table_of_contents=True).finish()

    o += b2latex.Section("Summary").finish()
    o += b2latex.String(r"""
        For each decay channel of each particle a multivariate selection method is trained after applying
        a fast pre-cut on the candidates. Afterwards, a post-cut is applied on the signal probability calculated by the method.
        This reduces combinatorics in the following stages of the Full
        Event Interpretation.
        """).finish()

    table = b2latex.LongTable(columnspecs=r'c|rr|rrrrrr',
                              caption='Per-particle efficiency before and after the applied pre- and post-cut.',
                              head=r'Particle & \multicolumn{2}{c}{Covered BR} '
                                   r' & \multicolumn{3}{c}{pre-cut}  &  \multicolumn{3}{c}{post-cut} \\'
                                   r' & exc & inc  & user & ranking & vertex '
                                   r' & absolute & ranking & unique',
                              format_string=r'{name} & {exc_br:.3f} & {inc_br:.3f} & {user_pre_cut:.3f} & {ranking_pre_cut:.3f}'
                                            r' & {vertex_pre_cut:.3f}  & {absolute_post_cut:.3f}'
                                            r' & {ranking_post_cut:.3f} & {after_tag:.3f}')

    for p in monitoringParticle:
        table.add(name=format.decayDescriptor(p.identifier),
                  exc_br=sum(p.exc_br_per_channel.values()),
                  inc_br=sum(p.inc_br_per_channel.values()),
                  user_pre_cut=sum(p.before_ranking.values()).efficiency,
                  ranking_pre_cut=sum(p.after_ranking.values()).efficiency,
                  vertex_pre_cut=sum(p.after_vertex.values()).efficiency,
                  absolute_post_cut=p.after_abs_postcut.efficiency,
                  ranking_post_cut=p.after_ranking_postcut.efficiency,
                  after_tag=p.after_tag.efficiency)
    o += table.finish()

    table = b2latex.LongTable(columnspecs=r'c|rrrrrr',
                              caption='Per-particle purity before and after the applied pre- and post-cut.',
                              head=r'Particle '
                                   r' & \multicolumn{3}{c}{pre-cut}  &  \multicolumn{3}{c}{post-cut} \\'
                                   r' & user & ranking & vertex '
                                   r' & absolute & ranking & unique',
                              format_string=r'{name} & {user_pre_cut:.3f} & {ranking_pre_cut:.3f}'
                                            r' & {vertex_pre_cut:.3f}  & {absolute_post_cut:.3f}'
                                            r' & {ranking_post_cut:.3f} & {after_tag:.3f}')

    for p in monitoringParticle:
        table.add(name=format.decayDescriptor(p.identifier),
                  user_pre_cut=sum(p.before_ranking.values()).purity,
                  ranking_pre_cut=sum(p.after_ranking.values()).purity,
                  vertex_pre_cut=sum(p.after_vertex.values()).purity,
                  absolute_post_cut=p.after_abs_postcut.purity,
                  ranking_post_cut=p.after_ranking_postcut.purity,
                  after_tag=p.after_tag.purity)
    o += table.finish()

    # If you change the number of colors, than change below \ifnum5 accordingly
    moduleTypes = ['ParticleCombiner', 'MVAExpert', 'MCMatch', 'ParticleVertexFitter', 'BestCandidateSelection', 'Other']

    o += b2latex.Section("CPU time").finish()
    colour_list = b2latex.DefineColourList()
    o += colour_list.finish()

    for p in monitoringParticle:
        o += b2latex.SubSection(format.decayDescriptor(p.identifier)).finish()

        table = b2latex.LongTable(columnspecs=r'lrcrr',
                                  caption='Total CPU time spent in event() calls for each channel. Bars show ' +
                                          ', '.join('\\textcolor{%s}{%s}' % (c, m)
                                                    for c, m in zip(colour_list.colours, moduleTypes)) +
                                          ', in this order. Does not include I/O, initialisation, training, post-cuts etc.',
                                  head=r'Decay & CPU time & by module & per (true) candidate & Relative time ',
                                  format_string=r'{name} & {time} & {bargraph} & {timePerCandidate} & {timePercent:.2f}\% ')

        tt_channel = sum(p.time_per_channel.values())
        tt_particle = p.total_time
        fraction = tt_channel / tt_particle * 100 if tt_particle > 0 else 0.0

        for channel in p.channels:
            time = p.time_per_channel[channel.name]
            trueCandidates = p.after_classifier[channel.name].nSig
            allCandidates = p.after_classifier[channel.name].nTotal

            if trueCandidates == 0 or allCandidates == 0:
                continue

            timePerCandidate = format.duration(time / trueCandidates) + ' (' + format.duration(time / allCandidates) + ')'
            timePercent = time / tt_particle * 100 if tt_particle > 0 else 0

            percents = [p.time_per_channel_per_module[channel.name][key] / float(time) * 100.0 if time > 0 else 0.0
                        for key in moduleTypes[:-1]]
            percents.append(100.0 - sum(percents))

            table.add(name=format.decayDescriptor(channel.name),
                      bargraph=r'\plotbar{ %g/, %g/, %g/, %g/, %g/, %g/, }' % tuple(percents),
                      time=format.duration(time),
                      timePerCandidate=timePerCandidate,
                      timePercent=time / p.total_time * 100 if p.total_time > 0 else 0)

        o += table.finish(tail='Total & & {tt_channel} / {tt_particle} & & {fraction:.2f}'.format(
            tt_channel=format.duration(tt_channel), tt_particle=format.duration(tt_particle), fraction=fraction))

    for p in monitoringParticle:
        print(p.identifier)
        # TODO Print config

        o += b2latex.Section(format.decayDescriptor(p.identifier)).finish()
        string = b2latex.String(r"In the reconstruction of {name} {nChannels} out of {max_nChannels} possible channels were used. "
                                r"The covered inclusive / exclusive branching fractions is {inc_br:.5f} / {exc_br:.5f}."
                                r"The final unqiue efficiency and purity was {eff:.5f} / {pur:.5f}")

        o += string.finish(name=format.decayDescriptor(p.identifier),
                           nChannels=p.reconstructed_number_of_channels,
                           max_nChannels=p.total_number_of_channels,
                           exc_br=sum(p.exc_br_per_channel.values()),
                           inc_br=sum(p.inc_br_per_channel.values()),
                           eff=p.after_tag.efficiency,
                           pur=p.after_tag.purity)

        roc_plot_filename = monitoring.removeJPsiSlash(p.identifier + '_ROC.png')
        monitoring.MonitorROCPlot(p, roc_plot_filename)
        diag_plot_filename = monitoring.removeJPsiSlash(p.identifier + '_Diag.png')
        monitoring.MonitorDiagPlot(p, diag_plot_filename)
        o += b2latex.Graphics().add(diag_plot_filename, width=0.49).add(roc_plot_filename, width=0.49).finish()

        if p.identifier in ['B+:generic', 'B0:generic']:
            money_plot_filename = monitoring.removeJPsiSlash(p.identifier + '_Money.png')
            monitoring.MonitorMbcPlot(p, money_plot_filename)
            o += b2latex.Graphics().add(money_plot_filename, width=0.8).finish()
        if p.identifier in ['B+:semileptonic', 'B0:semileptonic']:
            money_plot_filename = monitoring.removeJPsiSlash(p.identifier + '_Money.png')
            monitoring.MonitorCosBDLPlot(p, money_plot_filename)
            o += b2latex.Graphics().add(money_plot_filename, width=0.8).finish()

        table = b2latex.LongTable(columnspecs=r'c|rr|rrr',
                                  caption='Per-channel efficiency before and after the applied pre-cut.',
                                  head=r'Particle & \multicolumn{2}{c}{Covered BR} '
                                       r' & \multicolumn{3}{c}{pre-cut} \\'
                                       r' & exc & inc  & user & ranking & vertex ',
                                  format_string=r'{name} & {exc_br:.3f} & {inc_br:.3f} & {user_pre_cut:.5f} & '
                                                r'{ranking_pre_cut:.5f} & {vertex_pre_cut:.5f}')

        for channel in p.channels:
            table.add(name=format.decayDescriptor(channel.name),
                      exc_br=p.exc_br_per_channel[channel.name],
                      inc_br=p.inc_br_per_channel[channel.name],
                      user_pre_cut=p.before_ranking[channel.name].efficiency,
                      ranking_pre_cut=p.after_ranking[channel.name].efficiency,
                      vertex_pre_cut=p.after_vertex[channel.name].efficiency,
                      absolute_post_cut=p.after_abs_postcut.efficiency,
                      ranking_post_cut=p.after_ranking_postcut.efficiency,
                      after_tag=p.after_tag.efficiency)
        o += table.finish()

        table = b2latex.LongTable(columnspecs=r'c|c|rrr',
                                  caption='Per-channel purity before and after the applied pre-cut.',
                                  head=r'Particle & Ignored '
                                       r' & \multicolumn{3}{c}{pre-cut} \\'
                                       r' &&  user & ranking & vertex ',
                                  format_string=r'{name} & {ignored} & {user_pre_cut:.5f} & {ranking_pre_cut:.5f}'
                                                r' & {vertex_pre_cut:.5f}')

        for channel in p.channels:
            table.add(name=format.decayDescriptor(channel.name),
                      ignored=r'\textcolor{red}{$\blacksquare$}' if p.ignored_channels[channel.name] else '',
                      user_pre_cut=p.before_ranking[channel.name].purity,
                      ranking_pre_cut=p.after_ranking[channel.name].purity,
                      vertex_pre_cut=p.after_vertex[channel.name].purity)
        o += table.finish()

    o.save(sys.argv[2], compile=True)
