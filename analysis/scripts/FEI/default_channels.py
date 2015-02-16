from FEI import Particle


def get_default_channnels(BlevelExtraCut='', neutralB=True, chargedB=True, semileptonicB=True):
    """
    returns list of Particle objects with all default channels for running
    FEI on Upsilon(4S). For a training with analysis-specific signal selection,
    adding a cut on nRemainingTracksInRestOfEvent is recommended.
    @param neutralB wether to include B0
    @param chargedB wether to include B+
    @param semileptonicB wether to include semileptonic B decays
    """
    hadronicBuserCut = 'Mbc > 5.2 and abs(deltaE) < 0.5'
    if BlevelExtraCut != '':
        hadronicBuserCut += ' and [' + BlevelExtraCut + ']'
    mvaDefaults = {
        'name': 'FastBDT',
        'type': 'Plugin',
        'config': '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
    }

    pid_variables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                     'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                     'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                     'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH']
    kinematic_variables = ['p', 'pt', 'p_CMS', 'pt_CMS', 'M',
                           'E', 'px', 'py', 'pz', 'px_CMS', 'py_CMS', 'pz_CMS']
    vertex_variables = ['dr', 'dz', 'dx', 'dy', 'chiProb',
                        'cosAngleBetweenMomentumAndVertexVector', 'significanceOfDistance', 'distance']

    B_variables = ['dr', 'dz', 'dx', 'dy', 'distance', 'chiProb', 'significanceOfDistance',
                   'deltaE', 'cosAngleBetweenMomentumAndVertexVector',
                   'daughter({}, dz)', 'daughter({}, distance)', 'decayAngle({})', 'daughterAngle({},{})',
                   'daughterInvariantMass({},{})', 'daughterInvariantMass({},{}, {})',
                   'daughter({},extraInfo(SignalProbability))']

    postCut = Particle.PostCutConfiguration(
        value=0.1
    )

    postCutSoft = Particle.PostCutConfiguration(
        value=0.01
    )

    particles = []

# ################## Charged FSP ###############################
    mva_chargedFSP = Particle.MVAConfiguration(
        variables=pid_variables + kinematic_variables + ['chiProb'],
        target='isSignal',
        **mvaDefaults
    )

    particles.append(Particle('pi+', mva_chargedFSP, postCutConfig=postCut))
    particles.append(Particle('e+', mva_chargedFSP, postCutConfig=postCut))
    particles.append(Particle('mu+', mva_chargedFSP, postCutConfig=postCut))
    particles.append(Particle('K+', mva_chargedFSP, postCutConfig=postCut))

# ################# GAMMA ############################

    mva_gamma = Particle.MVAConfiguration(
        variables=kinematic_variables +
        ['clusterReg', 'goodGamma', 'goodGammaUnCal',
            'clusterNHits', 'clusterTrackMatch', 'clusterE9E25'],
        target='isSignal',
        **mvaDefaults
    )

    particles.append(Particle('gamma', mva_gamma, postCutConfig=postCut))

# ################# PI0 ###############################
    mva_pi0 = Particle.MVAConfiguration(
        variables=kinematic_variables + ['daughterAngle(0,1)'],
        target='isSignal',
        **mvaDefaults
    )

    pre_pi0 = Particle.PreCutConfiguration(
        variable='M',
        binning=(500, 0.08, 0.18),
        efficiency=0.95,
        purity=0.0001,
        userCut=''
    )

    particles.append(
        Particle('pi0', mva_pi0, pre_pi0, postCut).addChannel(['gamma', 'gamma']))

# ################## KS0 ###############################
    mva_KS0 = Particle.MVAConfiguration(
        variables=kinematic_variables +
        vertex_variables + ['daughterAngle(0,1)'],
        target='isSignal',
        **mvaDefaults
    )

    mva_KS0_pi0pi0 = Particle.MVAConfiguration(
        variables=['p', 'pt', 'M', 'E', 'daughterAngle(0,1)'],
        target='isSignal',
        **mvaDefaults
    )

    pre_KS0 = Particle.PreCutConfiguration(
        variable='M',
        binning=(500, 0.3, 0.7),
        efficiency=0.95,
        purity=0.0001,
        userCut=''
    )

    p = Particle('K_S0', mva_KS0, pre_KS0, postCut)
    p.addChannel(['pi+', 'pi-'])
    p.addChannel(['pi0', 'pi0'], mva_KS0_pi0pi0)
    particles.append(p)

# ####################### D0 #########################
    mva_D0 = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
        target='isSignal',
        **mvaDefaults
    )

    mva_D0_withoutVertex = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_D0 = Particle.PreCutConfiguration(
        variable='M',
        binning=(500, 1.5, 2.0),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('D0', mva_D0, pre_D0, postCutSoft)
    p.addChannel(['K-', 'pi+'])
    p.addChannel(['K-', 'pi+', 'pi0'])
    p.addChannel(['K-', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['K-', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['pi-', 'pi+'])
    p.addChannel(['pi-', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['pi-', 'pi+', 'pi0'])
    p.addChannel(['pi-', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['K_S0', 'pi0'], mva_D0_withoutVertex)
    p.addChannel(['K_S0', 'pi+', 'pi-'])
    p.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['K-', 'K+'])
    p.addChannel(['K-', 'K+', 'pi0'])
    p.addChannel(['K-', 'K+', 'K_S0'])
    particles.append(p)

# ####################### D+ #########################

    mva_DPlus = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
        target='isSignal',
        **mvaDefaults
    )

    mva_DPlus_withoutVertex = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_DPlus = Particle.PreCutConfiguration(
        variable='M',
        binning=(500, 1.5, 2.0),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('D+', mva_DPlus, pre_DPlus, postCutSoft)
    p.addChannel(['K-', 'pi+', 'pi+'])
    p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
    p.addChannel(['K-', 'K+', 'pi+'])
    p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
    p.addChannel(['pi+', 'pi0'], mva_DPlus_withoutVertex)
    p.addChannel(['pi+', 'pi+', 'pi-'])
    p.addChannel(['pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['K_S0', 'pi+'])
    p.addChannel(['K_S0', 'pi+', 'pi0'])
    p.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['K+', 'K_S0', 'K_S0'])

    particles.append(p)

# ####################### D*+ #########################

    mva_DStarPlus = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
        target='isSignal',
        **mvaDefaults
    )

    mva_DStarPlus_withoutVertex = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_DStarPlus = Particle.PreCutConfiguration(
        variable='Q',
        binning=(500, 0, 1),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('D*+', mva_DStarPlus, pre_DStarPlus, postCutSoft)
    p.addChannel(['D0', 'pi+'])
    p.addChannel(['D+', 'pi0'], mva_DStarPlus_withoutVertex)
    p.addChannel(['D+', 'gamma'], mva_DStarPlus_withoutVertex)
    particles.append(p)

# ####################### D*0 #########################
    mva_DStar0 = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_DStar0 = Particle.PreCutConfiguration(
        variable='Q',
        binning=(500, 0, 1),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('D*0', mva_DStar0, pre_DStar0, postCutSoft)
    p.addChannel(['D0', 'pi0'])
    p.addChannel(['D0', 'gamma'])
    particles.append(p)

# ####################### D_s+ #########################

    mva_DS = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_DS = Particle.PreCutConfiguration(
        variable='M',
        binning=(500, 1.6, 2.2),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('D_s+', mva_DS, pre_DS, postCutSoft)
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

    mva_DStarS = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_DStarS = Particle.PreCutConfiguration(
        variable='Q',
        binning=(500, 0, 1),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('D_s*+', mva_DStarS, pre_DStarS, postCutSoft)
    p.addChannel(['D_s+', 'gamma'])
    p.addChannel(['D_s+', 'pi0'])
    particles.append(p)

# ####################### J/psi #########################

    mva_J = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_J = Particle.PreCutConfiguration(
        variable='M',
        binning=(500, 2.5, 3.5),
        efficiency=0.95,
        purity=0.001,
        userCut=''
    )

    p = Particle('J/psi', mva_J, pre_J)
    p.addChannel(['e+', 'e-'])
    p.addChannel(['mu+', 'mu-'])
    particles.append(p)

# ################## B+ ##############################
    mva_BPlus = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
        target='isSignal',
        **mvaDefaults
    )

    mva_BPlus_low_stat = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_BPlus = Particle.PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
        userCut=hadronicBuserCut
    )

    p = Particle('B+', mva_BPlus, pre_BPlus)
    p.addChannel(['anti-D0', 'pi+'])
    p.addChannel(['anti-D0', 'pi+', 'pi0'])
    p.addChannel(['anti-D0', 'pi+', 'pi0', 'pi0'])
    p.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-'])
    p.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-', 'pi0'])
    p.addChannel(['anti-D0', 'D+'])
# p.addChannel(['anti-D0', 'D+', 'K_S0'])
# p.addChannel(['anti-D*0', 'D+', 'K_S0'])
# p.addChannel(['anti-D0', 'D*+', 'K_S0'])
# p.addChannel(['anti-D*0', 'D*+', 'K_S0'])
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
    p.addChannel(['J/psi', 'K+'], mva_BPlus_low_stat)
    p.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
    p.addChannel(['J/psi', 'K+', 'pi0'])
    p.addChannel(['J/psi', 'K_S0', 'pi+'])
    if chargedB:
        particles.append(p)

    mva_BPlusSemileptonic = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
        target='isSignalAcceptMissingNeutrino',
        **mvaDefaults
    )

    pre_BPlusSemileptonic = Particle.PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
        userCut=BlevelExtraCut
    )

    p = Particle(
        'B+:semileptonic', mva_BPlusSemileptonic, pre_BPlusSemileptonic)
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
    mva_B0 = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
        target='isSignal',
        **mvaDefaults
    )

    pre_B0 = Particle.PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
        userCut=hadronicBuserCut
    )

    p = Particle('B0', mva_B0, pre_B0)
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
# p.addChannel(['D-', 'D+', 'K_S0'])
# p.addChannel(['D*-', 'D+', 'K_S0'])
# p.addChannel(['D-', 'D*+', 'K_S0'])
# p.addChannel(['D*-', 'D*+', 'K_S0'])
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

    mva_B0Semileptonic = Particle.MVAConfiguration(
        variables=[
            'daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
        target='isSignalAcceptMissingNeutrino',
        **mvaDefaults
    )

    pre_B0Semileptonic = Particle.PreCutConfiguration(
        variable='daughterProductOf(extraInfo(SignalProbability))',
        binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
        efficiency=0.95,
        purity=0.0001,
        userCut=BlevelExtraCut
    )

    p = Particle('B0:semileptonic', mva_B0Semileptonic, pre_B0Semileptonic)
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
