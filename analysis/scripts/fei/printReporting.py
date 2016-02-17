#!/usr/bin/env python
# -*- coding: utf-8 -*-

from fei import monitoring
import pickle

import sys


def bold(text):
    """Use ANSI sequence to show string in bold"""
    return '\x1b[1m' + text + '\x1b[0m'


def print_summary(p):
    monitoring.MonitorROCPlot(p, monitoring.removeJPsiSlash(p.identifier + '_ROC.png'))
    monitoring.MonitorDiagPlot(p, monitoring.removeJPsiSlash(p.identifier + '_Diag.png'))
    if p.identifier in ['B+:generic', 'B0:generic']:
        monitoring.MonitorMbcPlot(p, monitoring.removeJPsiSlash(p.identifier + '_Money.png'))
    if p.identifier in ['B+:semileptonic', 'B0:semileptonic']:
        monitoring.MonitorCosBDLPlot(p, monitoring.removeJPsiSlash(p.identifier + '_Money.png'))

    print(bold(p.identifier))
    print('Total cpu time spent reconstructing this particle: ', p.total_time)
    print('Total amount of Monte Carlo particles in training sample: ', p.mc_count)
    print('Number of decay channels: ', p.reconstructed_number_of_channels, '/', p.total_number_of_channels)
    print(bold('PreCut'))
    print('UserCut', p.preCutConfig.userCut)
    print('BestCandidateCut', p.preCutConfig.bestCandidateVariable,
          p.preCutConfig.bestCandidateCut, p.preCutConfig.bestCandidateMode)
    print('VertexCut', p.preCutConfig.vertexCut)
    print('Stats before ranking')
    print(sum(p.before_ranking.values()))
    print('Stats after ranking')
    print(sum(p.after_ranking.values()))
    print('Stats after match')
    print(sum(p.after_match.values()))
    print('Stats before vertex')
    print(sum(p.before_vertex.values()))
    print('Stats after vertex')
    print(sum(p.after_vertex.values()))
    print('Stats after classifier')
    print(sum(p.after_classifier.values()))
    print(bold('PostCut'))
    print('Absolute', p.postCutConfig.value)
    print('Ranking', p.postCutConfig.bestCandidateCut)
    print('Stats before postcut')
    print(p.before_postcut)
    print('Stats after absolute postcut')
    print(p.after_abs_postcut)
    print('Stats after ranking postcut')
    print(p.after_ranking_postcut)
    print(bold('Tag unique signal'))
    print('Stats before tag')
    print(p.before_tag)
    print('Stats after tag')
    print(p.after_tag)
    print(bold('Multivariate classifier'))
    print('Method', p.mvaConfig.name)
    print('Target', p.mvaConfig.target)
    print(bold('Individual channels'))
    for channel in p.channels:
        print(bold(channel.name))
        print('Stats before ranking')
        print(p.before_ranking[channel.name])
        print('Stats after ranking')
        print(p.after_ranking[channel.name])
        print('Stats after match')
        print(p.after_match[channel.name])
        print('Stats before vertex')
        print(p.before_vertex[channel.name])
        print('Stats after vertex')
        print(p.after_vertex[channel.name])
        print('Stats after classifier')
        print(p.after_classifier[channel.name])


if __name__ == '__main__':

    if len(sys.argv) != 2:
        print("Usage: " + sys.argv[0] + ' SummaryFile.pickle')
        sys.exit(1)

    obj = pickle.load(open(sys.argv[1], 'rb'))
    for particle in obj['particles']:
        monitoringParticle = monitoring.MonitoringParticle(particle, obj)
        print_summary(monitoringParticle)
