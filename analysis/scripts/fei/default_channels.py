#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
 Contains some example configurations of the FEI
 Mostly you want to use get_default_channels,
 which can return the configuration for common use-cases
   - Hadronic tagging (hadronic = True)
   - Semileptonic tagging (semileptonic = True)
   - B+/B- (chargedB = True)
   - B0/anti-B0 (neutralB = True)
   - running on Belle 1 MC/data (convertedFromBelle = True)
   - running a specific FEI which is optimized for a signal selection and uses ROEs (specific = True)
   - run without semileptonic D channels (removeSLD = True )
   - B mesons with a strange quark in Y(5S) runs (strangeB = True)

 Another interesting configuration is given by get_fr_channels,
 which will return a configuration which is equivalent to the original Full Reconstruction algorithm used by Belle
"""

import b2bii
from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration
from basf2 import B2FATAL, B2INFO
from variables import variables


def get_default_channels(
        B_extra_cut=None,
        hadronic=True,
        semileptonic=True,
        KLong=False,
        baryonic=True,
        chargedB=True,
        neutralB=True,
        specific=False,
        removeSLD=False,
        usePIDNN=False,
        strangeB=False):
    """
    returns list of Particle objects with all default channels for running
    FEI on Upsilon(4S). For a training with analysis-specific signal selection,
    adding a cut on nRemainingTracksInRestOfEvent is recommended.
    @param B_extra_cut Additional user cut on recombination of tag-B-mesons
    @param hadronic whether to include hadronic B decays (default is True)
    @param semileptonic whether to include semileptonic B decays (default is True)
    @param KLong whether to include K_long decays into the training (default is False)
    @param baryonic whether to include baryons into the training (default is True)
    @param chargedB whether to recombine charged B mesons (default is True)
    @param neutralB whether to recombine neutral B mesons (default is True)
    @param specific if True, this adds isInRestOfEvent cut to all FSP
    @param removeSLD if True, removes semileptonic D modes from semileptonic B lists (default is False)
    @param usePIDNN if True, PID probabilities calculated from PID neural network are used (default is False)
    @param strangeB if True, reconstruct B_s mesons in Upsilon5S decays (default is False)
    """
    if strangeB is True:
        B2INFO('Running 5S FEI')
    if chargedB is False and neutralB is False and strangeB is False:
        B2FATAL('No B-Mesons will be recombined, since chargedB==False and neutralB==False and strangeB==False was selected!'
                ' Please reconfigure the arguments of get_default_channels() accordingly')
    if hadronic is False and semileptonic is False:
        if KLong is False:
            B2FATAL('No B-Mesons will be recombined, since hadronic==False, semileptonic==False, and KLong==False were selected.'
                    ' Please reconfigure the arguments of get_default_channels() accordingly')

    convertedFromBelle = b2bii.isB2BII()

    if convertedFromBelle:
        if usePIDNN:
            B2FATAL("The PIDNN variables do not exist for b2bii.")
        # Using Belle specific Variables for e-ID, mu-ID and K-ID
        # atcPIDBelle(3,2) is used as K-ID
        # atcPIDBelle(4,2) and atcPIDBelle(4,3) are used as pr-ID

        chargedVariables = ['eIDBelle',
                            'atcPIDBelle(3,2)',
                            'atcPIDBelle(4,2)', 'atcPIDBelle(4,3)',
                            'muIDBelle'
                            ]
    else:
        if usePIDNN:
            chargedVariables = ['electronIDNN', 'kaonIDNN', 'protonIDNN', 'muonIDNN']
        else:
            chargedVariables = ['electronID', 'kaonID', 'protonID', 'muonID']

    chargedVariables += ['p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']

    if specific:
        charged_user_cut = '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5'
    else:
        charged_user_cut = '[dr < 2] and [abs(dz) < 4]'

    pion = Particle('pi+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable="atcPIDBelle(2,3)" if convertedFromBelle
                                        else ("pionIDNN" if usePIDNN else "pionID"),
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable="atcPIDBelle(3,2)" if convertedFromBelle
                                        else ("kaonIDNN" if usePIDNN else "kaonID"),
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    proton = Particle('p+',
                      MVAConfiguration(variables=chargedVariables,
                                       target='isPrimarySignal'),
                      PreCutConfiguration(userCut=charged_user_cut,
                                          bestCandidateMode='highest',
                                          bestCandidateVariable="atcPIDBelle(4,3)" if convertedFromBelle
                                          else ("protonIDNN" if usePIDNN else "protonID"),
                                          bestCandidateCut=20),
                      PostCutConfiguration(bestCandidateCut=10, value=0.01))
    proton.addChannel(['p+:FSP'])

    electron = Particle('e+',
                        MVAConfiguration(variables=chargedVariables,
                                         target='isPrimarySignal'),
                        PreCutConfiguration(userCut=charged_user_cut,
                                            bestCandidateMode='highest',
                                            bestCandidateVariable="eIDBelle" if convertedFromBelle
                                            else ("electronIDNN" if usePIDNN else "electronID"),
                                            bestCandidateCut=10),
                        PostCutConfiguration(bestCandidateCut=5, value=0.01))
    electron.addChannel(['e+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable="muIDBelle" if convertedFromBelle
                                        else ("muonIDNN" if usePIDNN else "muonID"),
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=5, value=0.01))
    muon.addChannel(['mu+:FSP'])

    if convertedFromBelle:
        gamma_cut = 'goodBelleGamma == 1 and clusterBelleQuality == 0'
    else:
        # Same as goodBelleGamma == 1
        gamma_cut = '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.15]]'
    if specific:
        gamma_cut += ' and isInRestOfEvent > 0.5'

    gamma = Particle('gamma',
                     MVAConfiguration(variables=['clusterReg', 'clusterNHits', 'clusterTiming', 'extraInfo(preCut_rank)',
                                                 'clusterE9E25', 'pt', 'E', 'pz'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut=gamma_cut,
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40),
                     PostCutConfiguration(bestCandidateCut=20, value=0.01))
    gamma.addChannel(['gamma:FSP'])
    gamma.addChannel(['gamma:V0'],
                     MVAConfiguration(variables=['pt', 'E', 'pz', 'extraInfo(preCut_rank)'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut='',
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40))

    if convertedFromBelle:

        pi0_cut = '0.08 < InvM < 0.18'
        if specific:
            pi0_cut += ' and isInRestOfEvent > 0.5'

        pi0 = Particle('pi0',
                       MVAConfiguration(variables=['InvM', 'extraInfo(preCut_rank)', 'chiProb', 'abs(BellePi0SigM)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut=pi0_cut,
                                           bestCandidateVariable='abs(BellePi0SigM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        pi0.addChannel(['pi0:FSP'])

        ks0_cut = '0.4 < M < 0.6'
        if specific:
            ks0_cut += ' and isInRestOfEvent > 0.5'

        KS0 = Particle('K_S0',
                       MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                   'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                   'cosAngleBetweenMomentumAndVertexVector',
                                                   'extraInfo(preCut_rank)', 'extraInfo(goodKs)', 'extraInfo(ksnbVLike)',
                                                   'extraInfo(ksnbNoLam)', 'extraInfo(ksnbStandard)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut=ks0_cut,
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        KS0.addChannel(['K_S0:V0'])

        Lam_cut = '0.9 < M < 1.3'
        if specific:
            Lam_cut += ' and isInRestOfEvent > 0.5'
        L0 = Particle('Lambda0',
                      MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                  'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                  'cosAngleBetweenMomentumAndVertexVector',
                                                  'extraInfo(preCut_rank)', 'extraInfo(goodLambda)', 'extraInfo(ksnbVLike)',
                                                  'extraInfo(ksnbNoLam)'],
                                       target='isSignal'),
                      PreCutConfiguration(userCut=Lam_cut,
                                          bestCandidateVariable='abs(dM)',
                                          bestCandidateCut=20),
                      PostCutConfiguration(bestCandidateCut=10, value=0.01))
        L0.addChannel(['Lambda0:V0'])

    else:

        pi0 = Particle('pi0',
                       MVAConfiguration(variables=['M', 'daughter({},extraInfo(SignalProbability))', 'extraInfo(preCut_rank)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E', 'abs(dM)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.08 < M < 0.18',
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        pi0.addChannel(['gamma', 'gamma'])

        KS0 = Particle('K_S0',
                       MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                   'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                   'daughter({},extraInfo(SignalProbability))',
                                                   'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                   'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.4 < M < 0.6',
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        KS0.addChannel(['pi+', 'pi-'])
        KS0.addChannel(['pi0', 'pi0'])

        ks0_cut = '0.4 < M < 0.6'
        if specific:
            ks0_cut += ' and isInRestOfEvent > 0.5'

        KS0.addChannel(['K_S0:V0'],
                       MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M',
                                                   'useCMSFrame(E)', 'daughterAngle(0,1)', 'extraInfo(preCut_rank)', 'abs(dM)',
                                                   'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                   'daughter({}, dz)', 'daughter({}, dr)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut=ks0_cut,
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20))

        L0 = Particle('Lambda0',
                      MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                  'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                  'daughter({},extraInfo(SignalProbability))',
                                                  'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                  'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)'],
                                       target='isSignal'),
                      PreCutConfiguration(userCut='0.9 < M < 1.3',
                                          bestCandidateVariable='abs(dM)',
                                          bestCandidateCut=20),
                      PostCutConfiguration(bestCandidateCut=10, value=0.01))
        L0.addChannel(['p+', 'pi-'])

        Lam_cut = '0.9 < M < 1.3'
        if specific:
            Lam_cut += ' and isInRestOfEvent > 0.5'

        L0.addChannel(['Lambda0:V0'],
                      MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M',
                                                  'useCMSFrame(E)', 'daughterAngle(0,1)', 'extraInfo(preCut_rank)', 'abs(dM)',
                                                  'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                  'daughter({}, dz)', 'daughter({}, dr)'],
                                       target='isSignal'),
                      PreCutConfiguration(userCut=Lam_cut,
                                          bestCandidateVariable='abs(dM)',
                                          bestCandidateCut=20))
    kl0_cut = ''
    if specific:
        kl0_cut += 'isInRestOfEvent > 0.5'

    KL0 = Particle('K_L0',
                   MVAConfiguration(variables=['E', 'klmClusterTiming'],
                                    target='isSignal'),
                   PreCutConfiguration(userCut=kl0_cut,
                                       bestCandidateVariable='abs(dM)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.01))
    KL0.addChannel(['K_L0:FSP'])

    SigmaP = Particle('Sigma+',
                      MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                  'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                  'daughter({},extraInfo(SignalProbability))',
                                                  'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                  'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)'],
                                       target='isSignal'),
                      PreCutConfiguration(userCut='1.0 < M < 1.4',
                                          bestCandidateVariable='abs(dM)',
                                          bestCandidateCut=20),
                      PostCutConfiguration(bestCandidateCut=10, value=0.01))
    SigmaP.addChannel(['p+', 'pi0'])
    # variables for D mesons and J/Psi
    intermediate_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
                         'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)', 'abs(dM)',
                         'useRestFrame(daughter({}, p))',
                         'useRestFrame(daughter({}, distance))',
                         'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
                         'daughterInvariantMass({},{})', 'daughterInvariantMass({},{},{})', 'daughterInvariantMass({},{},{},{})',
                         'daughterInvariantMass({},{},{},{},{})', 'dQ', 'Q', 'dM', 'daughter({},extraInfo(decayModeID))']

    # TODO if specific:
    # We can not do this in the generic case (because this would heavily influence our performance on the unknown signal events
    # but in the specific case this could work well
    #    intermediate_vars = ['nRemainingTracksInEvent']
    LC = Particle('Lambda_c+',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='2.2 < M < 2.4',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    LC.addChannel(['p+', 'K-', 'pi+'])
    LC.addChannel(['p+', 'pi-', 'pi+'])
    LC.addChannel(['p+', 'K-', 'K+'])
    LC.addChannel(['p+', 'K-', 'pi+', 'pi0'])
    LC.addChannel(['p+', 'K-', 'pi+', 'pi0', 'pi0'])
    LC.addChannel(['p+', 'pi+', 'pi+', 'pi-', 'pi-'])
    LC.addChannel(['p+', 'K_S0'])
    LC.addChannel(['p+', 'K_S0', 'pi0'])
    LC.addChannel(['p+', 'K_S0', 'pi+', 'pi-'])
    LC.addChannel(['Lambda0', 'pi+'])
    LC.addChannel(['Lambda0', 'pi+', 'pi0'])
    LC.addChannel(['Lambda0', 'pi+', 'pi-', 'pi+'])
    LC.addChannel(['Lambda0', 'pi+', 'gamma'])
    LC.addChannel(['Lambda0', 'pi+', 'pi0', 'gamma'])
    LC.addChannel(['Lambda0', 'pi+', 'pi-', 'pi+', 'gamma'])
    LC.addChannel(['Sigma+', 'pi+', 'pi-'])
    LC.addChannel(['Sigma+', 'pi+', 'pi-', 'pi0'])
    LC.addChannel(['Sigma+', 'pi0'])

    D0 = Particle('D0',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    D0.addChannel(['K-', 'pi+'])
    D0.addChannel(['K-', 'pi+', 'pi0'])
    D0.addChannel(['K-', 'pi+', 'pi0', 'pi0'])
    D0.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
    D0.addChannel(['K-', 'pi+', 'pi+', 'pi-', 'pi0'])
    D0.addChannel(['pi-', 'pi+'])
    D0.addChannel(['pi-', 'pi+', 'pi+', 'pi-'])
    D0.addChannel(['pi-', 'pi+', 'pi0'])
    D0.addChannel(['pi-', 'pi+', 'pi0', 'pi0'])
    D0.addChannel(['K_S0', 'pi0'])
    D0.addChannel(['K_S0', 'pi+', 'pi-'])
    D0.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])
    D0.addChannel(['K-', 'K+'])
    D0.addChannel(['K-', 'K+', 'pi0'])
    D0.addChannel(['K-', 'K+', 'K_S0'])

    if not removeSLD and semileptonic:
        D0_SL = Particle('D0:semileptonic',
                         MVAConfiguration(variables=intermediate_vars,
                                          target='isSignalAcceptMissingNeutrino'),
                         PreCutConfiguration(userCut='',
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=10, value=0.001))

        D0_SL.addChannel(['K-', 'e+'])
        D0_SL.addChannel(['K-', 'mu+'])
        D0_SL.addChannel(['K-', 'pi0', 'e+'])
        D0_SL.addChannel(['K-', 'pi0', 'mu+'])
        D0_SL.addChannel(['K_S0', 'pi-', 'e+'])
        D0_SL.addChannel(['K_S0', 'pi-', 'mu+'])

    if KLong:
        D0_KL = Particle('D0:KL',
                         MVAConfiguration(variables=intermediate_vars,
                                          target='isSignal'),
                         PreCutConfiguration(userCut='',
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=10, value=0.001))

        D0_KL.addChannel(['K_L0', 'pi0'])
        D0_KL.addChannel(['K_L0', 'pi+', 'pi-'])
        D0_KL.addChannel(['K_L0', 'pi+', 'pi-', 'pi0'])
        D0_KL.addChannel(['K-', 'K+', 'K_L0'])

    DP = Particle('D+',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DP.addChannel(['K-', 'pi+', 'pi+'])
    DP.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
    DP.addChannel(['K-', 'K+', 'pi+'])
    DP.addChannel(['K-', 'K+', 'pi+', 'pi0'])
    DP.addChannel(['pi+', 'pi0'])
    DP.addChannel(['pi+', 'pi+', 'pi-'])
    DP.addChannel(['pi+', 'pi+', 'pi-', 'pi0'])
    DP.addChannel(['K_S0', 'pi+'])
    DP.addChannel(['K_S0', 'pi+', 'pi0'])
    DP.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])
    DP.addChannel(['K+', 'K_S0', 'K_S0'])

    if not removeSLD and semileptonic:
        DP_SL = Particle('D+:semileptonic',
                         MVAConfiguration(variables=intermediate_vars,
                                          target='isSignalAcceptMissingNeutrino'),
                         PreCutConfiguration(userCut='',
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DP_SL.addChannel(['K_S0', 'e+'])
        DP_SL.addChannel(['K_S0', 'mu+'])
        DP_SL.addChannel(['K-', 'pi+', 'e+'])
        DP_SL.addChannel(['K-', 'pi+', 'mu+'])

    if KLong:
        DP_KL = Particle('D+:KL',
                         MVAConfiguration(variables=intermediate_vars,
                                          target='isSignal'),
                         PreCutConfiguration(userCut='',
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DP_KL.addChannel(['K_L0', 'pi+'])
        DP_KL.addChannel(['K_L0', 'pi+', 'pi0'])
        DP_KL.addChannel(['K_L0', 'pi+', 'pi+', 'pi-'])
        DP_KL.addChannel(['K+', 'K_L0', 'K_S0'])
        DP_KL.addChannel(['K+', 'K_L0', 'K_L0'])

    Jpsi = Particle('J/psi',
                    MVAConfiguration(variables=intermediate_vars,
                                     target='isSignal'),
                    PreCutConfiguration(userCut='2.6 < M < 3.7',
                                        bestCandidateVariable='abs(dM)',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.001))

    Jpsi.addChannel(['e+', 'e-'])
    Jpsi.addChannel(['mu+', 'mu-'])

    DSP = Particle('D*+',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DSP.addChannel(['D0', 'pi+'])
    DSP.addChannel(['D+', 'pi0'])
    DSP.addChannel(['D+', 'gamma'])

    if not removeSLD and semileptonic:
        DSP_SL = Particle('D*+:semileptonic',
                          MVAConfiguration(variables=intermediate_vars,
                                           target='isSignalAcceptMissingNeutrino'),
                          PreCutConfiguration(userCut='0 < Q < 0.3',
                                              bestCandidateVariable='abs(dQ)',
                                              bestCandidateCut=20),
                          PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DSP_SL.addChannel(['D0:semileptonic', 'pi+'])
        DSP_SL.addChannel(['D+:semileptonic', 'pi0'])
        DSP_SL.addChannel(['D+:semileptonic', 'gamma'])

    if KLong:
        DSP_KL = Particle('D*+:KL',
                          MVAConfiguration(variables=intermediate_vars,
                                           target='isSignal'),
                          PreCutConfiguration(userCut='0 < Q < 0.3',
                                              bestCandidateVariable='abs(dQ)',
                                              bestCandidateCut=20),
                          PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DSP_KL.addChannel(['D0:KL', 'pi+'])
        DSP_KL.addChannel(['D+:KL', 'pi0'])
        DSP_KL.addChannel(['D+:KL', 'gamma'])

    DS0 = Particle('D*0',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DS0.addChannel(['D0', 'pi0'])
    DS0.addChannel(['D0', 'gamma'])

    if not removeSLD and semileptonic:
        DS0_SL = Particle('D*0:semileptonic',
                          MVAConfiguration(variables=intermediate_vars,
                                           target='isSignalAcceptMissingNeutrino'),
                          PreCutConfiguration(userCut='0 < Q < 0.3',
                                              bestCandidateVariable='abs(dQ)',
                                              bestCandidateCut=20),
                          PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DS0_SL.addChannel(['D0:semileptonic', 'pi0'])
        DS0_SL.addChannel(['D0:semileptonic', 'gamma'])

    if KLong:
        DS0_KL = Particle('D*0:KL',
                          MVAConfiguration(variables=intermediate_vars,
                                           target='isSignal'),
                          PreCutConfiguration(userCut='0 < Q < 0.3',
                                              bestCandidateVariable='abs(dQ)',
                                              bestCandidateCut=20),
                          PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DS0_KL.addChannel(['D0:KL', 'pi0'])
        DS0_KL.addChannel(['D0:KL', 'gamma'])

    DS = Particle('D_s+',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.68 < M < 2.1',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DS.addChannel(['K+', 'K_S0'])
    DS.addChannel(['K+', 'pi+', 'pi-'])
    DS.addChannel(['K+', 'K-', 'pi+'])
    DS.addChannel(['K+', 'K-', 'pi+', 'pi0'])
    DS.addChannel(['K+', 'K_S0', 'pi+', 'pi-'])
    DS.addChannel(['K-', 'K_S0', 'pi+', 'pi+'])
    DS.addChannel(['K+', 'K-', 'pi+', 'pi+', 'pi-'])
    DS.addChannel(['pi+', 'pi+', 'pi-'])
    DS.addChannel(['K_S0', 'pi+'])
    DS.addChannel(['K_S0', 'pi+', 'pi0'])

    if KLong:
        DS_KL = Particle('D_s+:KL',
                         MVAConfiguration(variables=intermediate_vars,
                                          target='isSignal'),
                         PreCutConfiguration(userCut='',
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DS_KL.addChannel(['K+', 'K_L0'])
        DS_KL.addChannel(['K+', 'K_L0', 'pi+', 'pi-'])
        DS_KL.addChannel(['K-', 'K_L0', 'pi+', 'pi+'])
        DS_KL.addChannel(['K_L0', 'pi+'])
        DS_KL.addChannel(['K_L0', 'pi+', 'pi0'])

    DSS = Particle('D_s*+',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0.0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DSS.addChannel(['D_s+', 'gamma'])
    DSS.addChannel(['D_s+', 'pi0'])

    if KLong:
        DSS_KL = Particle('D_s*+:KL',
                          MVAConfiguration(variables=intermediate_vars,
                                           target='isSignal'),
                          PreCutConfiguration(userCut='0.0 < Q < 0.3',
                                              bestCandidateVariable='abs(dQ)',
                                              bestCandidateCut=20),
                          PostCutConfiguration(bestCandidateCut=10, value=0.001))

        DSS_KL.addChannel(['D_s+:KL', 'gamma'])
        DSS_KL.addChannel(['D_s+:KL', 'pi0'])

    # note: these should not be correlated to Mbc (weak correlation of deltaE is OK)
    B_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
              'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)',
              'useRestFrame(daughter({}, p))',
              'useRestFrame(daughter({}, distance))',
              'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
              'dr', 'dz', 'dx', 'dy', 'distance', 'significanceOfDistance', 'deltaE', 'daughter({},extraInfo(decayModeID))']

    hadronic_user_cut = 'Mbc > 5.2 and abs(deltaE) < 0.5'
    if B_extra_cut is not None:
        hadronic_user_cut += ' and [' + B_extra_cut + ']'

    BP = Particle('B+',
                  MVAConfiguration(variables=B_vars,
                                   spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronic_user_cut,
                                      bestCandidateMode='highest',
                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=20))

    BP.addChannel(['anti-D0', 'pi+'])
    BP.addChannel(['anti-D0', 'pi+', 'pi0'])
    BP.addChannel(['anti-D0', 'pi+', 'pi0', 'pi0'])
    BP.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-'])
    BP.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-', 'pi0'])
    BP.addChannel(['anti-D0', 'D+'])
    BP.addChannel(['anti-D0', 'D+', 'K_S0'])
    BP.addChannel(['anti-D*0', 'D+', 'K_S0'])
    BP.addChannel(['anti-D0', 'D*+', 'K_S0'])
    BP.addChannel(['anti-D*0', 'D*+', 'K_S0'])
    BP.addChannel(['anti-D0', 'D0', 'K+'])
    BP.addChannel(['anti-D*0', 'D0', 'K+'])
    BP.addChannel(['anti-D0', 'D*0', 'K+'])
    BP.addChannel(['anti-D*0', 'D*0', 'K+'])
    BP.addChannel(['D_s+', 'anti-D0'])
    BP.addChannel(['anti-D*0', 'pi+'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi0'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi0', 'pi0'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-', 'pi0'])
    BP.addChannel(['D_s*+', 'anti-D0'])
    BP.addChannel(['D_s+', 'anti-D*0'])
    BP.addChannel(['anti-D0', 'K+'])
    BP.addChannel(['D-', 'pi+', 'pi+'])
    BP.addChannel(['D-', 'pi+', 'pi+', 'pi0'])
    BP.addChannel(['J/psi', 'K+'], preCutConfig=BP.preCutConfig._replace(noBackgroundSampling=True))
    BP.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
    BP.addChannel(['J/psi', 'K+', 'pi0'])
    BP.addChannel(['J/psi', 'K_S0', 'pi+'])
    if baryonic:
        BP.addChannel(['anti-Lambda_c-', 'p+', 'pi+', 'pi0'])
        BP.addChannel(['anti-Lambda_c-', 'p+', 'pi+', 'pi-', 'pi+'])
        BP.addChannel(['anti-D0', 'p+', 'anti-p-', 'pi+'])
        BP.addChannel(['anti-D*0', 'p+', 'anti-p-', 'pi+'])
        BP.addChannel(['D+', 'p+', 'anti-p-', 'pi+', 'pi-'])
        BP.addChannel(['D*+', 'p+', 'anti-p-', 'pi+', 'pi-'])
        BP.addChannel(['anti-Lambda_c-', 'p+', 'pi+'])

    B_SL_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
                 'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)',
                 'useRestFrame(daughter({}, p))',
                 'useRestFrame(daughter({}, distance))',
                 'cosAngleBetweenMomentumAndVertexVector',
                 'dr', 'dz', 'dx', 'dy', 'distance', 'significanceOfDistance', 'daughter({},extraInfo(decayModeID))']

    semileptonic_user_cut = ''
    if B_extra_cut is not None:
        semileptonic_user_cut += B_extra_cut

    BP_SL = Particle('B+:semileptonic',
                     MVAConfiguration(variables=B_SL_vars,
                                      spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                      target='isSignalAcceptMissingNeutrino'),
                     PreCutConfiguration(userCut=semileptonic_user_cut,
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                         bestCandidateCut=20),
                     PostCutConfiguration(bestCandidateCut=20))
    BP_SL.addChannel(['anti-D0', 'e+'])
    BP_SL.addChannel(['anti-D0', 'mu+'])
    BP_SL.addChannel(['anti-D*0', 'e+'])
    BP_SL.addChannel(['anti-D*0', 'mu+'])
    BP_SL.addChannel(['D-', 'pi+', 'e+'])
    BP_SL.addChannel(['D-', 'pi+', 'mu+'])
    BP_SL.addChannel(['D*-', 'pi+', 'e+'])
    BP_SL.addChannel(['D*-', 'pi+', 'mu+'])

    if not removeSLD and semileptonic:
        BP_SL.addChannel(['anti-D0:semileptonic', 'pi+'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi0'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi0', 'pi0'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi+', 'pi-'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'D+'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'D+', 'K_S0'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'D+', 'K_S0'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'D*+', 'K_S0'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'D*+', 'K_S0'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'D0', 'K+'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'D0', 'K+'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'D*0', 'K+'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'D*0', 'K+'])
        BP_SL.addChannel(['anti-D0', 'D+:semileptonic'])
        BP_SL.addChannel(['anti-D0', 'D+:semileptonic', 'K_S0'])
        BP_SL.addChannel(['anti-D*0', 'D+:semileptonic', 'K_S0'])
        BP_SL.addChannel(['anti-D0', 'D*+:semileptonic', 'K_S0'])
        BP_SL.addChannel(['anti-D*0', 'D*+:semileptonic', 'K_S0'])
        BP_SL.addChannel(['anti-D0', 'D0:semileptonic', 'K+'])
        BP_SL.addChannel(['anti-D*0', 'D0:semileptonic', 'K+'])
        BP_SL.addChannel(['anti-D0', 'D*0:semileptonic', 'K+'])
        BP_SL.addChannel(['anti-D*0', 'D*0:semileptonic', 'K+'])
        BP_SL.addChannel(['D_s+', 'anti-D0:semileptonic'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi0'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi0', 'pi0'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi+', 'pi-'])
        BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
        BP_SL.addChannel(['D_s*+', 'anti-D0:semileptonic'])
        BP_SL.addChannel(['D_s+', 'anti-D*0:semileptonic'])
        BP_SL.addChannel(['anti-D0:semileptonic', 'K+'])
        BP_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+'])
        BP_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi0'])

    if KLong:
        BP_KL = Particle('B+:KL',
                         MVAConfiguration(variables=B_vars,
                                          spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                          target='isSignal'),
                         PreCutConfiguration(userCut=semileptonic_user_cut,
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=20))
        BP_KL.addChannel(['anti-D0:KL', 'pi+'])
        BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi0'])
        BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi0', 'pi0'])
        BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi+', 'pi-'])
        BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
        BP_KL.addChannel(['anti-D0:KL', 'D+'])
        BP_KL.addChannel(['anti-D0:KL', 'D+', 'K_S0'])
        BP_KL.addChannel(['anti-D*0:KL', 'D+', 'K_S0'])
        BP_KL.addChannel(['anti-D0:KL', 'D*+', 'K_S0'])
        BP_KL.addChannel(['anti-D*0:KL', 'D*+', 'K_S0'])
        BP_KL.addChannel(['anti-D0:KL', 'D0', 'K+'])
        BP_KL.addChannel(['anti-D*0:KL', 'D0', 'K+'])
        BP_KL.addChannel(['anti-D0:KL', 'D*0', 'K+'])
        BP_KL.addChannel(['anti-D*0:KL', 'D*0', 'K+'])
        BP_KL.addChannel(['anti-D0', 'D+:KL'])
        BP_KL.addChannel(['anti-D0', 'D+:KL', 'K_S0'])
        BP_KL.addChannel(['anti-D*0', 'D+:KL', 'K_S0'])
        BP_KL.addChannel(['anti-D0', 'D*+:KL', 'K_S0'])
        BP_KL.addChannel(['anti-D*0', 'D*+:KL', 'K_S0'])
        BP_KL.addChannel(['anti-D0', 'D0:KL', 'K+'])
        BP_KL.addChannel(['anti-D*0', 'D0:KL', 'K+'])
        BP_KL.addChannel(['anti-D0', 'D*0:KL', 'K+'])
        BP_KL.addChannel(['anti-D*0', 'D*0:KL', 'K+'])
        BP_KL.addChannel(['D_s+', 'anti-D0:KL'])
        BP_KL.addChannel(['D_s+:KL', 'anti-D0'])
        BP_KL.addChannel(['anti-D*0:KL', 'pi+'])
        BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi0'])
        BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi0', 'pi0'])
        BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi+', 'pi-'])
        BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
        BP_KL.addChannel(['D_s*+', 'anti-D0:KL'])
        BP_KL.addChannel(['D_s+', 'anti-D*0:KL'])
        BP_KL.addChannel(['D_s*+:KL', 'anti-D0'])
        BP_KL.addChannel(['D_s+:KL', 'anti-D*0'])
        BP_KL.addChannel(['anti-D0:KL', 'K+'])
        BP_KL.addChannel(['D-:KL', 'pi+', 'pi+'])
        BP_KL.addChannel(['D-:KL', 'pi+', 'pi+', 'pi0'])
        BP_KL.addChannel(['anti-D0', 'D+', 'K_L0'])
        BP_KL.addChannel(['anti-D*0', 'D+', 'K_L0'])
        BP_KL.addChannel(['anti-D0', 'D*+', 'K_L0'])
        BP_KL.addChannel(['anti-D*0', 'D*+', 'K_L0'])
        BP_KL.addChannel(['J/psi', 'K_L0', 'pi+'])

    B0 = Particle('B0',
                  MVAConfiguration(variables=B_vars,
                                   spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronic_user_cut,
                                      bestCandidateMode='highest',
                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=20))
    B0.addChannel(['D-', 'pi+'])
    B0.addChannel(['D-', 'pi+', 'pi0'])
    B0.addChannel(['D-', 'pi+', 'pi0', 'pi0'])
    B0.addChannel(['D-', 'pi+', 'pi+', 'pi-'])
    B0.addChannel(['D-', 'pi+', 'pi+', 'pi-', 'pi0'])
    B0.addChannel(['anti-D0', 'pi+', 'pi-'])
    B0.addChannel(['D-', 'D0', 'K+'])
    B0.addChannel(['D-', 'D*0', 'K+'])
    B0.addChannel(['D*-', 'D0', 'K+'])
    B0.addChannel(['D*-', 'D*0', 'K+'])
    B0.addChannel(['D-', 'D+', 'K_S0'])
    B0.addChannel(['D*-', 'D+', 'K_S0'])
    B0.addChannel(['D-', 'D*+', 'K_S0'])
    B0.addChannel(['D*-', 'D*+', 'K_S0'])
    B0.addChannel(['D_s+', 'D-'])
    B0.addChannel(['D*-', 'pi+'])
    B0.addChannel(['D*-', 'pi+', 'pi0'])
    B0.addChannel(['D*-', 'pi+', 'pi0', 'pi0'])
    B0.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
    B0.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
    B0.addChannel(['D_s*+', 'D-'])
    B0.addChannel(['D_s+', 'D*-'])
    B0.addChannel(['D_s*+', 'D*-'])
    B0.addChannel(['J/psi', 'K_S0'], preCutConfig=B0.preCutConfig._replace(noBackgroundSampling=True))
    B0.addChannel(['J/psi', 'K+', 'pi-'])
    B0.addChannel(['J/psi', 'K_S0', 'pi+', 'pi-'])
    if baryonic:
        B0.addChannel(['anti-Lambda_c-', 'p+', 'pi+', 'pi-'])
        B0.addChannel(['anti-D0', 'p+', 'anti-p-'])
        B0.addChannel(['D-', 'p+', 'anti-p-', 'pi+'])
        B0.addChannel(['D*-', 'p+', 'anti-p-', 'pi+'])
        B0.addChannel(['anti-D0', 'p+', 'anti-p-', 'pi+', 'pi-'])
        B0.addChannel(['anti-D*0', 'p+', 'anti-p-', 'pi+', 'pi-'])

    B0_SL = Particle('B0:semileptonic',
                     MVAConfiguration(variables=B_SL_vars,
                                      spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                      target='isSignalAcceptMissingNeutrino'),
                     PreCutConfiguration(userCut=semileptonic_user_cut,
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                         bestCandidateCut=20),
                     PostCutConfiguration(bestCandidateCut=20))
    B0_SL.addChannel(['D-', 'e+'])
    B0_SL.addChannel(['D-', 'mu+'])
    B0_SL.addChannel(['D*-', 'e+'])
    B0_SL.addChannel(['D*-', 'mu+'])
    B0_SL.addChannel(['anti-D0', 'pi-', 'e+'])
    B0_SL.addChannel(['anti-D0', 'pi-', 'mu+'])
    B0_SL.addChannel(['anti-D*0', 'pi-', 'e+'])
    B0_SL.addChannel(['anti-D*0', 'pi-', 'mu+'])

    if not removeSLD and semileptonic:
        B0_SL.addChannel(['D-:semileptonic', 'pi+'])
        B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi0'])
        B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi0', 'pi0'])
        B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi-'])
        B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
        B0_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi-'])
        B0_SL.addChannel(['D-:semileptonic', 'D0', 'K+'])
        B0_SL.addChannel(['D-:semileptonic', 'D*0', 'K+'])
        B0_SL.addChannel(['D*-:semileptonic', 'D0', 'K+'])
        B0_SL.addChannel(['D*-:semileptonic', 'D*0', 'K+'])
        B0_SL.addChannel(['D-:semileptonic', 'D+', 'K_S0'])
        B0_SL.addChannel(['D*-:semileptonic', 'D+', 'K_S0'])
        B0_SL.addChannel(['D-:semileptonic', 'D*+', 'K_S0'])
        B0_SL.addChannel(['D*-:semileptonic', 'D*+', 'K_S0'])
        B0_SL.addChannel(['D-', 'D0:semileptonic', 'K+'])
        B0_SL.addChannel(['D-', 'D*0:semileptonic', 'K+'])
        B0_SL.addChannel(['D*-', 'D0:semileptonic', 'K+'])
        B0_SL.addChannel(['D*-', 'D*0:semileptonic', 'K+'])
        B0_SL.addChannel(['D-', 'D+:semileptonic', 'K_S0'])
        B0_SL.addChannel(['D*-', 'D+:semileptonic', 'K_S0'])
        B0_SL.addChannel(['D-', 'D*+:semileptonic', 'K_S0'])
        B0_SL.addChannel(['D*-', 'D*+:semileptonic', 'K_S0'])
        B0_SL.addChannel(['D_s+', 'D-:semileptonic'])
        B0_SL.addChannel(['D*-:semileptonic', 'pi+'])
        B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi0'])
        B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi0', 'pi0'])
        B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi+', 'pi-'])
        B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
        B0_SL.addChannel(['D_s*+', 'D-:semileptonic'])
        B0_SL.addChannel(['D_s+', 'D*-:semileptonic'])
        B0_SL.addChannel(['D_s*+', 'D*-:semileptonic'])

    if KLong:
        B0_KL = Particle('B0:KL',
                         MVAConfiguration(variables=B_vars,
                                          spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                          target='isSignal'),
                         PreCutConfiguration(userCut=semileptonic_user_cut,
                                             bestCandidateMode='highest',
                                             bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                             bestCandidateCut=20),
                         PostCutConfiguration(bestCandidateCut=20))
        B0_KL.addChannel(['D-:KL', 'pi+'])
        B0_KL.addChannel(['D-:KL', 'pi+', 'pi0'])
        B0_KL.addChannel(['D-:KL', 'pi+', 'pi0', 'pi0'])
        B0_KL.addChannel(['D-:KL', 'pi+', 'pi+', 'pi-'])
        B0_KL.addChannel(['D-:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
        B0_KL.addChannel(['anti-D0:KL', 'pi+', 'pi-'])
        B0_KL.addChannel(['D-:KL', 'D0', 'K+'])
        B0_KL.addChannel(['D-:KL', 'D*0', 'K+'])
        B0_KL.addChannel(['D*-:KL', 'D0', 'K+'])
        B0_KL.addChannel(['D*-:KL', 'D*0', 'K+'])
        B0_KL.addChannel(['D-:KL', 'D+', 'K_S0'])
        B0_KL.addChannel(['D*-:KL', 'D+', 'K_S0'])
        B0_KL.addChannel(['D-:KL', 'D*+', 'K_S0'])
        B0_KL.addChannel(['D*-:KL', 'D*+', 'K_S0'])
        B0_KL.addChannel(['D-', 'D0:KL', 'K+'])
        B0_KL.addChannel(['D-', 'D*0:KL', 'K+'])
        B0_KL.addChannel(['D*-', 'D0:KL', 'K+'])
        B0_KL.addChannel(['D*-', 'D*0:KL', 'K+'])
        B0_KL.addChannel(['D-', 'D+:KL', 'K_S0'])
        B0_KL.addChannel(['D*-', 'D+:KL', 'K_S0'])
        B0_KL.addChannel(['D-', 'D*+:KL', 'K_S0'])
        B0_KL.addChannel(['D*-', 'D*+:KL', 'K_S0'])
        B0_KL.addChannel(['D_s+', 'D-:KL'])
        B0_KL.addChannel(['D_s+:KL', 'D-'])
        B0_KL.addChannel(['D*-:KL', 'pi+'])
        B0_KL.addChannel(['D*-:KL', 'pi+', 'pi0'])
        B0_KL.addChannel(['D*-:KL', 'pi+', 'pi0', 'pi0'])
        B0_KL.addChannel(['D*-:KL', 'pi+', 'pi+', 'pi-'])
        B0_KL.addChannel(['D*-:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
        B0_KL.addChannel(['D_s*+', 'D-:KL'])
        B0_KL.addChannel(['D_s+', 'D*-:KL'])
        B0_KL.addChannel(['D_s*+', 'D*-:KL'])
        B0_KL.addChannel(['D_s*+:KL', 'D-'])
        B0_KL.addChannel(['D_s+:KL', 'D*-'])
        B0_KL.addChannel(['D_s*+:KL', 'D*-'])
        B0_KL.addChannel(['D-', 'D+', 'K_L0'])
        B0_KL.addChannel(['D*-', 'D+', 'K_L0'])
        B0_KL.addChannel(['D-', 'D*+', 'K_L0'])
        B0_KL.addChannel(['D*-', 'D*+', 'K_L0'])
        B0_KL.addChannel(['J/psi', 'K_L0'])
        B0_KL.addChannel(['J/psi', 'K_L0', 'pi+', 'pi-'])

    # Use deltaE + Mbc - m_(B_s) instead of deltaE since Bs has only one peak here (vs. 3 in deltaE)
    Bs_vars = ['formula(deltaE+Mbc-5.3669)' if x == 'deltaE' else x for x in B_vars]

    hadronic_bs_user_cut = 'Mbc > 5.3 and abs(formula(deltaE+Mbc-5.3669)) < 0.5'
    if B_extra_cut is not None:
        hadronic_bs_user_cut += ' and [' + B_extra_cut + ']'

    BS = Particle('B_s0',
                  MVAConfiguration(variables=Bs_vars,
                                   spectators={'Mbc': (5.23, None)},
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronic_bs_user_cut,
                                      bestCandidateMode='highest',
                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=20))

    # Override precut for some channels as this provides better performance
    tight_precut = BS.preCutConfig._replace(userCut=hadronic_bs_user_cut + ' and abs(formula(deltaE+Mbc-5.3669)) < 0.1')

    # D_s & D*
    BS.addChannel(['D_s-', 'D_s+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s*+', 'D_s-'], preCutConfig=tight_precut)
    BS.addChannel(['D_s*-', 'D_s+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s*+', 'D_s*-'], preCutConfig=tight_precut)
    BS.addChannel(['D_s-', 'D+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s+', 'D*-'])
    BS.addChannel(['D_s*+', 'D-'])
    BS.addChannel(['D_s*+', 'D*-'])

    # D_s
    BS.addChannel(['D_s-', 'K+'])
    BS.addChannel(['D_s+', 'K-'])
    BS.addChannel(['D_s-', 'pi+'])
    BS.addChannel(['D_s-', 'pi+', 'pi+', 'pi-'], preCutConfig=tight_precut)
    BS.addChannel(['D_s-', 'D0', 'K+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s-', 'D+', 'K_S0'])
    BS.addChannel(['D_s-', 'pi+', 'pi0'], preCutConfig=tight_precut)           # rho+
    BS.addChannel(['D_s-', 'D0', 'K+', 'pi0'], preCutConfig=tight_precut)      # K*+
    BS.addChannel(['D_s-', 'D0', 'K_S0', 'pi+'])    # K*+
    BS.addChannel(['D_s-', 'D+', 'K+', 'pi-'])      # K*0
    BS.addChannel(['D_s-', 'D+', 'K_S0', 'pi0'])    # K*0
    BS.addChannel(['D_s-', 'D*0', 'K+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s-', 'D*+', 'K_S0'])
    BS.addChannel(['D_s-', 'D*0', 'K+', 'pi0'])     # K*+
    BS.addChannel(['D_s-', 'D*0', 'K_S0', 'pi+'])   # K*+
    BS.addChannel(['D_s-', 'D*+', 'K+', 'pi-'])     # K*0
    BS.addChannel(['D_s-', 'D*+', 'K_S0', 'pi0'])   # K*0

    # D_s*
    BS.addChannel(['D_s*-', 'K+'])
    BS.addChannel(['D_s*-', 'pi+'])
    BS.addChannel(['D_s*-', 'D0', 'K+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s*-', 'D+', 'K_S0'])
    BS.addChannel(['D_s*-', 'D*0', 'K+'], preCutConfig=tight_precut)
    BS.addChannel(['D_s*-', 'D*+', 'K_S0'])
    BS.addChannel(['D_s*-', 'pi+', 'pi+', 'pi-'], preCutConfig=tight_precut)
    BS.addChannel(['D_s*-', 'pi+', 'pi0'], preCutConfig=tight_precut)          # rho+
    BS.addChannel(['D_s*-', 'D0', 'K+', 'pi0'])     # K*+
    BS.addChannel(['D_s*-', 'D0', 'K_S0', 'pi+'])   # K*+
    BS.addChannel(['D_s*-', 'D+', 'K+', 'pi-'])     # K*0
    BS.addChannel(['D_s*-', 'D+', 'K_S0', 'pi0'])   # K*0
    BS.addChannel(['D_s*-', 'D*0', 'K+', 'pi0'])    # K*+
    BS.addChannel(['D_s*-', 'D*0', 'K_S0', 'pi+'])  # K*+
    BS.addChannel(['D_s*-', 'D*+', 'K+', 'pi-'])    # K*0
    BS.addChannel(['D_s*-', 'D*+', 'K_S0', 'pi0'])  # K*0

    # J/Psi
    BS.addChannel(['J/psi', 'K_S0'])
    BS.addChannel(['J/psi', 'pi+', 'pi-'])
    BS.addChannel(['J/psi', 'K+', 'K-'], preCutConfig=BS.preCutConfig._replace(noBackgroundSampling=True))  # Phi
    BS.addChannel(['J/psi', 'K_S0', 'K-', 'pi+'])
    BS.addChannel(['J/psi', 'K-', 'K+', 'pi0'])
    BS.addChannel(['J/psi', 'pi-', 'pi+', 'pi0'])  # Eta
    BS.addChannel(['J/psi', 'pi+', 'pi-', 'pi-', 'pi+', 'pi0'])  # Etaprime

    # Other
    BS.addChannel(['anti-D*0', 'K_S0'])
    BS.addChannel(['anti-D0', 'K_S0'])
    BS.addChannel(['anti-D0', 'K-', 'pi+'])

    particles = []
    particles.append(pion)
    particles.append(kaon)
    if baryonic:
        particles.append(proton)
    particles.append(muon)
    particles.append(electron)
    particles.append(gamma)

    particles.append(pi0)
    particles.append(KS0)
    if baryonic:
        particles.append(L0)
        particles.append(SigmaP)
    particles.append(Jpsi)

    particles.append(D0)
    particles.append(DP)
    particles.append(DS)
    if baryonic:
        particles.append(LC)

    particles.append(DS0)
    particles.append(DSP)
    particles.append(DSS)

    if hadronic:
        if neutralB:
            particles.append(B0)
        if chargedB:
            particles.append(BP)

    if KLong:
        particles.append(KL0)
        particles.append(D0_KL)
        particles.append(DP_KL)
        particles.append(DS_KL)
        particles.append(DS0_KL)
        particles.append(DSP_KL)
        particles.append(DSS_KL)
        if neutralB:
            particles.append(B0_KL)
        if chargedB:
            particles.append(BP_KL)

    if semileptonic:
        if not removeSLD:
            particles.append(D0_SL)
            particles.append(DP_SL)
            particles.append(DS0_SL)
            particles.append(DSP_SL)
        if neutralB:
            particles.append(B0_SL)
        if chargedB:
            particles.append(BP_SL)

    if strangeB:
        particles.append(BS)

    return particles


def get_ccbarLambdaC_channels(
        specific=False,
        addPi0=False,
        addCharged=False,
        addStrangness=False,
        usePIDNN=False):
    """
    returns list of Particle objects with all default channels for running
    FEI on ccbar to tag Lambda_c+ decays
    These channel list has not been optimized yet and currenlty serves
    only as an example for FEI application on ccbar events.
    @param specific if True, this adds isInRestOfEvent cut to all FSP
    @param addPi0 if True, this adds pi0 to all channels
    @param addCharged if True, this adds charged pairs to all channels
    @param addStrangness if True, this adds strange particles to all channels
    @param usePIDNN if True, PID probabilities calculated from PID neural network are used (default is False)
    """

    convertedFromBelle = b2bii.isB2BII()

    if convertedFromBelle:
        if usePIDNN:
            B2FATAL("The PIDNN variables do not exist for b2bii.")
        # Using Belle specific Variables for e-ID, mu-ID and K-ID
        # atcPIDBelle(3,2) is used as K-ID
        # atcPIDBelle(4,2) and atcPIDBelle(4,3) are used as pr-ID

        chargedVariables = ['eIDBelle',
                            'atcPIDBelle(3,2)',
                            'atcPIDBelle(4,2)', 'atcPIDBelle(4,3)',
                            'muIDBelle'
                            ]
    else:
        if usePIDNN:
            chargedVariables = ['electronIDNN', 'kaonIDNN', 'protonIDNN', 'muonIDNN']
        else:
            chargedVariables = ['electronID', 'kaonID', 'protonID', 'muonID']

    chargedVariables += ['p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']

    if specific:
        charged_user_cut = '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5'
    else:
        charged_user_cut = '[dr < 2] and [abs(dz) < 4]'

    # region 1st stage
    pion = Particle('pi+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable="atcPIDBelle(2,3)" if convertedFromBelle
                                        else ("pionIDNN" if usePIDNN else "pionID"),
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable="atcPIDBelle(3,2)" if convertedFromBelle
                                        else ("kaonIDNN" if usePIDNN else "kaonID"),
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    proton = Particle('p+',
                      MVAConfiguration(variables=chargedVariables,
                                       target='isPrimarySignal'),
                      PreCutConfiguration(userCut=charged_user_cut,
                                          bestCandidateMode='highest',
                                          bestCandidateVariable="atcPIDBelle(4,3)" if convertedFromBelle
                                          else ("protonIDNN" if usePIDNN else "protonID"),
                                          bestCandidateCut=20),
                      PostCutConfiguration(bestCandidateCut=10, value=0.01))
    proton.addChannel(['p+:FSP'])

    electron = Particle('e+',
                        MVAConfiguration(variables=chargedVariables,
                                         target='isPrimarySignal'),
                        PreCutConfiguration(userCut=charged_user_cut,
                                            bestCandidateMode='highest',
                                            bestCandidateVariable="eIDBelle" if convertedFromBelle
                                            else ("electronIDNN" if usePIDNN else "electronID"),
                                            bestCandidateCut=10),
                        PostCutConfiguration(bestCandidateCut=5, value=0.01))
    electron.addChannel(['e+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable="muIDBelle" if convertedFromBelle
                                        else ("muonIDNN" if usePIDNN else "muonID"),
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=5, value=0.01))
    muon.addChannel(['mu+:FSP'])

    if convertedFromBelle:
        gamma_cut = 'goodBelleGamma == 1 and clusterBelleQuality == 0'
    else:
        # Same as goodBelleGamma == 1
        gamma_cut = '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.15]]'
    if specific:
        gamma_cut += ' and isInRestOfEvent > 0.5'

    gamma = Particle('gamma',
                     MVAConfiguration(variables=['clusterReg', 'clusterNHits', 'clusterTiming', 'extraInfo(preCut_rank)',
                                                 'clusterE9E25', 'pt', 'E', 'pz'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut=gamma_cut,
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40),
                     PostCutConfiguration(bestCandidateCut=20, value=0.01))
    gamma.addChannel(['gamma:FSP'])
    gamma.addChannel(['gamma:V0'],
                     MVAConfiguration(variables=['pt', 'E', 'pz', 'extraInfo(preCut_rank)'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut='',
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40))

    kl0_cut = ''
    if specific:
        kl0_cut += 'isInRestOfEvent > 0.5'

    KL0 = Particle('K_L0',
                   MVAConfiguration(variables=['E', 'klmClusterTiming'],
                                    target='isSignal'),
                   PreCutConfiguration(userCut=kl0_cut,
                                       bestCandidateVariable='abs(dM)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.01))
    KL0.addChannel(['K_L0:FSP'])
    # endregion 1st stage

    # region 2nd stage
    variables.addAlias('eneAsy', 'formula((daughter(0,E)-daughter(1,E))/(daughter(0,E)+daughter(1,E)))')
    if convertedFromBelle:
        pi0_cut = '0.08 < InvM < 0.18'
        if specific:
            pi0_cut += ' and isInRestOfEvent > 0.5'

        pi0 = Particle('pi0',
                       MVAConfiguration(variables=['InvM', 'extraInfo(preCut_rank)', 'chiProb', 'abs(BellePi0SigM)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E',
                                                   'daughter({},E)', 'daughter({},clusterReg)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut=pi0_cut,
                                           bestCandidateVariable='abs(BellePi0SigM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        pi0.addChannel(['pi0:FSP'])

        Lam_cut = '0.9 < M < 1.3'
        if specific:
            Lam_cut += ' and isInRestOfEvent > 0.5'
        L0 = Particle(
            'Lambda0',
            MVAConfiguration(
                variables=[
                    'dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                    'useCMSFrame(E)', 'daughterAngle(0,1)',
                    'cosAngleBetweenMomentumAndVertexVector',
                    'extraInfo(preCut_rank)', 'extraInfo(goodLambda)', 'extraInfo(ksnbVLike)',
                    'extraInfo(ksnbNoLam)'],
                target='isSignal'),
            PreCutConfiguration(
                userCut=Lam_cut,
                bestCandidateVariable='abs(dM)',
                bestCandidateCut=20),
            PostCutConfiguration(
                bestCandidateCut=10,
                value=0.01)
        )
        L0.addChannel(['Lambda0:V0'])
    else:
        pi0 = Particle('pi0',
                       MVAConfiguration(variables=['M', 'daughter({},extraInfo(SignalProbability))', 'extraInfo(preCut_rank)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E', 'abs(dM)',
                                                   'daughter({},E)', 'daughter({},clusterReg)', 'eneAsy'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.08 < M < 0.18',
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        pi0.addChannel(['gamma', 'gamma'])

        L0 = Particle(
            'Lambda0',
            MVAConfiguration(
                variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                           'useCMSFrame(E)', 'daughterAngle(0,1)',
                           'daughter({},extraInfo(SignalProbability))',
                           'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                           'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)'],
                target='isSignal'),
            PreCutConfiguration(
                userCut='0.9 < M < 1.3',
                bestCandidateVariable='abs(dM)',
                bestCandidateCut=20),
            PostCutConfiguration(
                bestCandidateCut=10,
                value=0.01)
        )
        L0.addChannel(['p+', 'pi-'])

        Lam_cut = '0.9 < M < 1.3'
        if specific:
            Lam_cut += ' and isInRestOfEvent > 0.5'

        L0.addChannel(
            ['Lambda0:V0'],
            MVAConfiguration(
                variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M',
                           'useCMSFrame(E)', 'daughterAngle(0,1)', 'extraInfo(preCut_rank)', 'abs(dM)',
                           'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                           'daughter({}, dz)', 'daughter({}, dr)'],
                target='isSignal'),
            PreCutConfiguration(
                userCut=Lam_cut,
                bestCandidateVariable='abs(dM)',
                bestCandidateCut=20))
    # endregion 2nd stage

    # region 3rd stage
    if convertedFromBelle:
        ks0_cut = '0.4 < M < 0.6'
        if specific:
            ks0_cut += ' and isInRestOfEvent > 0.5'

        KS0 = Particle(
            'K_S0',
            MVAConfiguration(
                variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                           'useCMSFrame(E)', 'daughterAngle(0,1)',
                           'cosAngleBetweenMomentumAndVertexVector',
                           'extraInfo(preCut_rank)', 'extraInfo(goodKs)', 'extraInfo(ksnbVLike)',
                           'extraInfo(ksnbNoLam)', 'extraInfo(ksnbStandard)'],
                target='isSignal'),
            PreCutConfiguration(
                userCut=ks0_cut,
                bestCandidateVariable='abs(dM)',
                bestCandidateCut=20),
            PostCutConfiguration(
                bestCandidateCut=10,
                value=0.01)
        )
        KS0.addChannel(['K_S0:V0'])
    else:
        KS0 = Particle(
            'K_S0',
            MVAConfiguration(
                variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                           'useCMSFrame(E)', 'daughterAngle(0,1)',
                           'useRestFrame(daughterAngle(0,1))',
                           'daughter({},extraInfo(SignalProbability))',
                           'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                           'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)', 'eneAsy'],
                target='isSignal'),
            PreCutConfiguration(
                userCut='0.4 < M < 0.6',
                bestCandidateVariable='abs(dM)',
                bestCandidateCut=20),
            PostCutConfiguration(
                bestCandidateCut=10,
                value=0.01)
        )
        KS0.addChannel(['pi+', 'pi-'])
        KS0.addChannel(['pi0', 'pi0'])

        ks0_cut = '0.4 < M < 0.6'
        if specific:
            ks0_cut += ' and isInRestOfEvent > 0.5'

        KS0.addChannel(
            ['K_S0:V0'],
            MVAConfiguration(
                variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M',
                           'useCMSFrame(E)', 'daughterAngle(0,1)', 'extraInfo(preCut_rank)', 'abs(dM)',
                           'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                           'daughter({}, dz)', 'daughter({}, dr)'],
                target='isSignal'),
            PreCutConfiguration(
                userCut=ks0_cut,
                bestCandidateVariable='abs(dM)',
                bestCandidateCut=20))

    SigmaP = Particle(
        'Sigma+',
        MVAConfiguration(
            variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                       'useCMSFrame(E)', 'daughterAngle(0,1)',
                       'daughter({},extraInfo(SignalProbability))',
                       'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                       'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)'],
            target='isSignal'),
        PreCutConfiguration(
            userCut='1.0 < M < 1.4',
            bestCandidateVariable='abs(dM)',
            bestCandidateCut=20),
        PostCutConfiguration(
            bestCandidateCut=10,
            value=0.01)
    )
    SigmaP.addChannel(['p+', 'pi0'])
    # endregion 3rd stage

    # region 4th stage
    pi0vetovars = ['extraInfo(pi0vetoMass)', 'extraInfo(pi0vetoEneAsy)']

    # variables for D mesons
    variables.addAlias('significanceOfFlightDistance', 'formula(flightDistance/flightDistanceErr)')
    Dtag_vars = [
        'pValueCombinationOfDaughters(extraInfo(SignalProbability))',
        'daughter({},extraInfo(SignalProbability))',
        'chiProb',
        'daughter({}, chiProb)',
        'extraInfo(preCut_rank)',
        'abs(dM)', 'Q',
        'useRestFrame(daughter({}, p))',
        'useRestFrame(daughter({}, distance))',
        'useCMSFrame(p)',
        'decayAngle({1..})',
        'daughterAngle({},{})',
        'daughterInvM({},{})',
        'daughterInvM({},{},{})',
        'daughterInvM({},{},{},{})',
        'daughterInvM({},{},{},{},{})',
        'daughter({},extraInfo(decayModeID))',
        'cosAngleBetweenMomentumAndVertexVector',
        'cosAngleBetweenMomentumAndVertexVectorInXYPlane',
        'vertexDistance',
        'vertexDistanceSignificance',
        'vertexDistanceOfDaughter({})',
        'vertexDistanceOfDaughterSignificance({})',
        'significanceOfFlightDistance'
    ]

    D0 = Particle('D0',
                  MVAConfiguration(variables=Dtag_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    D0.addChannel(['K-', 'pi+'])
    D0.addChannel(['K-', 'pi+', 'pi0'])
    D0.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
    D0.addChannel(['K-', 'pi+', 'pi+', 'pi-', 'pi0'])
    D0.addChannel(['K_S0', 'pi+', 'pi-'])
    D0.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])

    D0.addChannel(['K-', 'pi+', 'pi0', 'pi0'])
    D0.addChannel(['pi-', 'pi+'])
    D0.addChannel(['pi-', 'pi+', 'pi+', 'pi-'])
    D0.addChannel(['pi-', 'pi+', 'pi0'])
    D0.addChannel(['pi-', 'pi+', 'pi0', 'pi0'])
    D0.addChannel(['K-', 'K+'])
    D0.addChannel(['K-', 'K+', 'pi0'])
    D0.addChannel(['K-', 'K+', 'K_S0'])
    D0.addChannel(['K_S0', 'pi0'])

    DP = Particle('D+',
                  MVAConfiguration(variables=Dtag_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    DP.addChannel(['K-', 'pi+', 'pi+'])
    DP.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
    DP.addChannel(['K_S0', 'pi+'])
    DP.addChannel(['K_S0', 'pi+', 'pi0'])
    DP.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])
    DP.addChannel(['K-', 'K+', 'pi+'])

    DP.addChannel(['K-', 'K+', 'pi+', 'pi0'])
    DP.addChannel(['pi+', 'pi0'])
    DP.addChannel(['pi+', 'pi+', 'pi-'])
    DP.addChannel(['pi+', 'pi+', 'pi-', 'pi0'])
    DP.addChannel(['K+', 'K_S0', 'K_S0'])

    DS = Particle('D_s+',
                  MVAConfiguration(variables=Dtag_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.68 < M < 2.1',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    DS.addChannel(['K+', 'pi+', 'pi-'])
    DS.addChannel(['K_S0', 'K+'])
    DS.addChannel(['K_S0', 'K_S0', 'pi+'])
    DS.addChannel(['K+', 'K-', 'pi+', 'pi0'])
    DS.addChannel(['K-', 'K_S0', 'pi+', 'pi+'])

    DS.addChannel(['K+', 'K-', 'pi+'])
    DS.addChannel(['K+', 'K_S0', 'pi+', 'pi-'])
    DS.addChannel(['K+', 'K-', 'pi+', 'pi+', 'pi-'])
    DS.addChannel(['pi+', 'pi+', 'pi-'])
    DS.addChannel(['K_S0', 'pi+'])
    DS.addChannel(['K_S0', 'pi+', 'pi0'])

    LC = Particle('Lambda_c+',
                  MVAConfiguration(variables=Dtag_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='2.2 < M < 2.4',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    LC.addChannel(['p+', 'K-', 'pi+'])
    LC.addChannel(['p+', 'K-', 'pi+', 'pi0'])
    LC.addChannel(['p+', 'K_S0'])
    LC.addChannel(['Lambda0', 'pi+'])
    LC.addChannel(['Lambda0', 'pi+', 'pi0'])
    LC.addChannel(['Lambda0', 'pi+', 'pi+', 'pi-'])

    LC.addChannel(['p+', 'pi-', 'pi+'])
    LC.addChannel(['p+', 'K-', 'K+'])
    LC.addChannel(['p+', 'K-', 'pi+', 'pi0', 'pi0'])
    LC.addChannel(['p+', 'pi+', 'pi+', 'pi-', 'pi-'])
    LC.addChannel(['p+', 'K_S0', 'pi0'])
    LC.addChannel(['p+', 'K_S0', 'pi+', 'pi-'])
    LC.addChannel(['Lambda0', 'pi+', 'gamma'],
                  MVAConfiguration(variables=Dtag_vars+pi0vetovars, target='isSignal'),
                  pi0veto=True)
    LC.addChannel(['Lambda0', 'pi+', 'pi0', 'gamma'],
                  MVAConfiguration(variables=Dtag_vars+pi0vetovars, target='isSignal'),
                  pi0veto=True)
    LC.addChannel(['Lambda0', 'pi+', 'pi-', 'pi+', 'gamma'],
                  MVAConfiguration(variables=Dtag_vars+pi0vetovars, target='isSignal'),
                  pi0veto=True)
    LC.addChannel(['Sigma+', 'pi+', 'pi-'])
    LC.addChannel(['Sigma+', 'pi+', 'pi-', 'pi0'])
    LC.addChannel(['Sigma+', 'pi0'])
    # endregion 4th stage

    # region 5th stage
    Dstar_vars = Dtag_vars + ['pointingAngle(0)', 'massDifference(0)']
    DSP = Particle('D*+',
                   MVAConfiguration(variables=Dstar_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))
    DSP.addChannel(['D0', 'pi+'])
    DSP.addChannel(['D+', 'pi0'])
    DSP.addChannel(['D+', 'gamma'],
                   MVAConfiguration(variables=Dstar_vars+pi0vetovars, target='isSignal'),
                   pi0veto=True)

    DS0 = Particle('D*0',
                   MVAConfiguration(variables=Dstar_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))
    DS0.addChannel(['D0', 'pi0'])
    DS0.addChannel(['D0', 'gamma'],
                   MVAConfiguration(variables=Dstar_vars+pi0vetovars, target='isSignal'),
                   pi0veto=True)

    DSS = Particle('D_s*+',
                   MVAConfiguration(variables=Dstar_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0.0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))
    DSS.addChannel(['D_s+', 'gamma'],
                   MVAConfiguration(variables=Dstar_vars+pi0vetovars, target='isSignal'),
                   pi0veto=True)
    DSS.addChannel(['D_s+', 'pi0'])
    # endregion 5th stage

    CharmTag_vars = [
        'pValueCombinationOfDaughters(extraInfo(SignalProbability))',
        'daughter({},extraInfo(SignalProbability))',
        'chiProb',
        'daughter({}, chiProb)',
        'extraInfo(preCut_rank)',
        'useCMSFrame(daughter({1..}, p))',
        'useCMSFrame(daughter({}, distance))',
        'decayAngle({1..})',
        'useCMSFrame(daughterAngle(0,{1..}))',
        'cosAngleBetweenMomentumAndVertexVector',
        'cosAngleBetweenMomentumAndVertexVectorInXYPlane',
        'dr', 'dz', 'dx', 'dy', 'distance', 'significanceOfDistance',
        'daughter({},extraInfo(decayModeID))',
        'useCMSFrame(p)',
        'useCMSFrame(angleBetweenDaughterAndRecoil(0))',
        'vertexDistance',
        'vertexDistanceSignificance',
        'vertexDistanceOfDaughter({})',
        'vertexDistanceOfDaughterSignificance({})',
        'massDiffRecoil({2..})',
        'significanceOfFlightDistance',
    ]

    ccbarTag_user_cut = '1.8 < mRecoil < 2.6'
    variables.addAlias('ccbarTagSignalBinary', 'conditionalVariableSelector(ccbarTagSignal==1,1,0)')

    # region 6th stage
    LambdaCTag = Particle(
        'Lambda_c+:ccbarTag',
        MVAConfiguration(
            config='--nTrees 400 --nCutLevels 10 --nLevels 3 --shrinkage 0.1 --randRatio 0.5',  # --flatnessLoss 1.0',
            variables=CharmTag_vars,
            spectators={'mRecoil': (1.8, 2.6)},
            target='ccbarTagSignalBinary'),
        PreCutConfiguration(
            userCut=ccbarTag_user_cut,
            bestCandidateMode='highest',
            bestCandidateVariable='pValueCombinationOfDaughters(extraInfo(SignalProbability))',
            noSignalSampling=True,
            bkgSamplingFactor=0.01,
            bestCandidateCut=20),
        PostCutConfiguration(bestCandidateCut=20))
    LambdaCTag.addChannel(['D0', 'p+'])
    LambdaCTag.addChannel(['D*0', 'p+'])
    LambdaCTag.addChannel(['D+', 'p+', 'pi-'])
    LambdaCTag.addChannel(['D*+', 'p+', 'pi-'])
    LambdaCTag.addChannel(['D_s+', 'p+', 'K-'])
    LambdaCTag.addChannel(['D_s*+', 'p+', 'K-'])

    if addCharged:
        LambdaCTagCharged = Particle(
            'Lambda_c+:ccbarTagCharged',
            MVAConfiguration(
                config='--nTrees 400 --nCutLevels 10 --nLevels 3 --shrinkage 0.1 --randRatio 0.5',  # --flatnessLoss 1.0',
                variables=CharmTag_vars,
                spectators={'mRecoil': (1.8, 2.6)},
                target='ccbarTagSignalBinary'),
            PreCutConfiguration(
                userCut=ccbarTag_user_cut,
                bestCandidateMode='highest',
                bestCandidateVariable='pValueCombinationOfDaughters(extraInfo(SignalProbability))',
                noSignalSampling=True,
                bkgSamplingFactor=0.1,
                bestCandidateCut=20),
            PostCutConfiguration(bestCandidateCut=20))
        LambdaCTagCharged.addChannel(['D0', 'p+', 'pi+', 'pi-'])
        LambdaCTagCharged.addChannel(['D*0', 'p+', 'pi+', 'pi-'])
        LambdaCTagCharged.addChannel(['D+', 'p+', 'pi-', 'pi+', 'pi-'])
        LambdaCTagCharged.addChannel(['D*+', 'p+', 'pi-', 'pi+', 'pi-'])
        LambdaCTagCharged.addChannel(['D_s+', 'p+', 'K-', 'pi+', 'pi-'])
        LambdaCTagCharged.addChannel(['D_s*+', 'p+', 'K-', 'pi+', 'pi-'])
        LambdaCTagCharged.addChannel(['D0', 'p+', 'K+', 'K-'])
        LambdaCTagCharged.addChannel(['D*0', 'p+', 'K+', 'K-'])
        LambdaCTagCharged.addChannel(['D+', 'p+', 'pi-', 'K+', 'K-'])
        LambdaCTagCharged.addChannel(['D*+', 'p+', 'pi-', 'K+', 'K-'])

        LambdaCTagCharged.addChannel(['D0', 'p+', 'p+', 'anti-p-'])
        LambdaCTagCharged.addChannel(['D*0', 'p+', 'p+', 'anti-p-'])

    if addPi0:
        LambdaCTagPi0 = Particle(
            'Lambda_c+:ccbarTagPi0',
            MVAConfiguration(
                config='--nTrees 400 --nCutLevels 10 --nLevels 3 --shrinkage 0.1 --randRatio 0.5',  # --flatnessLoss 1.0',
                variables=CharmTag_vars,
                spectators={'mRecoil': (1.8, 2.6)},
                target='ccbarTagSignalBinary'),
            PreCutConfiguration(
                userCut=ccbarTag_user_cut,
                bestCandidateMode='highest',
                bestCandidateVariable='pValueCombinationOfDaughters(extraInfo(SignalProbability))',
                noSignalSampling=True,
                bkgSamplingFactor=0.05,
                bestCandidateCut=20),
            PostCutConfiguration(bestCandidateCut=20))
        LambdaCTagPi0.addChannel(['D0', 'p+', 'pi0'])
        LambdaCTagPi0.addChannel(['D*0', 'p+', 'pi0'])
        LambdaCTagPi0.addChannel(['D+', 'p+', 'pi-', 'pi0'])
        LambdaCTagPi0.addChannel(['D*+', 'p+', 'pi-', 'pi0'])
        LambdaCTagPi0.addChannel(['D_s+', 'p+', 'K-', 'pi0'])
        LambdaCTagPi0.addChannel(['D_s*+', 'p+', 'K-', 'pi0'])

    if addStrangness:
        LambdaCTagStrange = Particle(
            'Lambda_c+:ccbarTagStrange',
            MVAConfiguration(
                config='--nTrees 400 --nCutLevels 10 --nLevels 3 --shrinkage 0.1 --randRatio 0.5',  # --flatnessLoss 1.0',
                variables=CharmTag_vars,
                spectators={'mRecoil': (1.8, 2.6)},
                target='ccbarTagSignalBinary'),
            PreCutConfiguration(
                userCut=ccbarTag_user_cut,
                bestCandidateMode='highest',
                bestCandidateVariable='pValueCombinationOfDaughters(extraInfo(SignalProbability))',
                noSignalSampling=True,
                bkgSamplingFactor=0.05,
                bestCandidateCut=20),
            PostCutConfiguration(bestCandidateCut=20))
        LambdaCTagStrange.addChannel(['D0', 'Lambda0', 'K+'])
        LambdaCTagStrange.addChannel(['D*0', 'Lambda0', 'K+'])
        LambdaCTagStrange.addChannel(['D*+', 'Lambda0', 'K_S0'])
        LambdaCTagStrange.addChannel(['D_s+', 'Lambda0'])
        LambdaCTagStrange.addChannel(['D_s*+', 'Lambda0'])
        LambdaCTagStrange.addChannel(['D*+', 'p+', 'K_S0', 'K-'])
        LambdaCTagStrange.addChannel(['D_s+', 'p+', 'K_S0', 'pi-'])
        LambdaCTagStrange.addChannel(['D_s*+', 'p+', 'K_S0', 'pi-'])
        LambdaCTagStrange.addChannel(['D0', 'Lambda0', 'K_S0', 'pi+'])
        LambdaCTagStrange.addChannel(['D+', 'Lambda0', 'K+', 'pi-'])
        LambdaCTagStrange.addChannel(['D*+', 'Lambda0', 'K+', 'pi-'])

        LambdaCTagStrange.addChannel(['D0', 'Lambda0', 'K+', 'pi0'])

        LambdaCTagStrange.addChannel(['D0', 'Lambda0', 'K+', 'pi+', 'pi-'])
        LambdaCTagStrange.addChannel(['D*0', 'Lambda0', 'K+', 'pi+', 'pi-'])
        LambdaCTagStrange.addChannel(['D_s+', 'Lambda0', 'pi+', 'pi-'])
        LambdaCTagStrange.addChannel(['D_s*+', 'Lambda0', 'pi+', 'pi-'])

        LambdaCTagStrange.addChannel(['D0', 'Lambda0', 'K+', 'pi+', 'pi-', 'pi0'])
        LambdaCTagStrange.addChannel(['D_s+', 'Lambda0', 'pi+', 'pi-', 'pi0'])
        LambdaCTagStrange.addChannel(['D_s*+', 'Lambda0', 'pi+', 'pi-', 'pi0'])

    # endregion 6th stage

    particles = []
    particles.append(pion)
    particles.append(kaon)
    particles.append(proton)
    particles.append(muon)
    particles.append(electron)
    particles.append(gamma)

    particles.append(pi0)
    particles.append(KS0)
    particles.append(SigmaP)
    particles.append(L0)

    particles.append(D0)
    particles.append(DP)
    particles.append(DS)
    particles.append(LC)

    particles.append(DS0)
    particles.append(DSP)
    particles.append(DSS)

    particles.append(LambdaCTag)
    if addCharged:
        particles.append(LambdaCTagCharged)
    if addPi0:
        particles.append(LambdaCTagPi0)
    if addStrangness:
        particles.append(LambdaCTagStrange)

    return particles


def get_unittest_channels(specific=False):
    chargedVariables = ['electronID', 'extraInfo(preCut_rank)',
                        'kaonID', 'protonID', 'muonID',
                        'p', 'pt', 'pz', 'dr', 'dz', 'chiProb']

    specific_cut = ''
    if specific:
        specific_cut = ' and isInRestOfEvent > 0.5'

    pion = Particle('pi+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]' + specific_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='pionID',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]' + specific_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='kaonID',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]' + specific_cut,
                                        bestCandidateVariable='muonID',
                                        bestCandidateMode='highest',
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=5, value=0.01))
    muon.addChannel(['mu+:FSP'])

    gamma = Particle('gamma',
                     MVAConfiguration(variables=['clusterReg', 'clusterNHits', 'clusterTiming', 'clusterE9E25',
                                                 'pt', 'E', 'pz', 'extraInfo(preCut_rank)'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut='E > 0.05' + specific_cut,
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40),
                     PostCutConfiguration(bestCandidateCut=20, value=0.01))
    gamma.addChannel(['gamma:FSP'])
    gamma.addChannel(['gamma:V0'],
                     MVAConfiguration(variables=['pt', 'E', 'pz'],
                                      target='isPrimarySignal'))

    pi0 = Particle('pi0',
                   MVAConfiguration(variables=['M', 'daughter({},extraInfo(SignalProbability))', 'extraInfo(preCut_rank)',
                                               'daughterAngle(0,1)', 'pt', 'pz', 'E', 'abs(dM)'],
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0.08 < M < 0.18',
                                       bestCandidateVariable='abs(dM)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pi0.addChannel(['gamma', 'gamma'])

    intermediate_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
                         'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)', 'abs(dM)',
                         'useRestFrame(daughter({}, p))',
                         'useRestFrame(daughter({}, distance))',
                         'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
                         'daughterInvariantMass({},{})', 'daughterInvariantMass({},{},{})', 'daughterInvariantMass({},{},{},{})',
                         'daughterInvariantMass({},{},{},{},{})', 'dQ', 'Q', 'dM', 'daughter({},extraInfo(decayModeID))']

    D0 = Particle('D0',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    D0.addChannel(['K-', 'pi+'])
    D0.addChannel(['K-', 'pi+', 'pi0'])
    D0.addChannel(['pi-', 'pi+'])

    D0_SL = Particle('D0:semileptonic',
                     MVAConfiguration(variables=intermediate_vars,
                                      target='isSignalAcceptMissingNeutrino'),
                     PreCutConfiguration(userCut='',
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                         bestCandidateCut=20),
                     PostCutConfiguration(bestCandidateCut=10, value=0.001))

    D0_SL.addChannel(['K-', 'mu+'])
    D0_SL.addChannel(['K-', 'pi0', 'mu+'])

    particles = [gamma, muon, pion, kaon, pi0, D0, D0_SL]
    return particles


def get_fr_channels(convertedFromBelle=False):
    """
    Get channels of original FR for comparison
    @param convertedFromBelle whether to use Belle variables which is necessary for b2bii converted data (default is False)
    """

    if convertedFromBelle:
        # Using Belle specific Variables for e-ID, mu-ID and K-ID
        # atcPIDBelle(3,2) is used as K-ID
        # atcPIDBelle(4,2) and atcPIDBelle(4,3) are used as pr-ID
        chargedVariables = ['eIDBelle',
                            'atcPIDBelle(3,2)',
                            'atcPIDBelle(4,2)', 'atcPIDBelle(4,3)',
                            'muIDBelle',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']
    else:
        chargedVariables = ['electronID', 'kaonID', 'protonID', 'muonID',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']

    charged_user_cut = '[dr < 2] and [abs(dz) < 4]'

    pion = Particle('pi+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='pionID' if not convertedFromBelle else 'atcPIDBelle(2,3)',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='kaonID' if not convertedFromBelle else 'atcPIDBelle(3,2)',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    electron = Particle('e+',
                        MVAConfiguration(variables=chargedVariables,
                                         target='isPrimarySignal'),
                        PreCutConfiguration(userCut=charged_user_cut,
                                            bestCandidateMode='highest',
                                            bestCandidateVariable='electronID' if not convertedFromBelle else 'eIDBelle',
                                            bestCandidateCut=10),
                        PostCutConfiguration(bestCandidateCut=10, value=0.01))
    electron.addChannel(['e+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='muonID' if not convertedFromBelle else 'muIDBelle',
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    muon.addChannel(['mu+:FSP'])

    high_energy_photon = '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.09] or [clusterReg == 3 and E > 0.16]]'
    gamma = Particle('gamma',
                     MVAConfiguration(variables=['clusterReg', 'clusterNHits', 'clusterTiming', 'extraInfo(preCut_rank)',
                                                 'clusterE9E25', 'pt', 'E', 'pz'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut=high_energy_photon if not convertedFromBelle else
                                         'goodBelleGamma == 1 and clusterBelleQuality == 0',
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40),
                     PostCutConfiguration(bestCandidateCut=20, value=0.01))
    gamma.addChannel(['gamma:FSP'])
    gamma.addChannel(['gamma:V0'],
                     MVAConfiguration(variables=['pt', 'E', 'pz', 'extraInfo(preCut_rank)'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut='',
                                         bestCandidateMode='highest',
                                         bestCandidateVariable='E',
                                         bestCandidateCut=40))

    if convertedFromBelle:

        pi0 = Particle('pi0',
                       MVAConfiguration(variables=['InvM', 'extraInfo(preCut_rank)', 'chiProb', 'abs(BellePi0SigM)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.08 < InvM < 0.18',
                                           bestCandidateVariable='abs(BellePi0SigM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        pi0.addChannel(['pi0:FSP'])

        KS0 = Particle('K_S0',
                       MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                   'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                   'cosAngleBetweenMomentumAndVertexVector',
                                                   'extraInfo(preCut_rank)', 'extraInfo(goodKs)', 'extraInfo(ksnbVLike)',
                                                   'extraInfo(ksnbNoLam)', 'extraInfo(ksnbStandard)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.4 < M < 0.6',
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        KS0.addChannel(['K_S0:V0'])

    else:

        pi0 = Particle('pi0',
                       MVAConfiguration(variables=['M', 'daughter({},extraInfo(SignalProbability))', 'extraInfo(preCut_rank)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E', 'abs(dM)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.08 < M < 0.18',
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        pi0.addChannel(['gamma', 'gamma'])

        KS0 = Particle('K_S0',
                       MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                                   'useCMSFrame(E)', 'daughterAngle(0,1)',
                                                   'daughter({},extraInfo(SignalProbability))',
                                                   'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                   'daughter({}, dz)', 'daughter({}, dr)', 'extraInfo(preCut_rank)'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut='0.4 < M < 0.6',
                                           bestCandidateVariable='abs(dM)',
                                           bestCandidateCut=20),
                       PostCutConfiguration(bestCandidateCut=10, value=0.01))
        KS0.addChannel(['pi+', 'pi-'])
        KS0.addChannel(['pi0', 'pi0'])
        KS0.addChannel(['K_S0:V0'],
                       MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M',
                                                   'useCMSFrame(E)', 'daughterAngle(0,1)', 'extraInfo(preCut_rank)', 'abs(dM)',
                                                   'useRestFrame(daughter({}, p))', 'cosAngleBetweenMomentumAndVertexVector',
                                                   'daughter({}, dz)', 'daughter({}, dr)'],
                                        target='isSignal'))

    # variables for D mesons and J/Psi
    intermediate_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
                         'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)', 'abs(dM)',
                         'useRestFrame(daughter({}, p))',
                         'useRestFrame(daughter({}, distance))',
                         'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
                         'daughterInvariantMass({},{})', 'daughterInvariantMass({},{},{})', 'daughterInvariantMass({},{},{},{})',
                         'daughterInvariantMass({},{},{},{},{})', 'dQ', 'Q', 'dM', 'daughter({},extraInfo(decayModeID))']

    D0 = Particle('D0',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))
    D0.addChannel(['K-', 'pi+'])
    D0.addChannel(['K-', 'pi+', 'pi0'])
    D0.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
    D0.addChannel(['pi-', 'pi+'])
    D0.addChannel(['pi-', 'pi+', 'pi0'])
    D0.addChannel(['K_S0', 'pi0'])
    D0.addChannel(['K_S0', 'pi+', 'pi-'])
    D0.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])
    D0.addChannel(['K-', 'K+'])
    D0.addChannel(['K-', 'K+', 'K_S0'])

    DP = Particle('D+',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.7 < M < 1.95',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DP.addChannel(['K-', 'pi+', 'pi+'])
    DP.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
    DP.addChannel(['K-', 'K+', 'pi+'])
    DP.addChannel(['K-', 'K+', 'pi+', 'pi0'])
    DP.addChannel(['K_S0', 'pi+'])
    DP.addChannel(['K_S0', 'pi+', 'pi0'])
    DP.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])

    Jpsi = Particle('J/psi',
                    MVAConfiguration(variables=intermediate_vars,
                                     target='isSignal'),
                    PreCutConfiguration(userCut='2.5 < M < 3.7',
                                        bestCandidateVariable='abs(dM)',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.001))

    Jpsi.addChannel(['e+', 'e-'])
    Jpsi.addChannel(['mu+', 'mu-'])

    DSP = Particle('D*+',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DSP.addChannel(['D0', 'pi+'])
    DSP.addChannel(['D+', 'pi0'])

    DS0 = Particle('D*0',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DS0.addChannel(['D0', 'pi0'])
    DS0.addChannel(['D0', 'gamma'])

    DS = Particle('D_s+',
                  MVAConfiguration(variables=intermediate_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut='1.68 < M < 2.1',
                                      bestCandidateVariable='abs(dM)',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DS.addChannel(['K+', 'K_S0'])
    DS.addChannel(['K+', 'pi+', 'pi-'])
    DS.addChannel(['K+', 'K-', 'pi+'])
    DS.addChannel(['K+', 'K-', 'pi+', 'pi0'])
    DS.addChannel(['K+', 'K_S0', 'pi+', 'pi-'])
    DS.addChannel(['K-', 'K_S0', 'pi+', 'pi+'])
    DS.addChannel(['K+', 'K-', 'pi+', 'pi+', 'pi-'])
    DS.addChannel(['pi+', 'pi+', 'pi-'])

    DSS = Particle('D_s*+',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0.0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DSS.addChannel(['D_s+', 'gamma'])

    # note: these should not be correlated to Mbc (weak correlation of deltaE is OK)
    B_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
              'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)',
              'useRestFrame(daughter({}, p))',
              'useRestFrame(daughter({}, distance))',
              'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
              'dr', 'dz', 'dx', 'dy', 'distance', 'significanceOfDistance', 'deltaE', 'daughter({},extraInfo(decayModeID))']

    hadronic_user_cut = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BP = Particle('B+',
                  MVAConfiguration(variables=B_vars,
                                   spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronic_user_cut,
                                      bestCandidateMode='highest',
                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=20))

    BP.addChannel(['anti-D0', 'pi+'])
    BP.addChannel(['anti-D0', 'pi+', 'pi0'])
    BP.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-'])
    BP.addChannel(['anti-D0', 'D+'])
    BP.addChannel(['D_s+', 'anti-D0'])
    BP.addChannel(['anti-D*0', 'pi+'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi0'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-'])
    BP.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-', 'pi0'])
    BP.addChannel(['D_s*+', 'anti-D0'])
    BP.addChannel(['D_s+', 'anti-D*0'])
    BP.addChannel(['anti-D0', 'K+'])
    BP.addChannel(['D-', 'pi+', 'pi+'])
    BP.addChannel(['J/psi', 'K+'])
    BP.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
    BP.addChannel(['J/psi', 'K+', 'pi0'])
    BP.addChannel(['J/psi', 'K_S0', 'pi+'])

    B0 = Particle('B0',
                  MVAConfiguration(variables=B_vars,
                                   spectators={'Mbc': (5.23, None), 'cosThetaBetweenParticleAndNominalB': (-10, 10)},
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronic_user_cut,
                                      bestCandidateMode='highest',
                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
                                      bestCandidateCut=20),
                  PostCutConfiguration(bestCandidateCut=20))
    B0.addChannel(['D-', 'pi+'])
    B0.addChannel(['D-', 'pi+', 'pi0'])
    B0.addChannel(['D-', 'pi+', 'pi+', 'pi-'])
    B0.addChannel(['anti-D0', 'pi0'])
    B0.addChannel(['D_s+', 'D-'])
    B0.addChannel(['D*-', 'pi+'])
    B0.addChannel(['D*-', 'pi+', 'pi0'])
    B0.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
    B0.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
    B0.addChannel(['D_s*+', 'D-'])
    B0.addChannel(['D_s+', 'D*-'])
    B0.addChannel(['D_s*+', 'D*-'])
    B0.addChannel(['J/psi', 'K_S0'])
    B0.addChannel(['J/psi', 'K+', 'pi-'])
    B0.addChannel(['J/psi', 'K_S0', 'pi+', 'pi-'])

    particles = []
    particles.append(pion)
    particles.append(kaon)
    particles.append(muon)
    particles.append(electron)
    particles.append(gamma)

    particles.append(pi0)
    particles.append(KS0)
    particles.append(Jpsi)

    particles.append(D0)
    particles.append(DP)
    particles.append(DS)

    particles.append(DS0)
    particles.append(DSP)
    particles.append(DSS)

    particles.append(B0)
    particles.append(BP)

    return particles


def get_mode_names(particle_name: str,
                   hadronic=True,
                   semileptonic=False,
                   removeSLD=True,
                   remove_list_labels=True,
                   **channel_kwargs) -> list:
    """
    Get the ordered list of mode names for a given FEI particle name

    Arguments:
        particle_name(str): the name of the particle, e.g. B0 or B+
        hadronic(bool): whether to include hadronic modes
        semileptonic(bool): whether to include semileptonic modes
        removeSLD(bool): whether to remove the semileptonic D and D* modes, should be True for FEI skim
        remove_list_labels(bool): whether to remove the generic and semileptonic labels from the mode names
        channel_kwargs: keyword arguments for get_default_channels

    Returns:
        list(str): the list of mode names, or empty list if the particle was not found
    """
    if hadronic and semileptonic:
        B2INFO('Both semileptonic and hadronic arguments are set to True, set one of them to False for a more definite result.')
    if not hadronic and not semileptonic:
        B2INFO('Both semileptonic and hadronic arguments are set to False, set one of them to True for a more definite result.')
        return []
    channel_kwargs.update({'hadronic': hadronic,
                           'semileptonic': semileptonic,
                          'removeSLD': removeSLD
                           })
    channels = get_default_channels(**channel_kwargs)
    modes = []
    conjugate_name = particle_name.replace('-', '+')
    for channel in channels:
        if channel.name == particle_name or channel.name == conjugate_name:
            modes += [d_channel.label.split(' ==> ')[1] for d_channel in channel.channels]
    if remove_list_labels:
        modes = [mode.replace(':generic', '').replace(':semileptonic', '') for mode in modes]
    return modes
