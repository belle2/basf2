#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration, UserCutConfiguration


def get_default_channels(BlevelExtraCut='', neutralB=True, chargedB=True, semileptonicB=True):
    """
    returns list of Particle objects with all default channels for running
    FEI on Upsilon(4S). For a training with analysis-specific signal selection,
    adding a cut on nRemainingTracksInRestOfEvent is recommended.
    @param neutralB wether to include B0
    @param chargedB wether to include B+
    @param semileptonicB wether to include semileptonic B decays
    """

    if BlevelExtraCut != '':
        hadronicUserCut = UserCutConfiguration('Mbc > 5.2 and abs(deltaE) < 0.5 and [' + BlevelExtraCut + ']')
        semileptonicUserCut = UserCutConfiguration(BlevelExtraCut)
    else:
        hadronicUserCut = UserCutConfiguration('Mbc > 5.2 and abs(deltaE) < 0.5')
        semileptonicUserCut = UserCutConfiguration()

    postCut = PostCutConfiguration(
        value=0.1
    )

    postCutSoft = PostCutConfiguration(
        value=0.01
    )

    # variables for D mesons and J/Psi
    intermediate_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
                         'chiProb', 'daughter({}, chiProb)',
                         'useRestFrame(daughter({}, p))',
                         'useRestFrame(daughter({}, distance))',
                         'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
                         'daughterInvariantMass({},{})', 'daughterInvariantMass({},{},{})', 'daughterInvariantMass({},{},{},{})',
                         'daughterInvariantMass({},{},{},{},{})', 'Q', 'extraInfo(decayModeID)']

    # note: these should not be correlated to Mbc (weak correlation of deltaE is OK)
    B_vars = ['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
              'chiProb', 'daughter({}, chiProb)',
              'useRestFrame(daughter({}, p))',
              'useRestFrame(daughter({}, distance))',
              'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
              'dr', 'dz', 'dx', 'dy', 'distance', 'significanceOfDistance', 'deltaE', 'extraInfo(decayModeID)']

    particles = []

# ################## Charged FSP ###############################
    mva_chargedFSP = MVAConfiguration(
        variables=['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                   'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                   'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                   'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH',
                   'p', 'pt', 'pz',
                   'dr', 'dz', 'chiProb'],
        target='isPrimarySignal',
    )

    pre_chargedFSP = PreCutConfiguration(
        variable='dr',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(100, 0.0, 10.0),
        efficiency=0.99,
        purity=0.001,
    )

    user_chargedFSP = UserCutConfiguration('[dr < 2] and [dz < 4]')

    particles.append(Particle('pi+', mva_chargedFSP, pre_chargedFSP, user_chargedFSP,
                              postCutConfig=postCut).addChannel(['pi+:FSP']))
    particles.append(Particle('e+', mva_chargedFSP, pre_chargedFSP, user_chargedFSP,
                              postCutConfig=postCut).addChannel(['e+:FSP']))
    particles.append(Particle('mu+', mva_chargedFSP, pre_chargedFSP, user_chargedFSP,
                              postCutConfig=postCut).addChannel(['mu+:FSP']))
    particles.append(Particle('K+', mva_chargedFSP, pre_chargedFSP, user_chargedFSP,
                              postCutConfig=postCut).addChannel(['K+:FSP']))

# ################# GAMMA ############################

    mva_gamma = MVAConfiguration(
        variables=['clusterReg', 'goodGamma', 'goodGammaUnCal',
                   'clusterNHits', 'clusterTiming', 'clusterE9E25',
                   'pt', 'E', 'pz'],
        target='isPrimarySignal',
    )

    pre_gamma = PreCutConfiguration(
        variable='goodGamma',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(100, 0.0, 1.0),
        efficiency=0.99,
        purity=0.001,
    )

    user_gamma = UserCutConfiguration('E > 0.1')

    particles.append(Particle('gamma', mva_gamma, pre_gamma, user_gamma, postCutConfig=postCut).addChannel(['gamma:FSP']))

# ################# PI0 ###############################
    mva_pi0 = MVAConfiguration(
        variables=['M', 'daughter({},extraInfo(SignalProbability))', 'daughterAngle(0,1)',
                   'pt', 'pz', 'E'],
        target='isSignal',
    )

    pre_pi0 = PreCutConfiguration(
        variable='M',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 0.08, 0.18),
        efficiency=0.95,
        purity=0.0001,
    )

    particles.append(
        Particle('pi0', mva_pi0, pre_pi0, postCutConfig=postCut).addChannel(['gamma', 'gamma']))

# ################## KS0 ###############################
    mva_KS0 = MVAConfiguration(
        variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb',
                   'M', 'useCMSFrame(E)', 'daughterAngle(0,1)', 'daughter({},extraInfo(SignalProbability))',
                   'useRestFrame(daughter({}, p))',
                   'cosAngleBetweenMomentumAndVertexVector',
                   'daughter({}, dz)', 'daughter({}, dr)'],
        target='isSignal',
    )

    mva_KS0_FSP = MVAConfiguration(
        variables=['dr', 'dz', 'distance', 'significanceOfDistance', 'chiProb',
                   'M', 'useCMSFrame(E)', 'daughterAngle(0,1)',
                   'useRestFrame(daughter({}, p))',
                   'cosAngleBetweenMomentumAndVertexVector',
                   'daughter({}, dz)', 'daughter({}, dr)'],
        target='isSignal',
    )

    pre_KS0 = PreCutConfiguration(
        variable='M',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 0.4, 0.6),
        efficiency=0.95,
        purity=0.0001,
    )

    p = Particle('K_S0', mva_KS0, pre_KS0, postCutConfig=postCut)
    p.addChannel(['pi+', 'pi-'])
    p.addChannel(['pi0', 'pi0'])
    p.addChannel(['K_S0:FSP'], mvaConfig=mva_KS0_FSP)
    particles.append(p)

# ####################### D0 #########################
    mva_D0 = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_D0 = PreCutConfiguration(
        variable='M',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 1.70, 1.95),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('D0', mva_D0, pre_D0, postCutConfig=postCutSoft)
    p.addChannel(['K-', 'pi+'])
    p.addChannel(['K-', 'pi+', 'pi0'])
    p.addChannel(['K-', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['K-', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['pi-', 'pi+'])
    p.addChannel(['pi-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['pi-', 'pi+', 'pi0'])
    p.addChannel(['pi-', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['K_S0', 'pi0'])
    p.addChannel(['K_S0', 'pi+', 'pi-'])
    p.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['K-', 'K+'])
    p.addChannel(['K-', 'K+', 'pi0'])
    p.addChannel(['K-', 'K+', 'K_S0'])
    particles.append(p)

# ####################### D+ #########################

    mva_DPlus = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_DPlus = PreCutConfiguration(
        variable='M',
        binning=(500, 1.7, 1.95),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('D+', mva_DPlus, pre_DPlus, postCutConfig=postCutSoft)
    p.addChannel(['K-', 'pi+', 'pi+'])
    p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
    p.addChannel(['K-', 'K+', 'pi+'])
    p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
    p.addChannel(['pi+', 'pi0'])
    p.addChannel(['pi+', 'pi+', 'pi-'])
    p.addChannel(['pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['K_S0', 'pi+'])
    p.addChannel(['K_S0', 'pi+', 'pi0'])
    p.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['K+', 'K_S0', 'K_S0'])

    particles.append(p)

# ####################### D*+ #########################

    mva_DStarPlus = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_DStarPlus = PreCutConfiguration(
        variable='Q',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 0, 0.3),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('D*+', mva_DStarPlus, pre_DStarPlus, postCutConfig=postCutSoft)
    p.addChannel(['D0', 'pi+'])
    p.addChannel(['D+', 'pi0'])
    p.addChannel(['D+', 'gamma'])
    particles.append(p)

# ####################### D*0 #########################
    mva_DStar0 = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_DStar0 = PreCutConfiguration(
        variable='Q',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 0, 0.3),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('D*0', mva_DStar0, pre_DStar0, postCutConfig=postCutSoft)
    p.addChannel(['D0', 'pi0'])
    p.addChannel(['D0', 'gamma'])
    particles.append(p)

# ####################### D_s+ #########################

    mva_DS = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_DS = PreCutConfiguration(
        variable='M',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 1.68, 2.1),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('D_s+', mva_DS, pre_DS, postCutConfig=postCutSoft)
    p.addChannel(['K+', 'K_S0'])
    p.addChannel(['K+', 'pi+', 'pi-'])
    p.addChannel(['K+', 'K-', 'pi+'])
    p.addChannel(['K+', 'K-', 'pi+', 'pi0'])
    p.addChannel(['K+', 'K_S0', 'pi+', 'pi-'])
    p.addChannel(['K-', 'K_S0', 'pi+', 'pi+'])
    p.addChannel(['K+', 'K-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['pi+', 'pi+', 'pi-'])
    p.addChannel(['K_S0', 'pi+'])
    p.addChannel(['K_S0', 'pi+', 'pi0'])
    particles.append(p)

# ####################### DStar_s+ #########################

    mva_DStarS = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_DStarS = PreCutConfiguration(
        variable='Q',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 0, 0.3),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('D_s*+', mva_DStarS, pre_DStarS, postCutConfig=postCutSoft)
    p.addChannel(['D_s+', 'gamma'])
    p.addChannel(['D_s+', 'pi0'])
    particles.append(p)

# ####################### J/psi #########################

    mva_J = MVAConfiguration(
        variables=intermediate_vars,
        target='isSignal',
    )

    pre_J = PreCutConfiguration(
        variable='M',
        # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
        binning=(500, 2.8, 3.5),
        efficiency=0.95,
        purity=0.001,
    )

    p = Particle('J/psi', mva_J, pre_J)
    p.addChannel(['e+', 'e-'])
    p.addChannel(['mu+', 'mu-'])
    particles.append(p)

# ################## B+ ##############################
    mva_BPlus = MVAConfiguration(
        variables=B_vars,
        target='isSignal',
    )

    pre_BPlus = PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
    )

    p = Particle('B+', mva_BPlus, pre_BPlus, hadronicUserCut)
    p.addChannel(['anti-D0', 'pi+'])
    p.addChannel(['anti-D0', 'pi+', 'pi0'])
    p.addChannel(['anti-D0', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['anti-D0', 'D+'])
    p.addChannel(['anti-D0', 'D+', 'K_S0'])
    p.addChannel(['anti-D*0', 'D+', 'K_S0'])
    p.addChannel(['anti-D0', 'D*+', 'K_S0'])
    p.addChannel(['anti-D*0', 'D*+', 'K_S0'])
    p.addChannel(['anti-D0', 'D0', 'K+'])
    p.addChannel(['anti-D*0', 'D0', 'K+'])
    p.addChannel(['anti-D0', 'D*0', 'K+'])
    p.addChannel(['anti-D*0', 'D*0', 'K+'])
    p.addChannel(['D_s+', 'anti-D0'])
    p.addChannel(['anti-D*0', 'pi+'])
    p.addChannel(['anti-D*0', 'pi+', 'pi0'])
    p.addChannel(['anti-D*0', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['D_s*+', 'anti-D0'])
    p.addChannel(['D_s+', 'anti-D*0'])
    p.addChannel(['anti-D0', 'K+'])
    p.addChannel(['D-', 'pi+', 'pi+'])
    p.addChannel(['D-', 'pi+', 'pi+', 'pi0'])
    p.addChannel(['J/psi', 'K+'])
    p.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
    p.addChannel(['J/psi', 'K+', 'pi0'])
    p.addChannel(['J/psi', 'K_S0', 'pi+'])
    if chargedB:
        particles.append(p)

    mva_BPlusSemileptonic = MVAConfiguration(
        variables=B_vars,
        target='isSignalAcceptMissingNeutrino',
    )

    pre_BPlusSemileptonic = PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
    )

    p = Particle(
        'B+:semileptonic', mva_BPlusSemileptonic, pre_BPlusSemileptonic, semileptonicUserCut)
    p.addChannel(['anti-D0', 'e+'])
    p.addChannel(['anti-D0', 'mu+'])
    p.addChannel(['anti-D*0', 'e+'])
    p.addChannel(['anti-D*0', 'mu+'])
    p.addChannel(['D-', 'pi+', 'e+'])
    p.addChannel(['D-', 'pi+', 'mu+'])
    p.addChannel(['D*-', 'pi+', 'e+'])
    p.addChannel(['D*-', 'pi+', 'mu+'])
    if chargedB and semileptonicB:
        particles.append(p)

# #################################  B0 #################################
    mva_B0 = MVAConfiguration(
        variables=B_vars,
        target='isSignal',
    )

    pre_B0 = PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
    )

    p = Particle('B0', mva_B0, pre_B0, hadronicUserCut)
    p.addChannel(['D-', 'pi+'])
    p.addChannel(['D-', 'pi+', 'pi0'])
    p.addChannel(['D-', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['D-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['D-', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['anti-D0', 'pi+', 'pi-'])
    p.addChannel(['D-', 'D0', 'K+'])
    p.addChannel(['D-', 'D*0', 'K+'])
    p.addChannel(['D*-', 'D0', 'K+'])
    p.addChannel(['D*-', 'D*0', 'K+'])
    p.addChannel(['D-', 'D+', 'K_S0'])
    p.addChannel(['D*-', 'D+', 'K_S0'])
    p.addChannel(['D-', 'D*+', 'K_S0'])
    p.addChannel(['D*-', 'D*+', 'K_S0'])
    p.addChannel(['D_s+', 'D-'])
    p.addChannel(['D*-', 'pi+'])
    p.addChannel(['D*-', 'pi+', 'pi0'])
    p.addChannel(['D*-', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['D_s*+', 'D-'])
    p.addChannel(['D_s+', 'D*-'])
    p.addChannel(['D_s*+', 'D*-'])
    p.addChannel(['J/psi', 'K_S0'])
    p.addChannel(['J/psi', 'K+', 'pi-'])
    p.addChannel(['J/psi', 'K_S0', 'pi+', 'pi-'])
    if neutralB:
        particles.append(p)

    mva_B0Semileptonic = MVAConfiguration(
        variables=B_vars,
        target='isSignalAcceptMissingNeutrino',
    )

    pre_B0Semileptonic = PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
    )

    p = Particle('B0:semileptonic', mva_B0Semileptonic, pre_B0Semileptonic, semileptonicUserCut)
    p.addChannel(['D-', 'e+'])
    p.addChannel(['D-', 'mu+'])
    p.addChannel(['D*-', 'e+'])
    p.addChannel(['D*-', 'mu+'])
    p.addChannel(['anti-D0', 'pi-', 'e+'])
    p.addChannel(['anti-D0', 'pi-', 'mu+'])
    p.addChannel(['anti-D*0', 'pi-', 'e+'])
    p.addChannel(['anti-D*0', 'pi-', 'mu+'])
    if neutralB and semileptonicB:
        particles.append(p)

    return particles
