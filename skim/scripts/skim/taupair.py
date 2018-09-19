#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for tau-pair analyses """

__authors__ = [
    "Kenji Inami"
]

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdV0s import stdKshorts


def SetTauGenericSkimVariables():
    """
    Set variables for TauGeneric skim
    """
    __author__ = "Kenji Inami"

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
    variables.addAlias('E_ECL', 'formula(totalECLEnergyOfParticlesInList(pi+:all)+totalECLEnergyOfParticlesInList(gamma:all))')


def TauList():
    """
    Note:
        * Skim for Tau generic decays
        * Skim LFN code: 18570600
        * Channel: :math:`e^+ e^- \\to \tau^+ \tau^-`
        * Skim category: physics, tau
    Criteria:
        1. 1 < No. good tracks < 7
        2. |net charge| < 2
        3. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
        4. Pmiss>0.4, 5<thetamiss<150
        5. M^2miss<8.5^2, 17<thetamiss
        6. Mtag<1.8, Etag<5
        7. Etot<9 or 1<E_ECL<9
    Returns:
        list name of the skim candidates
    """
    __author__ = "Kenji Inami"

    SetTauGenericSkimVariables()

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


def TauLFVList():
    """
    Note:
        * Skim for Tau LFV decays
        * Skim LFN code: 18360100
        * Channel: :math:`\tau --> l \gamma, lll, l \pi^0, l V^0, lhh, llp, phh`
        * Skim category: physics, tau
        * Criteria: `1.58 < M < 1.98, |\Delta E|<1`
    Returns:
        list name of the skim candidates
    """
    __author__ = "P. Urquijo"

    stdPi0s('loose')
    loadStdCharged()
    loadStdSkimPhoton()
    loadStdSkimPi0()
    loadStdKS()
    stdPhotons('loose')
    loadStdLightMesons()

    tauLFVCuts = '1.58 < Mbc < 1.98 and abs(deltaE) < 1.0'

    tau_lgamma_Channels = ['e+:loose gamma:loose',
                           'mu+:loose gamma:loose'
                           ]

    tau_lll_Channels = ['e-:loose e-:loose e+:loose',
                        'mu-:loose mu-:loose mu+:loose',
                        'mu-:loose e-:loose e+:loose',
                        'e-:loose mu-:loose mu+:loose',
                        'e-:loose e-:loose mu+:loose',
                        'mu-:loose mu-:loose e+:loose'
                        ]

    tau_lP0_Channels = ['e+:loose pi0:skim',
                        'mu+:loose pi0:skim',
                        'e+:loose eta:loose',
                        'mu+:loose eta:loose',
                        'e+:loose eta\':loose',
                        'mu+:loose eta\':loose',
                        'e+:loose K_S0:all',
                        'mu+:loose K_S0:all'
                        ]

    tau_lS0_Channels = ['e+:loose f_0:loose',
                        'mu+:loose f_0:loose'
                        ]

    tau_lV0_Channels = ['e+:loose rho0:loose',
                        'mu+:loose rho0:loose',
                        'e+:loose K*0:loose',
                        'mu+:loose K*0:loose',
                        'e+:loose anti-K*0:loose',
                        'mu+:loose anti-K*0:loose',
                        'e+:loose phi:loose',
                        'mu+:loose phi:loose',
                        'e+:loose omega:loose',
                        'mu+:loose omega:loose'
                        ]

    tau_lhh_Channels = ['e+:loose pi-:loose pi+:loose',
                        'mu+:loose pi-:loose pi+:loose',
                        'e-:loose pi+:loose pi+:loose',
                        'mu-:loose pi+:loose pi+:loose',
                        'e+:loose K-:loose K+:loose',
                        'mu+:loose K-:loose K+:loose',
                        'e-:loose K+:loose K+:loose',
                        'mu-:loose K+:loose K+:loose',
                        'e+:loose K-:loose pi+:loose',
                        'mu+:loose K-:loose pi+:loose',
                        'e-:loose K+:loose pi+:loose',
                        'mu-:loose K+:loose pi+:loose',
                        'e-:loose K_S0:all K_S0:all',
                        'mu-:loose K_S0:all K_S0:all'
                        ]

    tau_bnv_Channels = ['mu+:loose mu+:loose anti-p-:loose',
                        'mu-:loose mu+:loose p+:loose',
                        'anti-p-:loose pi+:loose pi+:loose',
                        'p+:loose pi-:loose pi+:loose',
                        'anti-p-:loose pi+:loose K+:loose',
                        'p+:loose pi-:loose K+:loose'
                        ]

    tau_lgamma_list = []
    for chID, channel in enumerate(tau_lgamma_Channels):
        reconstructDecay('tau+:LFV_lgamma' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lgamma_list.append('tau+:LFV_lgamma' + str(chID))

    tau_lll_list = []
    for chID, channel in enumerate(tau_lll_Channels):
        reconstructDecay('tau+:LFV_lll' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lll_list.append('tau+:LFV_lll' + str(chID))

    tau_lP0_list = []
    for chID, channel in enumerate(tau_lP0_Channels):
        reconstructDecay('tau+:LFV_lP0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lP0_list.append('tau+:LFV_lP0' + str(chID))

    tau_lS0_list = []
    for chID, channel in enumerate(tau_lS0_Channels):
        reconstructDecay('tau+:LFV_lS0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lS0_list.append('tau+:LFV_lS0' + str(chID))

    tau_lV0_list = []
    for chID, channel in enumerate(tau_lV0_Channels):
        reconstructDecay('tau+:LFV_lV0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lV0_list.append('tau+:LFV_lV0' + str(chID))

    tau_lhh_list = []
    for chID, channel in enumerate(tau_lhh_Channels):
        reconstructDecay('tau+:LFV_lhh' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lhh_list.append('tau+:LFV_lhh' + str(chID))

    tau_bnv_list = []
    for chID, channel in enumerate(tau_bnv_Channels):
        reconstructDecay('tau+:LFV_bnv' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_bnv_list.append('tau+:LFV_bnv' + str(chID))

    tau_lfv_lists = tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list
    return tau_lfv_lists
