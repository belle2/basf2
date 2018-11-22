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


def SetTauGenericSkimVariables():
    """
    Set particle lists and variables for TauGeneric skim

    * output particle lists: pi+:S1/S2, gamma:S1/S2

    * nGoodTracks: number of good tracks in an event
    * netCharge: total net charge of good tracks
    * nTracksS1/nTracksS2: number of good tracks in each hemisphere S1/S2 divided by thrust axis
    * MinvS1/MinvS2: invariant mass of particles in each hemisphere
    * E_S1/E_S2: total energy of particles in each hemisphere in CMS
    * Etot: visibleEnergyOfEventCMS + missingMomentumOfEventCMS (sum of energy of particles and missing momentum in CMS)
    * E_ECL: total ECL energy of particles


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
    buildEventShape(['pi+:all', 'gamma:all'], allMoments=False, foxWolfram=False, cleoCones=False, sphericity=False, jets=False)
    buildEventKinematics(['pi+:all', 'gamma:all'])

    # Split in signal and tag
    cutAndCopyList('pi+:S1', 'pi+:all', 'cosToThrustOfEvent > 0')
    cutAndCopyList('pi+:S2', 'pi+:all', 'cosToThrustOfEvent < 0')
    cutAndCopyList('gamma:S1', 'gamma:all', 'cosToThrustOfEvent > 0')
    cutAndCopyList('gamma:S2', 'gamma:all', 'cosToThrustOfEvent < 0')

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
        * Channel: :math:`e^+ e^- \\to \\tau^+ \\tau^-`
        * Skim category: physics, tau

    Criteria:
        1. :math:`1 <`  No. good tracks :math:`< 7`
        2. :math:`|` net charge :math:`| < 2`
        3. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
        4. :math:`P_{miss}>0.4` GeV, :math:`5<\\theta_{miss}<150` degree
        5. :math:`M^2_{miss}<(8.5)^2` GeV, :math:`17<\\theta_{miss}`
        6. :math:`M_{tag}<1.8` GeV, :math:`E_{tag}<5` GeV
        7. :math:`E_{tot}<9` GeV or :math:`1<E_{ECL}<9` GeV

    Returns:
        list name of the TauGeneric skim candidates
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


def TauLFVList(flag=1):
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
        if(flag):
            reconstructDecay('tau+:LFV_lgamma' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lgamma_list.append('tau+:LFV_lgamma' + str(chID))

    tau_lll_list = []
    for chID, channel in enumerate(tau_lll_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lll' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lll_list.append('tau+:LFV_lll' + str(chID))

    tau_lP0_list = []
    for chID, channel in enumerate(tau_lP0_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lP0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lP0_list.append('tau+:LFV_lP0' + str(chID))

    tau_lS0_list = []
    for chID, channel in enumerate(tau_lS0_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lS0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lS0_list.append('tau+:LFV_lS0' + str(chID))

    tau_lV0_list = []
    for chID, channel in enumerate(tau_lV0_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lV0' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lV0_list.append('tau+:LFV_lV0' + str(chID))

    tau_lhh_list = []
    for chID, channel in enumerate(tau_lhh_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_lhh' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_lhh_list.append('tau+:LFV_lhh' + str(chID))

    tau_bnv_list = []
    for chID, channel in enumerate(tau_bnv_Channels):
        if(flag):
            reconstructDecay('tau+:LFV_bnv' + str(chID) + ' -> ' + channel, tauLFVCuts, chID)
        tau_bnv_list.append('tau+:LFV_bnv' + str(chID))

    tau_lfv_lists = tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list
    return tau_lfv_lists
