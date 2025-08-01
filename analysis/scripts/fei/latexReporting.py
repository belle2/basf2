#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
 Call this "python3 fei/latexReporting.py summary.tex"
 in a directory containing the monitoring output of the FEI
 It will create a latex document containing a summary and plots
 and tries to compile this summary.tex into a PDF file summary.pdf

 You can improve / modify this script
 E.g. If you want to add new plots:
 Add your plot in the monitoring.py file
 Add your plot below using b2latex.Graphics
"""


from fei import monitoring
from fei.core import get_stages_from_particles

from B2Tools import b2latex
from B2Tools import format

import sys
import glob


def create_latex(output_file, monitoringParticle):
    # Create latex file
    o = b2latex.LatexFile()

    o += b2latex.TitlePage(title='Full Event Interpretation Report',
                           authors=['Thomas Keck', 'Christian Pulvermacher', 'William Sutcliffe'],
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
                              format_string=r'{name} & {exc_br} & {inc_br} & {user_pre_cut:.3f} & {ranking_pre_cut:.3f}'
                                            r' & {vertex_pre_cut:.3f}  & {absolute_post_cut:.3f}'
                                            r' & {ranking_post_cut:.3f} & {after_tag:.3f}')
    for p in monitoringParticle:
        table.add(name=format.decayDescriptor(p.particle.identifier),
                  exc_br=f"{sum(p.exc_br_per_channel.values()):.3f}",
                  inc_br=f"{sum(p.inc_br_per_channel.values()):.3f}",
                  user_pre_cut=sum(p.before_ranking.values()).efficiency,
                  ranking_pre_cut=sum(p.after_ranking.values()).efficiency,
                  vertex_pre_cut=sum(p.after_vertex.values()).efficiency,
                  absolute_post_cut=p.before_ranking_postcut.efficiency,
                  ranking_post_cut=p.after_ranking_postcut.efficiency,
                  after_tag=p.after_tag.efficiency)
        table.add(name="",
                  exc_br="",
                  inc_br="",
                  user_pre_cut=sum(p.before_ranking.values()).efficiency,
                  ranking_pre_cut=sum(p.after_ranking.values()).nSig / sum(p.before_ranking.values()).nSig,
                  vertex_pre_cut=sum(p.after_vertex.values()).nSig / sum(p.after_ranking.values()).nSig,
                  absolute_post_cut=p.before_ranking_postcut.nSig / sum(p.after_vertex.values()).nSig,
                  ranking_post_cut=p.after_ranking_postcut.nSig / p.before_ranking_postcut.nSig,
                  after_tag=p.after_tag.nSig / p.after_ranking_postcut.nSig)
    o += table.finish()

    table = b2latex.LongTable(columnspecs=r'c|rrrrrr',
                              caption='Per-particle nSignal before and after the applied pre- and post-cut.',
                              head=r'Particle '
                                   r' & \multicolumn{3}{c}{pre-cut}  &  \multicolumn{3}{c}{post-cut} \\'
                                   r' & user & ranking & vertex '
                                   r' & absolute & ranking & unique',
                              format_string=r'{name} & {user_pre_cut:.4e} & {ranking_pre_cut:.4e}'
                                            r' & {vertex_pre_cut:.4e}  & {absolute_post_cut:.4e}'
                                            r' & {ranking_post_cut:.4e} & {after_tag:.4e}')
    for p in monitoringParticle:
        table.add(name=format.decayDescriptor(p.particle.identifier),
                  user_pre_cut=sum(p.before_ranking.values()).nSig,
                  ranking_pre_cut=sum(p.after_ranking.values()).nSig,
                  vertex_pre_cut=sum(p.after_vertex.values()).nSig,
                  absolute_post_cut=p.before_ranking_postcut.nSig,
                  ranking_post_cut=p.after_ranking_postcut.nSig,
                  after_tag=p.after_tag.nSig)
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

    table = b2latex.LongTable(columnspecs=r'c|rrrrrr',
                              caption='Per-particle nBackground before and after the applied pre- and post-cut.',
                              head=r'Particle '
                                   r' & \multicolumn{3}{c}{pre-cut}  &  \multicolumn{3}{c}{post-cut} \\'
                                   r' & user & ranking & vertex '
                                   r' & absolute & ranking & unique',
                              format_string=r'{name} & {user_pre_cut:.4e} & {ranking_pre_cut:.4e}'
                                            r' & {vertex_pre_cut:.4e}  & {absolute_post_cut:.4e}'
                                            r' & {ranking_post_cut:.4e} & {after_tag:.4e}')
    for p in monitoringParticle:
        table.add(name=format.decayDescriptor(p.particle.identifier),
                  user_pre_cut=sum(p.before_ranking.values()).nBg,
                  ranking_pre_cut=sum(p.after_ranking.values()).nBg,
                  vertex_pre_cut=sum(p.after_vertex.values()).nBg,
                  absolute_post_cut=p.before_ranking_postcut.nBg,
                  ranking_post_cut=p.after_ranking_postcut.nBg,
                  after_tag=p.after_tag.nBg)
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
                                          ', '.join(f'\\textcolor{{{c}}}{{{m}}}'
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
            # timePercent = time / tt_particle * 100 if tt_particle > 0 else 0 # is this redundant?

            percents = [p.module_statistic.channel_time_per_module[channel.label].get(key, 0.0) / float(time) * 100.0
                        if time > 0 else 0.0 for key in moduleTypes[:-1]]
            percents.append(100.0 - sum(percents))

            table.add(name=format.decayDescriptor(channel.label),
                      bargraph=r'\plotbar{ %g/, %g/, %g/, %g/, %g/, %g/, }' % tuple(percents),
                      time=format.duration(time),
                      timePerCandidate=timePerCandidate,
                      timePercent=time / tt_particle * 100 if p.total_time > 0 else 0)

        o += table.finish(tail=f'Total & & {format.duration(tt_channel)} / {format.duration(tt_particle)} & & {fraction:.2f}')

    for p in monitoringParticle:
        print(p.particle.identifier)

        o += b2latex.Section(format.decayDescriptor(p.particle.identifier)).finish()
        string = b2latex.String(r"In the reconstruction of {name} {nChannels} out of {max_nChannels} possible channels were used. "
                                r"The covered inclusive / exclusive branching fractions is {inc_br:.5f} / {exc_br:.5f}."
                                r"The final unique efficiency and purity was {eff:.5f} / {pur:.5f}")

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

        sigprob_plot_filename = monitoring.removeJPsiSlash(p.particle.identifier + '_SigProb')
        monitoring.MonitorSigProbPlot(p, sigprob_plot_filename)
        o += b2latex.Graphics().add(sigprob_plot_filename + '.png', width=0.8).finish()

        for spectator in p.particle.mvaConfig.spectators.keys():
            money_plot_filename = monitoring.removeJPsiSlash(p.particle.identifier + '_' + spectator + '_Money')
            monitoring.MonitorSpectatorPlot(p, spectator, money_plot_filename, p.particle.mvaConfig.spectators[spectator])
            g = b2latex.Graphics()
            for filename in glob.glob(money_plot_filename + '_*.png'):
                g.add(filename, width=0.49)
            o += g.finish()

        table = b2latex.LongTable(columnspecs=r'c|rr|rrr',
                                  caption='Per-channel efficiency before and after the applied pre-cut.',
                                  head=r'Particle & \multicolumn{2}{c}{Covered BR} '
                                       r' & \multicolumn{3}{c}{pre-cut} \\'
                                       r' & exc & inc  & user & ranking & vertex ',
                                  format_string=r'{name} & {exc_br} & {inc_br} & {user_pre_cut:.5f} & '
                                                r'{ranking_pre_cut:.5f} & {vertex_pre_cut:.5f}')

        for channel in p.particle.channels:
            table.add(name=format.decayDescriptor(channel.label),
                      exc_br=f"{p.exc_br_per_channel[channel.label]:.3f}",
                      inc_br=f"{p.inc_br_per_channel[channel.label]:.3f}",
                      user_pre_cut=p.before_ranking[channel.label].efficiency,
                      ranking_pre_cut=p.after_ranking[channel.label].efficiency,
                      vertex_pre_cut=p.after_vertex[channel.label].efficiency,
                      absolute_post_cut=p.before_ranking_postcut.efficiency,
                      ranking_post_cut=p.after_ranking_postcut.efficiency,
                      after_tag=p.after_tag.efficiency)
            table.add(name="",
                      exc_br="",
                      inc_br="",
                      user_pre_cut=p.before_ranking[channel.label].efficiency,
                      ranking_pre_cut=p.after_ranking[channel.label].nSig / p.before_ranking[channel.label].nSig,
                      vertex_pre_cut=p.after_vertex[channel.label].nSig / p.after_ranking[channel.label].nSig,
                      absolute_post_cut=p.before_ranking_postcut.nSig / p.after_vertex[channel.label].nSig,
                      ranking_post_cut=p.after_ranking_postcut.nSig / p.before_ranking_postcut.nSig,
                      after_tag=p.after_tag.nSig / p.after_ranking_postcut.nSig)
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

        table = b2latex.LongTable(columnspecs=r'c|rrr',
                                  caption='Per-channel nSignal before and after the applied pre-cut.',
                                  head=r'Particle '
                                  r' & \multicolumn{3}{c}{pre-cut} \\'
                                  r' & user & ranking & vertex ',
                                  format_string=r'{name} & {user_pre_cut:.4e} & '
                                  r'{ranking_pre_cut:.4e} & {vertex_pre_cut:.4e}')
        for channel in p.particle.channels:
            table.add(name=format.decayDescriptor(channel.label),
                      user_pre_cut=p.before_ranking[channel.label].nSig,
                      ranking_pre_cut=p.after_ranking[channel.label].nSig,
                      vertex_pre_cut=p.after_vertex[channel.label].nSig)
        o += table.finish()

        table = b2latex.LongTable(columnspecs=r'c|rrr',
                                  caption='Per-channel nBackground before and after the applied pre-cut.',
                                  head=r'Particle '
                                  r' & \multicolumn{3}{c}{pre-cut} \\'
                                  r' & user & ranking & vertex ',
                                  format_string=r'{name} & {user_pre_cut:.4e} & '
                                  r'{ranking_pre_cut:.4e} & {vertex_pre_cut:.4e}')
        for channel in p.particle.channels:
            table.add(name=format.decayDescriptor(channel.label),
                      user_pre_cut=p.before_ranking[channel.label].nBg,
                      ranking_pre_cut=p.after_ranking[channel.label].nBg,
                      vertex_pre_cut=p.after_vertex[channel.label].nBg)
        o += table.finish()

    o.save(output_file, compile=False)


# =============================================================================
if __name__ == '__main__':
    try:
        output_file = sys.argv[1]
    except IndexError:
        raise AttributeError("You have to supply the output tex file.")

    particles, configuration = monitoring.load_config()
    cache = configuration.cache
    stages = get_stages_from_particles(particles)
    monitoringParticle = []
    for i in range(cache):
        for particle in particles:
            if particle in stages[i]:
                print('FEI: latexReporting: ', i, particle.identifier)
                monitoringParticle.append(monitoring.MonitoringParticle(particle))
    create_latex(output_file, monitoringParticle)
