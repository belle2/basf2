#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


def get_default_channels(BlevelExtraCut='', semileptonic=True, KLong=True, convertedFromBelle=False):
    """
    returns list of Particle objects with all default channels for running
    FEI on Upsilon(4S). For a training with analysis-specific signal selection,
    adding a cut on nRemainingTracksInRestOfEvent is recommended.
    @param semileptonicB wether to include semileptonic B decays
    """

    if convertedFromBelle:
        # Using Belle specific Variables for e-ID, mu-ID and K-ID
        # atcPIDBelle(3,2) is used as K-ID
        # atcPIDBelle(4,2) and atcPIDBelle(4,3) are used as pr-ID
        # TODO Check how the respective *_dEdx, *_TOP, *_ARICH and *_ECL should be treated
        chargedVariables = ['eIDBelle', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                            'atcPIDBelle(3,2)', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                            'atcPIDBelle(4,2)', 'atcPIDBelle(4,3)', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                            'muIDBelle', 'muid_dEdx', 'muid_TOP', 'muid_ARICH',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']
    else:
        chargedVariables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                            'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                            'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                            'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH',
                            'p', 'pt', 'pz', 'dr', 'dz', 'chiProb', 'extraInfo(preCut_rank)']

    pion = Particle('pi+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='piid',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='Kid',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    electron = Particle('e+',
                        MVAConfiguration(variables=chargedVariables,
                                         target='isPrimarySignal'),
                        PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                            bestCandidateMode='highest',
                                            bestCandidateVariable='eid',
                                            bestCandidateCut=10),
                        PostCutConfiguration(bestCandidateCut=5, value=0.01))
    electron.addChannel(['e+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='muid',
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=5, value=0.01))
    muon.addChannel(['mu+:FSP'])

    gamma = Particle('gamma',
                     MVAConfiguration(variables=['clusterReg', 'clusterNHits', 'clusterTiming', 'extraInfo(preCut_rank)',
                                                 'clusterE9E25', 'pt', 'E', 'pz', 'goodGamma'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut='',
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

    KS0 = Particle('K_S0',
                   MVAConfiguration(variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb', 'M', 'abs(dM)',
                                               'useCMSFrame(E)', 'daughterAngle(0,1)', 'daughter({},extraInfo(SignalProbability))',
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

    KL0 = Particle('K_L0',
                   MVAConfiguration(variables=['E', 'klmClusterTiming'],
                                    target='isSignal'),
                   PreCutConfiguration(userCut='',
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

    DS0_SL = Particle('D*0:semileptonic',
                      MVAConfiguration(variables=intermediate_vars,
                                       target='isSignalAcceptMissingNeutrino'),
                      PreCutConfiguration(userCut='0 < Q < 0.3',
                                          bestCandidateVariable='abs(dQ)',
                                          bestCandidateCut=20),
                      PostCutConfiguration(bestCandidateCut=10, value=0.001))

    DS0_SL.addChannel(['D0:semileptonic', 'pi0'])
    DS0_SL.addChannel(['D0:semileptonic', 'gamma'])

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

    hadronicUserCut = 'Mbc > 5.2 and abs(deltaE) < 0.5'
    if BlevelExtraCut != '':
        hadronicUserCut += ' and [' + BlevelExtraCut + ']'

    BP = Particle('B+',
                  MVAConfiguration(variables=B_vars,
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronicUserCut,
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
        target='isSignalAcceptMissingNeutrino',
    )

    BP_SL = Particle('B+:semileptonic',
                     MVAConfiguration(variables=B_vars,
                                      target='isSignalAcceptMissingNeutrino'),
                     PreCutConfiguration(userCut='',
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

    BP_KL = Particle('B+:KL',
                     MVAConfiguration(variables=B_vars,
                                      target='isSignal'),
                     PreCutConfiguration(userCut='',
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
                                   target='isSignal'),
                  PreCutConfiguration(userCut=hadronicUserCut,
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
                     PreCutConfiguration(userCut='',
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

    B0_KL = Particle('B0:KL',
                     MVAConfiguration(variables=B_vars,
                                      target='isSignal'),
                     PreCutConfiguration(userCut='',
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

    if KLong:
        particles.append(KL0)
        particles.append(D0_KL)
        particles.append(DP_KL)
        particles.append(DS_KL)
        particles.append(DS0_KL)
        particles.append(DSP_KL)
        particles.append(DSS_KL)
        particles.append(B0_KL)
        particles.append(BP_KL)

    if semileptonic:
        particles.append(D0_SL)
        particles.append(DP_SL)
        particles.append(DS0_SL)
        particles.append(DSP_SL)
        particles.append(B0_SL)
        particles.append(BP_SL)

    return particles


def get_unittest_channels():
    chargedVariables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL', 'extraInfo(preCut_rank)',
                        'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                        'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                        'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH',
                        'p', 'pt', 'pz', 'dr', 'dz', 'chiProb']

    pion = Particle('pi+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='piid',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    pion.addChannel(['pi+:FSP'])

    kaon = Particle('K+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateMode='highest',
                                        bestCandidateVariable='Kid',
                                        bestCandidateCut=20),
                    PostCutConfiguration(bestCandidateCut=10, value=0.01))
    kaon.addChannel(['K+:FSP'])

    muon = Particle('mu+',
                    MVAConfiguration(variables=chargedVariables,
                                     target='isPrimarySignal'),
                    PreCutConfiguration(userCut='[dr < 2] and [abs(dz) < 4]',
                                        bestCandidateVariable='muid',
                                        bestCandidateMode='highest',
                                        bestCandidateCut=10),
                    PostCutConfiguration(bestCandidateCut=5, value=0.01))
    muon.addChannel(['mu+:FSP'])

    gamma = Particle('gamma',
                     MVAConfiguration(variables=['clusterReg', 'clusterNHits', 'clusterTiming', 'clusterE9E25',
                                                 'pt', 'E', 'pz', 'goodGamma', 'extraInfo(preCut_rank)'],
                                      target='isPrimarySignal'),
                     PreCutConfiguration(userCut='',
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
