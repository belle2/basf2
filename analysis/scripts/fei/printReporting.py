#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
 Call this "python3 fei/printReporting.py"
 in a directory containing the monitoring output of the FEI
 It will print out a summary and create some plots
"""

import sys
from contextlib import redirect_stdout

import fei.monitoring as monitoring
from fei.core import get_stages_from_particles


def bold(text):
    """Use ANSI sequence to show string in bold"""
    return '\x1b[1m' + text + '\x1b[0m'


def print_summary(p):
    try:
        print("FEI: printReporting - DEBUG: ", p.particle.identifier)
        monitoring.MonitorROCPlot(p, monitoring.removeJPsiSlash(p.particle.identifier + '_ROC'))
        monitoring.MonitorDiagPlot(p, monitoring.removeJPsiSlash(p.particle.identifier + '_Diag'))
        monitoring.MonitorSigProbPlot(p, monitoring.removeJPsiSlash(p.particle.identifier + '_SigProb'))
        for spectator in p.particle.mvaConfig.spectators.keys():
            monitoring.MonitorSpectatorPlot(
                p, spectator, monitoring.removeJPsiSlash(
                    p.particle.identifier + '_' + spectator + '_Money'), p.particle.mvaConfig.spectators[spectator])
    except Exception as e:
        print('FEI-printReporting Error: Could not create plots for particle', p.particle.identifier, e)
    print("FEI: printReporting - DEBUG: finished plots")
    print(bold(p.particle.identifier))
    print('Total cpu time spent reconstructing this particle: ',
          p.module_statistic.particle_time + sum(p.module_statistic.channel_time.values()))
    print('Total amount of Monte Carlo particles in training sample: ', p.mc_count)
    print('Number of decay channels: ', p.reconstructed_number_of_channels, '/', p.total_number_of_channels)
    print(bold('PreCut'))
    print('UserCut', p.particle.preCutConfig.userCut)
    print('BestCandidateCut', p.particle.preCutConfig.bestCandidateVariable,
          p.particle.preCutConfig.bestCandidateCut, p.particle.preCutConfig.bestCandidateMode)
    print('VertexCut', p.particle.preCutConfig.vertexCut)
    print('Stats before ranking')
    print(sum(p.before_ranking.values()))
    print('Stats after ranking')
    print(sum(p.after_ranking.values()))
    print('Stats after vertex')
    print(sum(p.after_vertex.values()))
    print('Stats after classifier')
    print(sum(p.after_classifier.values()))
    print(bold('PostCut'))
    print('Absolute', p.particle.postCutConfig.value)
    print('Ranking', p.particle.postCutConfig.bestCandidateCut)
    print('Stats before postcut')
    print(p.before_postcut)
    print('Stats before ranking postcut')
    print(p.before_ranking_postcut)
    print('Stats after ranking postcut')
    print(p.after_ranking_postcut)
    print(bold('Tag unique signal'))
    print('Stats before tag')
    print(p.before_tag)
    print('Stats after tag')
    print(p.after_tag)
    print(bold('Multivariate classifier'))
    print('Method', p.particle.mvaConfig.method)
    print('Target', p.particle.mvaConfig.target)
    print(bold('Individual channels'))
    for channel in p.particle.channels:
        print(bold(channel.label))
        print('Stats before ranking')
        print(p.before_ranking[channel.label])
        print('Stats after ranking')
        print(p.after_ranking[channel.label])
        print('Stats after vertex')
        print(p.after_vertex[channel.label])
        print('Stats after classifier')
        print(p.after_classifier[channel.label])


# =============================================================================
if __name__ == '__main__':
    particles, configuration = monitoring.load_config()
    cache = configuration.cache
    stages = get_stages_from_particles(particles)

    if len(sys.argv) >= 2:
        output = sys.argv[1]
        redirect = open(output, 'w')
        print('FEI: printReporting; Output redirected to', output)
    else:
        redirect = sys.stdout

    with redirect_stdout(redirect):
        for i in range(cache):
            for particle in particles:
                if particle in stages[i]:
                    print('FEI: printReporting: ', i, particle.identifier)
                    monitoringParticle = monitoring.MonitoringParticle(particle)
                    print_summary(monitoringParticle)
    if len(sys.argv) >= 2:
        redirect.close()
