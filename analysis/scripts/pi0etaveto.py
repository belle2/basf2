from modularAnalysis import *
from variables import variables


def pi0etaveto(
    particleList,
    decayString,
    selection='',
    path=analysis_main,
):
    """
    Give pi0/eta probability for high energy photon when its energy is higher than 1.4GeV.
    The clusterTimingThreshold precut of low energy photon is energy dependent clusterTiming cut.

    @param particleList     The input ParticleList
    @param decayString specify Particle to be added to the ParticleList
    @param selection Selection criteria that Particle needs meet in order for for_each ROE path to continue
    """

    variables.addAlias('ctime', 'clusterTimingThreshold')

    roe_path = create_path()

    deadEndPath = create_path()

    signalSideParticleFilter(particleList, selection, roe_path, deadEndPath)

    fillParticleList(
        'gamma:pi0',
        'abs(clusterTiming)<ctime and [clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.02] or [clusterReg==3 and E>0.02]',
        path=roe_path)
    fillParticleList(
        'gamma:eta',
        'abs(clusterTiming)<ctime and [clusterReg==1 and E>0.035] or [clusterReg==2 and E>0.03] or [clusterReg==3 and E>0.03]',
        path=roe_path)

    fillSignalSideParticleList('gamma:sig', decayString, path=roe_path)

    reconstructDecay('pi0:veto -> gamma:sig gamma:pi0', '', path=roe_path)
    reconstructDecay('eta:veto -> gamma:sig gamma:eta', '', path=roe_path)

    variables.addAlias('lowE', 'daughter(1,E)')
    variables.addAlias('cTheta', 'daughter(1,clusterTheta)')
    variables.addAlias('Zmva', 'daughter(1,clusterZernikeMVA)')
    variables.addAlias('minC2Hdist', 'daughter(1,minC2HDist)')

    roe_path.add_module('MVAExpert', listNames=['pi0:veto'], extraInfoName='Pi0Veto',
                        identifier='/gpfs/home/belle2/otakyo/pi0etaveto/pi0veto.root')
    roe_path.add_module('MVAExpert', listNames=['eta:veto'], extraInfoName='EtaVeto',
                        identifier='/gpfs/home/belle2/otakyo/pi0etaveto/etaveto.root')

    rankByHighest('pi0:veto', 'extraInfo(Pi0Veto)', 1, path=roe_path)
    rankByHighest('eta:veto', 'extraInfo(EtaVeto)', 1, path=roe_path)

    variableToSignalSideExtraInfo('pi0:veto', {'extraInfo(Pi0Veto)': 'Pi0_Prob'}, path=roe_path)
    variableToSignalSideExtraInfo('eta:veto', {'extraInfo(EtaVeto)': 'Eta_Prob'}, path=roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
