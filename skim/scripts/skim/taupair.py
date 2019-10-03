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
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *


def SetTauGenericSkimVariables(path):
    """
    Set particle lists and variables for TauGeneric skim

    * input particle lists: pi+:all, gamma:all

    * output particle lists: pi+:tauskim, gamma:tauskim, pi+:S1/S2, gamma:S1/S2

    * nGoodTracks: number of good tracks in an event
    * netCharge: total net charge of good tracks
    * nTracksS1/nTracksS2: number of good tracks in each hemisphere S1/S2 divided by thrust axis
    * invMS1/invMS2: invariant mass of particles in each hemisphere
    * maxPt: maximum Pt amoung good tracks
    * E_ECLtrk: total ECL energy of good tracks
    """
    __author__ = "Kenji Inami"

    # Track and gamma cuts
    trackCuts = 'pt > 0.1 and abs(d0) < 1 and abs(z0) < 5'
    # trackCuts += ' and -0.8660 < cosTheta < 0.9563'
    gammaCuts = 'E > 0.15'
    gammaCuts += ' and -0.8660 < cosTheta < 0.9563'
    cutAndCopyList('pi+:tauskim', 'pi+:all', trackCuts, path=path)
    cutAndCopyList('gamma:tauskim', 'gamma:all', gammaCuts, path=path)

    # Get EventShape variables
    buildEventShape(['pi+:tauskim', 'gamma:tauskim'],
                    allMoments=False, foxWolfram=False, cleoCones=False,
                    sphericity=False, jets=False, path=path)
    buildEventKinematics(['pi+:tauskim', 'gamma:tauskim'], path=path)

    # Split in signal and tag
    cutAndCopyList('pi+:S1', 'pi+:tauskim', 'cosToThrustOfEvent > 0', path=path)
    cutAndCopyList('pi+:S2', 'pi+:tauskim', 'cosToThrustOfEvent < 0', path=path)
    cutAndCopyList('gamma:S1', 'gamma:tauskim', 'cosToThrustOfEvent > 0', path=path)
    cutAndCopyList('gamma:S2', 'gamma:tauskim', 'cosToThrustOfEvent < 0', path=path)

    variables.addAlias('nGoodTracks', 'nParticlesInList(pi+:tauskim)')
    variables.addAlias('netCharge', 'formula(countInList(pi+:tauskim, charge == 1) - countInList(pi+:tauskim, charge == -1))')
    variables.addAlias('nTracksS1', 'nParticlesInList(pi+:S1)')
    variables.addAlias('nTracksS2', 'nParticlesInList(pi+:S2)')
    variables.addAlias('invMS1', 'invMassInLists(pi+:S1, gamma:S1)')
    variables.addAlias('invMS2', 'invMassInLists(pi+:S2, gamma:S2)')
    # variables.addAlias('Evis', 'visibleEnergyOfEventCMS')
    variables.addAlias('maxPt', 'maxPtInList(pi+:tauskim)')
    variables.addAlias('E_ECLtrk', 'formula(totalECLEnergyOfParticlesInList(pi+:tauskim))')


def TauList(path):
    """
    Note:
        * Skim for Tau generic decays
        * Skim LFN code: 18570600
        * Channel: :math:`e^+ e^- \\to \\tau^+ \\tau^-`
        * Skim category: physics, tau

    Criteria:
        1. :math:`1 <`  No. good tracks :math:`< 7`
        2. :math:`|` net charge :math:`| < 2`
        3. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
        4. :math:`visibleEnergyOfEventCMS < 10` GeV and :math:`E_{ECLtrk} < 6` GeV
        5. :math:`M_{tag} < 1.8` GeV
        6. :math:`visibleEnergyOfEventCMS > 3` GeV or max `P_t > 1` GeV
        7. :math:`\\theta_{miss}<150`
        8. :math:`M_{sig} < 2.3` GeV

    Returns:
        list name of the TauGeneric skim candidates
    """
    __author__ = "Kenji Inami"

    SetTauGenericSkimVariables(path=path)

    reconstructDecay('tau+:S1 -> pi+:S1', '', path=path)
    eventParticle = ['tau+:S1']

    # Selection criteria
    applyCuts('tau+:S1', '1 < nGoodTracks < 7', path=path)  # cut1
    applyCuts('tau+:S1', '-2 < netCharge < 2', path=path)  # cut2

    # applyCuts('tau+:S1',
    #          '[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 ] or '
    #          '[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 ]', path=path)  # cut3+cut5
    applyCuts('tau+:S1',
              '[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 and invMS2 < 2.3 ] or '
              '[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 and invMS1 < 2.3 ]', path=path)  # cut3+cut5+cut8

    applyCuts('tau+:S1', 'visibleEnergyOfEventCMS < 10 and E_ECLtrk < 6', path=path)  # cut4
    applyCuts('tau+:S1', 'visibleEnergyOfEventCMS > 3 or maxPt > 1', path=path)  # cut6
    applyCuts('tau+:S1', 'missingMomentumOfEvent_theta < 2.6180', path=path)  # cut7

    # For skimming, the important thing is if the final particleList is empty or not.
    return eventParticle


def TauLFVList(flag=1, path=None):
    """
    Note:
        * Skim for Tau LFV decays
        * Skim LFN code: 18360100
        * Channel: :math:`\\tau \\to l \\gamma, lll, l \\pi^0, l V^0, lhh, llp, phh`
        * Skim category: physics, tau
        * Criteria: :math:`1.58 < M < 1.98` GeV, :math:`|\Delta E|<1` GeV

    Returns:
        list name of the TauLFV skim candidates
        * If called as TauLFVList(0), not execute skim (reconstructDecay) and
        only return list of names
        (such as tau+:LFV_lgamma0, tau+:LFV_lgamma1, tau+:LFV_lll0, ...)
    """
    __author__ = "P. Urquijo, K. Inami"

    tauLFVCuts = '1.58 < M < 1.98 and abs(deltaE) < 1.0'

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
                        'e+:loose K_S0:merged',
                        'mu+:loose K_S0:merged'
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
                        'e-:loose K_S0:merged K_S0:merged',
                        'mu-:loose K_S0:merged K_S0:merged'
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
        if(flag):
            reconstructDecay('tau+:LFV_lgamma' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_lgamma_list.append('tau+:LFV_lgamma' + str(chID))

    tau_lll_list = []
    for chID, channel in enumerate(tau_lll_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lll' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_lll_list.append('tau+:LFV_lll' + str(chID))

    tau_lP0_list = []
    for chID, channel in enumerate(tau_lP0_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lP0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_lP0_list.append('tau+:LFV_lP0' + str(chID))

    tau_lS0_list = []
    for chID, channel in enumerate(tau_lS0_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lS0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_lS0_list.append('tau+:LFV_lS0' + str(chID))

    tau_lV0_list = []
    for chID, channel in enumerate(tau_lV0_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lV0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_lV0_list.append('tau+:LFV_lV0' + str(chID))

    tau_lhh_list = []
    for chID, channel in enumerate(tau_lhh_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lhh' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_lhh_list.append('tau+:LFV_lhh' + str(chID))

    tau_bnv_list = []
    for chID, channel in enumerate(tau_bnv_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_bnv' + str(chID) + ' -> ' + channel, tauLFVCuts, chID, path=path)
        tau_bnv_list.append('tau+:LFV_bnv' + str(chID))

    tau_lfv_lists = tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list
    return tau_lfv_lists


def SetTauThrustSkimVariables(path):
    """
    Set particle lists and variables for TauThrust skim

    * input particle lists: pi+:all, gamma:all

    * output particle lists: pi+:thrust, gamma:thrust, pi+:thrustS1/thrustS2

    * nGoodTracksThrust: number of good tracks in an event
    * netChargeThrust: total net charge of good tracks
    * nTracksS1Thrust/nTracksS2Thrust: number of good tracks in each hemisphere S1/S2 divided by thrust axis

    """
    __author__ = "Ami Rostomyan, Kenji Inami"

    # Track and gamma cuts
    trackCuts = '-3.0 < dz < 7.0 and dr < 1.0'
    cutAndCopyList('pi+:thrust', 'pi+:all', trackCuts, path=path)
    gammaCuts = 'E > 0.20 and clusterNHits > 1.5 and -0.8660 < cosTheta < 0.9563'
    cutAndCopyList('gamma:thrust', 'gamma:all', gammaCuts, path=path)

    # Get EventShape variables
    buildEventShape(['pi+:thrust', 'gamma:thrust'],
                    allMoments=False, foxWolfram=False, cleoCones=False,
                    sphericity=False, jets=False, path=path)
    buildEventKinematics(['pi+:thrust', 'gamma:thrust'], path=path)

    # Split in signal and tag
    cutAndCopyList('pi+:thrustS1', 'pi+:thrust', 'cosToThrustOfEvent > 0', path=path)
    cutAndCopyList('pi+:thrustS2', 'pi+:thrust', 'cosToThrustOfEvent < 0', path=path)

    variables.addAlias('nGoodTracksThrust', 'nParticlesInList(pi+:thrust)')
    variables.addAlias('netChargeThrust', 'formula(countInList(pi+:thrust, charge == 1) - countInList(pi+:thrust, charge == -1))')
    variables.addAlias('nTracksS1Thrust', 'nParticlesInList(pi+:thrustS1)')
    variables.addAlias('nTracksS2Thrust', 'nParticlesInList(pi+:thrustS2)')


def TauThrustList(path):
    """
    Note:
        * Skim for Tau decays using thrust
        * Skim LFN code: 18570700
        * Channel: :math:`e^+ e^- \\to \\tau^+ \\tau^-`
        * Skim category: physics, tau

    Criteria:
        1. :math:`1 <`  No. good tracks :math:`< 7`
        2. net charge :math:` == 0`
        3. Event divided by thrust axis; select 1x1, 1x3, 1x5, 3x3 topology
        4. :math:`0.8 < thrust`
        5. :math:`visibleEnergyOfEventCMS < 10.4` GeV
        6. For 1x1 topology, :math:`thrust < 0.99`

    Returns:
        list name of the TauThrust skim candidates
    """
    __author__ = "Ami Rostomyan, Kenji Inami"

    SetTauThrustSkimVariables(path=path)

    reconstructDecay('tau+:thrust -> pi+:thrustS1', '', path=path)
    eventParticle = ['tau+:thrust']

    # Selection criteria
    applyCuts('tau+:thrust', '1 < nGoodTracksThrust < 7', path=path)  # cut1
    applyCuts('tau+:thrust', 'netChargeThrust == 0', path=path)  # cut2

    topologyCuts = '[nTracksS1Thrust == 1 and nTracksS2Thrust == 1]'  # 1x1
    topologyCuts += ' or [nTracksS1Thrust == 1 and nTracksS2Thrust == 3]'\
        ' or [nTracksS1Thrust == 3 and nTracksS2Thrust == 1]'  # 1x3, 3x1
    topologyCuts += ' or [nTracksS1Thrust == 1 and nTracksS2Thrust == 5]'\
        ' or [nTracksS1Thrust == 5 and nTracksS2Thrust == 1]'  # 1x5, 5x1
    topologyCuts += ' or [nTracksS1Thrust == 3 and nTracksS2Thrust == 3]'  # 3x3

    applyCuts('tau+:thrust', topologyCuts, path=path)  # cut3

    applyCuts('tau+:thrust', '0.8 < thrust', path=path)  # cut4
    applyCuts('tau+:thrust', 'visibleEnergyOfEventCMS < 10.4', path=path)  # cut5

    oneProngPath = create_path()
    applyCuts('tau+:thrust', 'thrust < 0.99', path=oneProngPath)  # cut6 thrust upper cut for 1x1
    sigThrustModule = path.add_module('VariableToReturnValue', variable='nGoodTracksThrust')
    sigThrustModule.if_value('== 2', oneProngPath, AfterConditionPath.CONTINUE)

    # For skimming, the important thing is if the final particleList is empty or not.
    return eventParticle
