
# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This defines a function that outputs the  mdst outputs for skim testing purposes.
"""

from basf2 import *
import os
import sys
import inspect
from vertex import *
from analysisPath import *
from modularAnalysis import *

# For channels in fei skim
from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


def encodeSkimName(skimListName):
    """Returns the skim code used in the output Udst file name."""
    skimCode = skimListName[0:7]

    if (skimListName == 'PRsemileptonicUntagged'):
        skimCode = '11110100'
    if (skimListName == 'BottomoniumUpsilon'):
        skimCode = '15440100'
    if (skimListName == 'BottomoniumEtabExclusive'):
        skimCode = '15420100'
    if (skimListName == 'SLUntagged'):
        skimCode = '11160200'
    if (skimListName == 'LeptonicUntagged'):
        skimCode = '11130300'
    if (skimListName == 'BtoDh_hh'):
        skimCode = '14140100'
    if (skimListName == 'BtoDh_Kspi0'):
        skimCode = '14120300'
    if (skimListName == 'BtoDh_Kshh'):
        skimCode = '14140200'
    if (skimListName == 'BtoDh_Kspipipi0'):
        skimCode = '14120400'
    if (skimListName == 'feiHadronicB0'):
        skimCode = '11180100'
    if (skimListName == 'feiHadronicBplus'):
        skimCode = '11180200'
    if (skimListName == 'feiSLB0WithOneLep'):
        skimCode = '11180300'
    if (skimListName == 'feiSLBplusWithOneLep'):
        skimCode = '11180400'
    if (skimListName == 'BtoXgamma'):
        skimCode = '12160100'
    if (skimListName == 'BtoXll'):
        skimCode = '12160200'
    if (skimListName == 'BtoPi0Pi0'):
        skimCode = '14120500'
    if (skimListName == 'Charm2BodyHadronic'):
        skimCode = '17240100'
    if (skimListName == 'Charm2BodyHadronicD0'):
        skimCode = '17230200'
    if (skimListName == 'Charm2BodyNeutrals'):
        skimCode = '17240300'
    if (skimListName == 'Charm2BodyNeutralsD0'):
        skimCode = '17230400'
    if (skimListName == 'Charm3BodyHadronic2'):
        skimCode = '17240500'
    if (skimListName == 'Charm3BodyHadronic'):
        skimCode = '17240600'
    if (skimListName == 'Charm3BodyHadronicD0'):
        skimCode = '17230700'
    if (skimListName == 'CharmRare'):
        skimCode = '17230800'
    if (skimListName == 'CharmSemileptonic'):
        skimCode = '17260900'
    if (skimListName == 'CharmlessHad'):
        skimCode = '14130100'
    if (skimListName == 'DoubleCharm'):
        skimCode = '14130200'
    if (skimListName == 'ISRpipicc'):
        skimCode = '16460100'
    if (skimListName == 'Systematics'):
        skimCode = '10600100'
    if (skimListName == 'SystematicsLambda'):
        skimCode = '10620200'
    if (skimListName == 'SystematicsTracking'):
        skimCode = '10600300'
    if (skimListName == 'Resonance'):
        skimCode = '10600400'
    if (skimListName == 'Tau'):
        skimCode = '18360100'
    if (skimListName == 'TCPV'):
        skimCode = '13160100'

    if (skimCode == skimListName[0:7]):
        B2FATAL("Skim unknown. Please add your skim to skimExpertFunctions.py!")

    return skimCode


def decodeSkimName(skimCode):
    """Returns the name of the skim given a skim code."""
    skimName = 'NoIdea'

    if (skimCode == '11110100'):
        skimName = 'PRsemileptonicUntagged'
    if (skimCode == '15440100'):
        skimName = 'BottomoniumUpsilon'
    if (skimCode == '15420100'):
        skimName = 'BottomoniumEtabExclusive'
    if (skimCode == '11160200'):
        skimName = 'SLUntagged'
    if (skimCode == '11130300'):
        skimName = 'LeptonicUntagged'
    if (skimCode == '14140100'):
        skimName = 'BtoDh_hh'
    if (skimCode == '14120300'):
        skimName = 'BtoDh_Kspi0'
    if (skimCode == '14140200'):
        skimName = 'BtoDh_Kshh'
    if (skimCode == '14120400'):
        skimName = 'BtoDh_Kspipipi0'
    if (skimCode == '11180100'):
        skimName = 'feiHadronicB0'
    if (skimCode == '11180200'):
        skimName = 'feiHadronicBplus'
    if (skimCode == '11180300'):
        skimName = 'feiSLB0WithOneLep'
    if (skimCode == '11180400'):
        skimName = 'feiSLBplusWithOneLep'
    if (skimCode == '12160100'):
        skimName = 'BtoXgamma'
    if (skimCode == '12160200'):
        skimName = 'BtoXll'
    if (skimCode == '14120500'):
        skimName = 'BtoPi0Pi0'
    if (skimCode == '17240100'):
        skimName = 'Charm2BodyHadronic'
    if (skimCode == '17230200'):
        skimName = 'Charm2BodyHadronicD0'
    if (skimCode == '17240300'):
        skimName = 'Charm2BodyNeutrals'
    if (skimCode == '17230400'):
        skimName = 'Charm2BodyNeutralsD0'
    if (skimCode == '17240500'):
        skimName = 'Charm3BodyHadronic2'
    if (skimCode == '17240600'):
        skimName = 'Charm3BodyHadronic'
    if (skimCode == '17230700'):
        skimName = 'Charm3BodyHadronicD0'
    if (skimCode == '17230800'):
        skimName = 'CharmRare'
    if (skimCode == '17260900'):
        skimName = 'CharmSemileptonic'
    if (skimCode == '14130100'):
        skimName = 'CharmlessHad'
    if (skimCode == '14130200'):
        skimName = 'DoubleCharm'
    if (skimCode == '16460100'):
        skimName = 'ISRpipicc'
    if (skimCode == '10600100'):
        skimName = 'Systematics'
    if (skimCode == '10620200'):
        skimName = 'SystematicsLambda'
    if (skimCode == '10600300'):
        skimName = 'SystematicsTracking'
    if (skimCode == '10600400'):
        skimName = 'Resonance'
    if (skimCode == '18360100'):
        skimName = 'Tau'
    if (skimCode == '13160100'):
        skimName = 'TCPV'

    if (skimName == 'NoIdea'):
        B2FATAL("Skim code  unknown. Please add your skim to skimExpertFunctions.py!")

    return skimName


def skimOutputMdst(skimDecayMode, skimParticleLists=[], outputParticleLists=[], includeArrays=[], path=analysis_main, *,
                   outputFile=None, dataDescription=None):
    """
    Create a new path for events that contain a non-empty particle list specified via skimParticleLists.
    Write the accepted events as a mdst file, saving only particles from skimParticleLists
    and from outputParticleLists. It outputs a .mdst file.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    :param str skimDecayMode: Name of the skim. If no outputFile is given this is
        also the name of the output filename. This name will be added to the
        FileMetaData as an extra data description "skimDecayMode"
    :param list(str) skimParticleLists: Names of the particle lists to skim for.
        An event will be accepted if at least one of the particle lists is not empty
    :param list(str) outputParticleLists: Names of the particle lists to store in
        the output in addition to the ones in skimParticleLists
    :param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param basf2.Path path: Path to add the skim output to. Defaults to the default analysis path
    :param str outputFile: Name of the output file if different from the skim name
    :param dict dataDescription: Additional data descriptions to add to the output file. For example {"mcEventType":"mixed"}
    """

    # if no outputfile is specified, set it to the skim name
    if outputFile is None:
        outputFile = skimDecayMode

    # make sure the output filename has the correct extension
    if not outputFile.endswith(".mdst.root"):
        outputFile += ".mdst.root"

    skimfilter = register_module('SkimFilter')
    skimfilter.set_name('SkimFilter_' + skimDecayMode)
    skimfilter.param('particleLists', skimParticleLists)
    path.add_module(skimfilter)
    filter_path = create_path()
    skimfilter.if_value('=1', filter_path, AfterConditionPath.CONTINUE)

    # add_independent_path() is rather expensive, only do this for skimmed events
    skim_path = create_path()
    saveParticleLists = skimParticleLists + outputParticleLists
    removeParticlesNotInLists(saveParticleLists, path=skim_path)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("skimDecayMode", skimDecayMode)
    outputMdst(outputFile)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)


def get_fei_skim_channels(B_extra_cut=None, hadronic=True, semileptonic=True, KLong=False, chargedB=True, neutralB=True,
                          convertedFromBelle=False, specific=False):
    """
    returns list of Particle objects with all default channels for running
    FEI on Upsilon(4S). For a training with analysis-specific signal selection,
    adding a cut on nRemainingTracksInRestOfEvent is recommended.
    @param B_extra_cut Additional user cut on rekombination of tag-B-mesons
    @param semileptonic whether to include hadronic B decays (default is True)
    @param semileptonic whether to include semileptonic B decays (default is True)
    @param KLong whether to include K_long decays into the training (default is True)
    @param chargedB whether to recombine charged B mesons (default is True)
    @param neutralB whether to recombine neutral B mesons (default is True)
    @param convertedFromBelle whether to use Belle variables which is necessary for b2bii converted data (default is False)
    @param specific if True, this adds isInRestOfEvent cut to all FSP
    """

    if chargedB is False and neutralB is False:
        print('FATAL: No B-Mesons will be recombined, since chargedB==False and neutralB==False was selected!')
        print('       Please reconfigure the arguments of get_default_channels() accordingly')
        sys.exit('Invalid get_default_channels configuration provided!')

    if convertedFromBelle:
        # Using Belle specific Variables for e-ID, mu-ID and K-ID
        # atcPIDBelle(3,2) is used as K-ID
        # atcPIDBelle(4,2) and atcPIDBelle(4,3) are used as pr-ID
        chargedVariables = ['eIDBelle',
                            'atcPIDBelle(3,2)', 'kIDBelle',
                            'atcPIDBelle(4,2)', 'atcPIDBelle(4,3)',
                            'muIDBelle',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']
    else:
        chargedVariables = ['electronID', 'kaonID', 'protonID', 'muonID',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']

    if specific:
        charged_user_cut = '[dr < 2] and [abs(dz) < 4] and isInRestOfEvent > 0.5'
    else:
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
                        PostCutConfiguration(bestCandidateCut=5, value=0.01))
    electron.addChannel(['e+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut=charged_user_cut,
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='muonID' if not convertedFromBelle else 'muIDBelle',
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=5, value=0.01))
    muon.addChannel(['mu+:FSP'])

    if convertedFromBelle:
        gamma_cut = 'goodBelleGamma == 1 and clusterBelleQuality == 0'
    else:
        gamma_cut = '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.09] or [clusterReg == 3 and E > 0.16]]'
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
                       MVAConfiguration(variables=['InvM', 'extraInfo(preCut_rank)', 'chiProb', 'abs(SigMBF)',
                                                   'daughterAngle(0,1)', 'pt', 'pz', 'E'],
                                        target='isSignal'),
                       PreCutConfiguration(userCut=pi0_cut,
                                           bestCandidateVariable='abs(SigMBF)',
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

    # variables for D mesons and J/Psi
    intermediate_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
                         'chiProb', 'daughter({}, chiProb)', 'extraInfo(preCut_rank)', 'abs(dM)',
                         'useRestFrame(daughter({}, p))',
                         'useRestFrame(daughter({}, distance))',
                         'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
                         'daughterInvariantMass({},{})', 'daughterInvariantMass({},{},{})', 'daughterInvariantMass({},{},{},{})',
                         'daughterInvariantMass({},{},{},{},{})', 'dQ', 'Q', 'dM', 'daughter({},extraInfo(decayModeID))']

    # TODO if specific:
    # We can not do this in the generic case (because this would heavily influence our performance on the unkown signal events
    # but in the specific case this could work well
    #    intermediate_vars = ['nRemainingTracksInEvent']

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

    # D0_SL = Particle('D0:semileptonic',
    #                  MVAConfiguration(variables=intermediate_vars,
    #                                   target='isSignalAcceptMissingNeutrino'),
    #                  PreCutConfiguration(userCut='',
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # D0_SL.addChannel(['K-', 'e+'])
    # D0_SL.addChannel(['K-', 'mu+'])
    # D0_SL.addChannel(['K-', 'pi0', 'e+'])
    # D0_SL.addChannel(['K-', 'pi0', 'mu+'])
    # D0_SL.addChannel(['K_S0', 'pi-', 'e+'])
    # D0_SL.addChannel(['K_S0', 'pi-', 'mu+'])

    # D0_KL = Particle('D0:KL',
    #                  MVAConfiguration(variables=intermediate_vars,
    #                                   target='isSignal'),
    #                  PreCutConfiguration(userCut='',
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # D0_KL.addChannel(['K_L0', 'pi0'])
    # D0_KL.addChannel(['K_L0', 'pi+', 'pi-'])
    # D0_KL.addChannel(['K_L0', 'pi+', 'pi-', 'pi0'])
    # D0_KL.addChannel(['K-', 'K+', 'K_L0'])

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

    # DP_SL = Particle('D+:semileptonic',
    #                  MVAConfiguration(variables=intermediate_vars,
    #                                   target='isSignalAcceptMissingNeutrino'),
    #                  PreCutConfiguration(userCut='',
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DP_SL.addChannel(['K_S0', 'e+'])
    # DP_SL.addChannel(['K_S0', 'mu+'])
    # DP_SL.addChannel(['K-', 'pi+', 'e+'])
    # DP_SL.addChannel(['K-', 'pi+', 'mu+'])

    # DP_KL = Particle('D+:KL',
    #                  MVAConfiguration(variables=intermediate_vars,
    #                                   target='isSignal'),
    #                  PreCutConfiguration(userCut='',
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DP_KL.addChannel(['K_L0', 'pi+'])
    # DP_KL.addChannel(['K_L0', 'pi+', 'pi0'])
    # DP_KL.addChannel(['K_L0', 'pi+', 'pi+', 'pi-'])
    # DP_KL.addChannel(['K+', 'K_L0', 'K_S0'])
    # DP_KL.addChannel(['K+', 'K_L0', 'K_L0'])

    Jpsi = Particle('J/psi',
                    MVAConfiguration(variables=intermediate_vars,
                                     target='isSignal'),
                    PreCutConfiguration(userCut='2.8 < M < 3.5',
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

    # DSP_SL = Particle('D*+:semileptonic',
    #                   MVAConfiguration(variables=intermediate_vars,
    #                                    target='isSignalAcceptMissingNeutrino'),
    #                   PreCutConfiguration(userCut='0 < Q < 0.3',
    #                                       bestCandidateVariable='abs(dQ)',
    #                                       bestCandidateCut=20),
    #                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DSP_SL.addChannel(['D0:semileptonic', 'pi+'])
    # DSP_SL.addChannel(['D+:semileptonic', 'pi0'])
    # DSP_SL.addChannel(['D+:semileptonic', 'gamma'])

    # DSP_KL = Particle('D*+:KL',
    #                   MVAConfiguration(variables=intermediate_vars,
    #                                    target='isSignal'),
    #                   PreCutConfiguration(userCut='0 < Q < 0.3',
    #                                       bestCandidateVariable='abs(dQ)',
    #                                       bestCandidateCut=20),
    #                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DSP_KL.addChannel(['D0:KL', 'pi+'])
    # DSP_KL.addChannel(['D+:KL', 'pi0'])
    # DSP_KL.addChannel(['D+:KL', 'gamma'])

    DS0 = Particle('D*0',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DS0.addChannel(['D0', 'pi0'])
    DS0.addChannel(['D0', 'gamma'])

    # DS0_SL = Particle('D*0:semileptonic',
    #                   MVAConfiguration(variables=intermediate_vars,
    #                                    target='isSignalAcceptMissingNeutrino'),
    #                   PreCutConfiguration(userCut='0 < Q < 0.3',
    #                                       bestCandidateVariable='abs(dQ)',
    #                                       bestCandidateCut=20),
    #                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DS0_SL.addChannel(['D0:semileptonic', 'pi0'])
    # DS0_SL.addChannel(['D0:semileptonic', 'gamma'])

    # DS0_KL = Particle('D*0:KL',
    #                   MVAConfiguration(variables=intermediate_vars,
    #                                    target='isSignal'),
    #                   PreCutConfiguration(userCut='0 < Q < 0.3',
    #                                       bestCandidateVariable='abs(dQ)',
    #                                       bestCandidateCut=20),
    #                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DS0_KL.addChannel(['D0:KL', 'pi0'])
    # DS0_KL.addChannel(['D0:KL', 'gamma'])

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

    # DS_KL = Particle('D_s+:KL',
    #                  MVAConfiguration(variables=intermediate_vars,
    #                                   target='isSignal'),
    #                  PreCutConfiguration(userCut='',
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DS_KL.addChannel(['K+', 'K_L0'])
    # DS_KL.addChannel(['K+', 'K_L0', 'pi+', 'pi-'])
    # DS_KL.addChannel(['K-', 'K_L0', 'pi+', 'pi+'])
    # DS_KL.addChannel(['K_L0', 'pi+'])
    # DS_KL.addChannel(['K_L0', 'pi+', 'pi0'])

    DSS = Particle('D_s*+',
                   MVAConfiguration(variables=intermediate_vars,
                                    target='isSignal'),
                   PreCutConfiguration(userCut='0.0 < Q < 0.3',
                                       bestCandidateVariable='abs(dQ)',
                                       bestCandidateCut=20),
                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DSS.addChannel(['D_s+', 'gamma'])
    DSS.addChannel(['D_s+', 'pi0'])

    # DSS_KL = Particle('D_s*+:KL',
    #                   MVAConfiguration(variables=intermediate_vars,
    #                                    target='isSignal'),
    #                   PreCutConfiguration(userCut='0.0 < Q < 0.3',
    #                                       bestCandidateVariable='abs(dQ)',
    #                                       bestCandidateCut=20),
    #                   PostCutConfiguration(bestCandidateCut=10, value=0.001))

    # DSS_KL.addChannel(['D_s+:KL', 'gamma'])
    # DSS_KL.addChannel(['D_s+:KL', 'pi0'])

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
    BP.addChannel(['J/psi', 'K+'])
    BP.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
    BP.addChannel(['J/psi', 'K+', 'pi0'])
    BP.addChannel(['J/psi', 'K_S0', 'pi+'])

    mva_BPlusSemileptonic = MVAConfiguration(
        variables=B_vars,
        target='isSignalAcceptMissingNeutrino')

    semileptonic_user_cut = ''
    if B_extra_cut is not None:
        semileptonic_user_cut += B_extra_cut

    BP_SL = Particle('B+:semileptonic',
                     MVAConfiguration(variables=B_vars,
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
    # BP_SL.addChannel(['anti-D0:semileptonic', 'pi+'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi0'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi0', 'pi0'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi+', 'pi-'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'D+'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'D+', 'K_S0'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'D+', 'K_S0'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'D*+', 'K_S0'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'D*+', 'K_S0'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'D0', 'K+'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'D0', 'K+'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'D*0', 'K+'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'D*0', 'K+'])
    # BP_SL.addChannel(['anti-D0', 'D+:semileptonic'])
    # BP_SL.addChannel(['anti-D0', 'D+:semileptonic', 'K_S0'])
    # BP_SL.addChannel(['anti-D*0', 'D+:semileptonic', 'K_S0'])
    # BP_SL.addChannel(['anti-D0', 'D*+:semileptonic', 'K_S0'])
    # BP_SL.addChannel(['anti-D*0', 'D*+:semileptonic', 'K_S0'])
    # BP_SL.addChannel(['anti-D0', 'D0:semileptonic', 'K+'])
    # BP_SL.addChannel(['anti-D*0', 'D0:semileptonic', 'K+'])
    # BP_SL.addChannel(['anti-D0', 'D*0:semileptonic', 'K+'])
    # BP_SL.addChannel(['anti-D*0', 'D*0:semileptonic', 'K+'])
    # BP_SL.addChannel(['D_s+', 'anti-D0:semileptonic'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi0'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi0', 'pi0'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi+', 'pi-'])
    # BP_SL.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
    # BP_SL.addChannel(['D_s*+', 'anti-D0:semileptonic'])
    # BP_SL.addChannel(['D_s+', 'anti-D*0:semileptonic'])
    # BP_SL.addChannel(['anti-D0:semileptonic', 'K+'])
    # BP_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+'])
    # BP_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi0'])

    # BP_KL = Particle('B+:KL',
    #                  MVAConfiguration(variables=B_vars,
    #                                   target='isSignal'),
    #                  PreCutConfiguration(userCut=semileptonic_user_cut,
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=20))
    # BP_KL.addChannel(['anti-D0:KL', 'pi+'])
    # BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi0'])
    # BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi0', 'pi0'])
    # BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi+', 'pi-'])
    # BP_KL.addChannel(['anti-D0:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
    # BP_KL.addChannel(['anti-D0:KL', 'D+'])
    # BP_KL.addChannel(['anti-D0:KL', 'D+', 'K_S0'])
    # BP_KL.addChannel(['anti-D*0:KL', 'D+', 'K_S0'])
    # BP_KL.addChannel(['anti-D0:KL', 'D*+', 'K_S0'])
    # BP_KL.addChannel(['anti-D*0:KL', 'D*+', 'K_S0'])
    # BP_KL.addChannel(['anti-D0:KL', 'D0', 'K+'])
    # BP_KL.addChannel(['anti-D*0:KL', 'D0', 'K+'])
    # BP_KL.addChannel(['anti-D0:KL', 'D*0', 'K+'])
    # BP_KL.addChannel(['anti-D*0:KL', 'D*0', 'K+'])
    # BP_KL.addChannel(['anti-D0', 'D+:KL'])
    # BP_KL.addChannel(['anti-D0', 'D+:KL', 'K_S0'])
    # BP_KL.addChannel(['anti-D*0', 'D+:KL', 'K_S0'])
    # BP_KL.addChannel(['anti-D0', 'D*+:KL', 'K_S0'])
    # BP_KL.addChannel(['anti-D*0', 'D*+:KL', 'K_S0'])
    # BP_KL.addChannel(['anti-D0', 'D0:KL', 'K+'])
    # BP_KL.addChannel(['anti-D*0', 'D0:KL', 'K+'])
    # BP_KL.addChannel(['anti-D0', 'D*0:KL', 'K+'])
    # BP_KL.addChannel(['anti-D*0', 'D*0:KL', 'K+'])
    # BP_KL.addChannel(['D_s+', 'anti-D0:KL'])
    # BP_KL.addChannel(['D_s+:KL', 'anti-D0'])
    # BP_KL.addChannel(['anti-D*0:KL', 'pi+'])
    # BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi0'])
    # BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi0', 'pi0'])
    # BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi+', 'pi-'])
    # BP_KL.addChannel(['anti-D*0:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
    # BP_KL.addChannel(['D_s*+', 'anti-D0:KL'])
    # BP_KL.addChannel(['D_s+', 'anti-D*0:KL'])
    # BP_KL.addChannel(['D_s*+:KL', 'anti-D0'])
    # BP_KL.addChannel(['D_s+:KL', 'anti-D*0'])
    # BP_KL.addChannel(['anti-D0:KL', 'K+'])
    # BP_KL.addChannel(['D-:KL', 'pi+', 'pi+'])
    # BP_KL.addChannel(['D-:KL', 'pi+', 'pi+', 'pi0'])
    # BP_KL.addChannel(['anti-D0', 'D+', 'K_L0'])
    # BP_KL.addChannel(['anti-D*0', 'D+', 'K_L0'])
    # BP_KL.addChannel(['anti-D0', 'D*+', 'K_L0'])
    # BP_KL.addChannel(['anti-D*0', 'D*+', 'K_L0'])
    # BP_KL.addChannel(['J/psi', 'K_L0', 'pi+'])

    B0 = Particle('B0',
                  MVAConfiguration(variables=B_vars,
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
    B0.addChannel(['J/psi', 'K_S0'])
    B0.addChannel(['J/psi', 'K+', 'pi-'])
    B0.addChannel(['J/psi', 'K_S0', 'pi+', 'pi-'])

    B0_SL = Particle('B0:semileptonic',
                     MVAConfiguration(variables=B_vars,
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
    # B0_SL.addChannel(['D-:semileptonic', 'pi+'])
    # B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi0'])
    # B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi0', 'pi0'])
    # B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi-'])
    # B0_SL.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
    # B0_SL.addChannel(['anti-D0:semileptonic', 'pi+', 'pi-'])
    # B0_SL.addChannel(['D-:semileptonic', 'D0', 'K+'])
    # B0_SL.addChannel(['D-:semileptonic', 'D*0', 'K+'])
    # B0_SL.addChannel(['D*-:semileptonic', 'D0', 'K+'])
    # B0_SL.addChannel(['D*-:semileptonic', 'D*0', 'K+'])
    # B0_SL.addChannel(['D-:semileptonic', 'D+', 'K_S0'])
    # B0_SL.addChannel(['D*-:semileptonic', 'D+', 'K_S0'])
    # B0_SL.addChannel(['D-:semileptonic', 'D*+', 'K_S0'])
    # B0_SL.addChannel(['D*-:semileptonic', 'D*+', 'K_S0'])
    # B0_SL.addChannel(['D-', 'D0:semileptonic', 'K+'])
    # B0_SL.addChannel(['D-', 'D*0:semileptonic', 'K+'])
    # B0_SL.addChannel(['D*-', 'D0:semileptonic', 'K+'])
    # B0_SL.addChannel(['D*-', 'D*0:semileptonic', 'K+'])
    # B0_SL.addChannel(['D-', 'D+:semileptonic', 'K_S0'])
    # B0_SL.addChannel(['D*-', 'D+:semileptonic', 'K_S0'])
    # B0_SL.addChannel(['D-', 'D*+:semileptonic', 'K_S0'])
    # B0_SL.addChannel(['D*-', 'D*+:semileptonic', 'K_S0'])
    # B0_SL.addChannel(['D_s+', 'D-:semileptonic'])
    # B0_SL.addChannel(['D*-:semileptonic', 'pi+'])
    # B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi0'])
    # B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi0', 'pi0'])
    # B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi+', 'pi-'])
    # B0_SL.addChannel(['D*-:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
    # B0_SL.addChannel(['D_s*+', 'D-:semileptonic'])
    # B0_SL.addChannel(['D_s+', 'D*-:semileptonic'])
    # B0_SL.addChannel(['D_s*+', 'D*-:semileptonic'])

    # B0_KL = Particle('B0:KL',
    #                  MVAConfiguration(variables=B_vars,
    #                                   target='isSignal'),
    #                  PreCutConfiguration(userCut=semileptonic_user_cut,
    #                                      bestCandidateMode='highest',
    #                                      bestCandidateVariable='daughterProductOf(extraInfo(SignalProbability))',
    #                                      bestCandidateCut=20),
    #                  PostCutConfiguration(bestCandidateCut=20))
    # B0_KL.addChannel(['D-:KL', 'pi+'])
    # B0_KL.addChannel(['D-:KL', 'pi+', 'pi0'])
    # B0_KL.addChannel(['D-:KL', 'pi+', 'pi0', 'pi0'])
    # B0_KL.addChannel(['D-:KL', 'pi+', 'pi+', 'pi-'])
    # B0_KL.addChannel(['D-:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
    # B0_KL.addChannel(['anti-D0:KL', 'pi+', 'pi-'])
    # B0_KL.addChannel(['D-:KL', 'D0', 'K+'])
    # B0_KL.addChannel(['D-:KL', 'D*0', 'K+'])
    # B0_KL.addChannel(['D*-:KL', 'D0', 'K+'])
    # B0_KL.addChannel(['D*-:KL', 'D*0', 'K+'])
    # B0_KL.addChannel(['D-:KL', 'D+', 'K_S0'])
    # B0_KL.addChannel(['D*-:KL', 'D+', 'K_S0'])
    # B0_KL.addChannel(['D-:KL', 'D*+', 'K_S0'])
    # B0_KL.addChannel(['D*-:KL', 'D*+', 'K_S0'])
    # B0_KL.addChannel(['D-', 'D0:KL', 'K+'])
    # B0_KL.addChannel(['D-', 'D*0:KL', 'K+'])
    # B0_KL.addChannel(['D*-', 'D0:KL', 'K+'])
    # B0_KL.addChannel(['D*-', 'D*0:KL', 'K+'])
    # B0_KL.addChannel(['D-', 'D+:KL', 'K_S0'])
    # B0_KL.addChannel(['D*-', 'D+:KL', 'K_S0'])
    # B0_KL.addChannel(['D-', 'D*+:KL', 'K_S0'])
    # B0_KL.addChannel(['D*-', 'D*+:KL', 'K_S0'])
    # B0_KL.addChannel(['D_s+', 'D-:KL'])
    # B0_KL.addChannel(['D_s+:KL', 'D-'])
    # B0_KL.addChannel(['D*-:KL', 'pi+'])
    # B0_KL.addChannel(['D*-:KL', 'pi+', 'pi0'])
    # B0_KL.addChannel(['D*-:KL', 'pi+', 'pi0', 'pi0'])
    # B0_KL.addChannel(['D*-:KL', 'pi+', 'pi+', 'pi-'])
    # B0_KL.addChannel(['D*-:KL', 'pi+', 'pi+', 'pi-', 'pi0'])
    # B0_KL.addChannel(['D_s*+', 'D-:KL'])
    # B0_KL.addChannel(['D_s+', 'D*-:KL'])
    # B0_KL.addChannel(['D_s*+', 'D*-:KL'])
    # B0_KL.addChannel(['D_s*+:KL', 'D-'])
    # B0_KL.addChannel(['D_s+:KL', 'D*-'])
    # B0_KL.addChannel(['D_s*+:KL', 'D*-'])
    # B0_KL.addChannel(['D-', 'D+', 'K_L0'])
    # B0_KL.addChannel(['D*-', 'D+', 'K_L0'])
    # B0_KL.addChannel(['D-', 'D*+', 'K_L0'])
    # B0_KL.addChannel(['D*-', 'D*+', 'K_L0'])
    # B0_KL.addChannel(['J/psi', 'K_L0'])
    # B0_KL.addChannel(['J/psi', 'K_L0', 'pi+', 'pi-'])

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

    if hadronic:
        if neutralB:
            particles.append(B0)
        if chargedB:
            particles.append(BP)

    # if KLong:
    #     particles.append(KL0)
    #     particles.append(D0_KL)
    #     particles.append(DP_KL)
    #     particles.append(DS_KL)
    #     particles.append(DS0_KL)
    #     particles.append(DSP_KL)
    #     particles.append(DSS_KL)
    #     if neutralB:
    #         particles.append(B0_KL)
    #     if chargedB:
    #         particles.append(BP_KL)

    if semileptonic:
        # particles.append(D0_SL)
        # particles.append(DP_SL)
        # particles.append(DS0_SL)
        # particles.append(DSP_SL)
        if neutralB:
            particles.append(B0_SL)
        if chargedB:
            particles.append(BP_SL)

    return particles
