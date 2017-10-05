#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Thomas Keck 2017

"""
 Call this "python3 fei/latexReporting.py summary.tex"
 in a directory containing the monitoring output of the FEI
 It will create a latex document containing a summary and plots
 and tries to compile this summary.tex into a PDF file summary.pdf

 You can improvide modify this script
 E.g. If you want to add new plots:
 Add your plot in the monitoring.py file
 Add your plot below using b2latex.Graphics
"""


import ROOT
from ROOT import Belle2

from fei import monitoring

from B2Tools import b2latex
from B2Tools import format

import pickle
import sys
import glob


if __name__ == '__main__':

    particles, configuration = monitoring.load_config()
    monitoringParticle = []
    for particle in particles:
        monitoringParticle.append(monitoring.MonitoringParticle(particle))

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
        table.add(name=format.decayDescriptor(p.particle.identifier),
                  exc_br=sum(p.exc_br_per_channel.values()),
                  inc_br=sum(p.inc_br_per_channel.values()),
                  user_pre_cut=sum(p.before_ranking.values()).efficiency,
                  ranking_pre_cut=sum(p.after_ranking.values()).efficiency,
                  vertex_pre_cut=sum(p.after_vertex.values()).efficiency,
                  absolute_post_cut=p.before_ranking_postcut.efficiency,
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
        table.add(name=format.decayDescriptor(p.particle.identifier),
                  user_pre_cut=sum(p.before_ranking.values()).purity,
                  ranking_pre_cut=sum(p.after_ranking.values()).purity,
                  vertex_pre_cut=sum(p.after_vertex.values()).purity,
                  absolute_post_cut=p.before_ranking_postcut.purity,
                  ranking_post_cut=p.after_ranking_postcut.purity,
                  after_tag=p.after_tag.purity)
    o += table.finish()

    # If you change the number of colors, than change below \ifnum5 accordingly
    moduleTypes = ['ParticleCombiner', 'MVAExpert', 'MCMatch', 'ParticleVertexFitter', 'BestCandidateSelection', 'Other']

    o += b2latex.Section("CPU time").finish()
    colour_list = b2latex.DefineColourList()
    o += colour_list.finish()

    for p in monitoringParticle:
        o += b2latex.SubSection(format.decayDescriptor(p.particle.identifier)).finish()

        table = b2latex.LongTable(columnspecs=r'lrcrr',
                                  caption='Total CPU time spent in event() calls for each channel. Bars show ' +
                                          ', '.join('\\textcolor{%s}{%s}' % (c, m)
                                                    for c, m in zip(colour_list.colours, moduleTypes)) +
                                          ', in this order. Does not include I/O, initialisation, training, post-cuts etc.',
                                  head=r'Decay & CPU time & by module & per (true) candidate & Relative time ',
                                  format_string=r'{name} & {time} & {bargraph} & {timePerCandidate} & {timePercent:.2f}\% ')

        tt_channel = sum(p.module_statistic.channel_time.values())
        tt_particle = p.module_statistic.particle_time + sum(p.module_statistic.channel_time.values())
        fraction = tt_channel / tt_particle * 100 if tt_particle > 0 else 0.0

        for channel in p.particle.channels:
            time = p.time_per_channel[channel.label]
            trueCandidates = p.after_classifier[channel.label].nSig
            allCandidates = p.after_classifier[channel.label].nTotal

            if trueCandidates == 0 or allCandidates == 0:
                continue

            timePerCandidate = format.duration(time / trueCandidates) + ' (' + format.duration(time / allCandidates) + ')'
            timePercent = time / tt_particle * 100 if tt_particle > 0 else 0

            percents = [p.module_statistic.channel_time_per_module[channel.label].get(key, 0.0) / float(time) * 100.0
                        if time > 0 else 0.0 for key in moduleTypes[:-1]]
            percents.append(100.0 - sum(percents))

            table.add(name=format.decayDescriptor(channel.label),
                      bargraph=r'\plotbar{ %g/, %g/, %g/, %g/, %g/, %g/, }' % tuple(percents),
                      time=format.duration(time),
                      timePerCandidate=timePerCandidate,
                      timePercent=time / tt_particle * 100 if p.total_time > 0 else 0)

        o += table.finish(tail='Total & & {tt_channel} / {tt_particle} & & {fraction:.2f}'.format(
            tt_channel=format.duration(tt_channel), tt_particle=format.duration(tt_particle), fraction=fraction))

    for p in monitoringParticle:
        print(p.particle.identifier)

        o += b2latex.Section(format.decayDescriptor(p.particle.identifier)).finish()
        string = b2latex.String(r"In the reconstruction of {name} {nChannels} out of {max_nChannels} possible channels were used. "
                                r"The covered inclusive / exclusive branching fractions is {inc_br:.5f} / {exc_br:.5f}."
                                r"The final unqiue efficiency and purity was {eff:.5f} / {pur:.5f}")

        o += string.finish(name=format.decayDescriptor(p.particle.identifier),
                           nChannels=p.reconstructed_number_of_channels,
                           max_nChannels=p.total_number_of_channels,
                           exc_br=sum(p.exc_br_per_channel.values()),
                           inc_br=sum(p.inc_br_per_channel.values()),
                           eff=p.after_tag.efficiency,
                           pur=p.after_tag.purity)

        roc_plot_filename = monitoring.removeJPsiSlash(p.particle.identifier + '_ROC')
        monitoring.MonitorROCPlot(p, roc_plot_filename)
        o += b2latex.Graphics().add(roc_plot_filename + '.png', width=0.8).finish()

        diag_plot_filename = monitoring.removeJPsiSlash(p.particle.identifier + '_Diag')
        monitoring.MonitorDiagPlot(p, diag_plot_filename)
        o += b2latex.Graphics().add(diag_plot_filename + '.png', width=0.8).finish()

        if p.particle.identifier in ['B+:generic', 'B0:generic']:
            money_plot_filename = monitoring.removeJPsiSlash(p.particle.identifier + '_Money')
            monitoring.MonitorMbcPlot(p, money_plot_filename)
            g = b2latex.Graphics()
            for filename in glob.glob(money_plot_filename + '_*.png'):
                g.add(filename, width=0.49)
            o += g.finish()

        if p.particle.identifier in ['B+:semileptonic', 'B0:semileptonic']:
            money_plot_filename = monitoring.removeJPsiSlash(p.particle.identifier + '_Money')
            monitoring.MonitorCosBDLPlot(p, money_plot_filename)
            g = b2latex.Graphics()
            for filename in glob.glob(money_plot_filename + '_*.png'):
                g.add(filename, width=0.49)
            o += g.finish()

        table = b2latex.LongTable(columnspecs=r'c|rr|rrr',
                                  caption='Per-channel efficiency before and after the applied pre-cut.',
                                  head=r'Particle & \multicolumn{2}{c}{Covered BR} '
                                       r' & \multicolumn{3}{c}{pre-cut} \\'
                                       r' & exc & inc  & user & ranking & vertex ',
                                  format_string=r'{name} & {exc_br:.3f} & {inc_br:.3f} & {user_pre_cut:.5f} & '
                                                r'{ranking_pre_cut:.5f} & {vertex_pre_cut:.5f}')

        for channel in p.particle.channels:
            table.add(name=format.decayDescriptor(channel.label),
                      exc_br=p.exc_br_per_channel[channel.label],
                      inc_br=p.inc_br_per_channel[channel.label],
                      user_pre_cut=p.before_ranking[channel.label].efficiency,
                      ranking_pre_cut=p.after_ranking[channel.label].efficiency,
                      vertex_pre_cut=p.after_vertex[channel.label].efficiency,
                      absolute_post_cut=p.before_ranking_postcut.efficiency,
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

        for channel in p.particle.channels:
            table.add(name=format.decayDescriptor(channel.label),
                      ignored=r'\textcolor{red}{$\blacksquare$}' if p.ignored_channels[channel.label] else '',
                      user_pre_cut=p.before_ranking[channel.label].purity,
                      ranking_pre_cut=p.after_ranking[channel.label].purity,
                      vertex_pre_cut=p.after_vertex[channel.label].purity)
        o += table.finish()

    o.save(sys.argv[1], compile=True)
