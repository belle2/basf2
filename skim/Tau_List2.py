#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *


def TauList():

    # All charged tracks and gammas
    stdPi('all')
    stdPhotons('all')

    # Track and gamma cuts
    trackCuts = 'pt > 0.1'
    gammaCuts = 'E > 0.1'
    gammaCuts += ' and -0.8660 < cosTheta < 0.9563'
    applyCuts('pi+:all', trackCuts)
    applyCuts('gamma:all', gammaCuts)

    # Get EventShape variables
    buildEventShape(['pi+:all', 'gamma:all'])

    # Split in signal and tag
    cutAndCopyList('pi+:S1', 'pi+:all', 'cosToEvtThrust > 0')
    cutAndCopyList('pi+:S2', 'pi+:all', 'cosToEvtThrust < 0')
    cutAndCopyList('gamma:S1', 'gamma:all', 'cosToEvtThrust > 0')
    cutAndCopyList('gamma:S2', 'gamma:all', 'cosToEvtThrust < 0')

    variables.addAlias('nGoodTracks', 'nParticlesInList(pi+:all)')
    variables.addAlias('netCharge', 'formula(countInList(pi+:all, charge == 1) - countInList(pi+:all, charge == -1))')
    variables.addAlias('nTracksS1', 'nParticlesInList(pi+:S1)')
    variables.addAlias('nTracksS2', 'nParticlesInList(pi+:S2)')
    variables.addAlias('MinvS1', 'invMassInLists(pi+:S1, gamma:S1)')
    variables.addAlias('MinvS2', 'invMassInLists(pi+:S2, gamma:S2)')
    variables.addAlias(
        'E_S1',
        'formula(useCMSFrame(totalEnergyOfParticlesInList(pi+:S1)) + useCMSFrame(totalEnergyOfParticlesInList(gamma:S1)))')
    variables.addAlias(
        'E_S2',
        'formula(useCMSFrame(totalEnergyOfParticlesInList(pi+:S2)) + useCMSFrame(totalEnergyOfParticlesInList(gamma:S2)))')
    variables.addAlias('Etot', 'formula(visibleEnergyOfEventCMS + missingMomentumOfEventCMS)')
    variables.addAlias('E_ECL', 'formula(totalECLenergyOfParticlesInList(pi+:all)+totalECLenergyOfParticlesInList(gamma:all))')

    # Selection criteria
    applyEventCuts('1 < nGoodTracks < 7')  # cut1
    applyEventCuts('-2 < netCharge < 2')  # cut2

    applyEventCuts('missingMomentumOfEvent > 0.4 and 0.0873 < missingMomentumOfEvent_theta < 2.6180')  # cut4
    applyEventCuts('missingMass2OfEvent < 72.25 or missingMomentumOfEvent_theta > 0.2967')  # cut5
    applyEventCuts('Etot < 9.0 or 1 < E_ECL < 9')  # cut7

    applyEventCuts('[[ nTracksS1 == 1 or nTracksS1 == 3 ] and MinvS1 < 1.8 and E_S1 < 5]'
                   ' or [[ nTracksS2 == 1 or nTracksS2 == 3 ] and MinvS2 < 1.8 and E_S2 < 5]')  # cut3+cut6

    # not use 'thrustOfEvent < 0.999'  (cut8) for now

    # For skimming, the important thing is if the final particleList is empty or not.
    reconstructDecay('tau+:S1 -> pi+:S1', '')
    eventParticle = ['tau+:S1']

    return eventParticle
