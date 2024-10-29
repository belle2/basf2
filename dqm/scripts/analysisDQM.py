# !/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This module defines functions to add analysis DQM modules.
"""

import basf2 as b2
from stdPi0s import stdPi0s
from stdCharged import stdCharged
import modularAnalysis as ma
import stdV0s
import vertex


def add_analysis_dqm(path):
    """Add the analysis DQM modules to the ``path``.
    Builds a list of muons, pi0's, Kshorts and the Fox Wolfram event shape variables.
    Also M(mumu) for nominal beam energy monitoring.

    Parameters:
        path (basf2.Path): modules are loaded onto this path
    """

    # muons, Kshorts and pi0s
    ma.fillParticleList('mu+:KLMDQM', 'p>1.5', path=path)
    ma.fillParticleList('gamma:physDQM', 'E > 0.15', path=path)
    ma.fillParticleList('mu+:physDQM', 'pt>2. and abs(d0) < 2 and abs(z0) < 4', path=path)
    ma.reconstructDecay('pi0:physDQM -> gamma:physDQM gamma:physDQM', '0.10 < M < 0.15', 1, True, path)
    # std Kshorts-TreeFit
    stdV0s.stdKshorts(path=path, updateAllDaughters=True, writeOut=True)
    ma.reconstructDecay('Upsilon:physDQM -> mu-:physDQM mu+:physDQM', '9 < M < 12', 1, True, path)
    # bhabha,hadrons
    ma.fillParticleList('e+:physDQM', 'pt>0.2 and abs(d0) < 2 and abs(z0) < 4 and thetaInCDCAcceptance', path=path)
    ma.reconstructDecay('Upsilon:ephysDQM -> e-:physDQM e+:physDQM', '4 < M < 12', 1, True, path)
    ma.fillParticleList('pi+:hadbphysDQM', 'p>0.1 and abs(d0) < 2 and abs(z0) < 4 and thetaInCDCAcceptance', path=path)

    # have to manually create "all" lists of pi+ and photons to use inside buildEventShape
    # to avoid loading the photons' beamBackgroundMVA variable on the DQM
    ma.fillParticleList('pi+:evtshape', '', path=path)
    ma.fillParticleList('gamma:evtshape', '', path=path)
    ma.buildEventShape(
        path=path,
        inputListNames=['pi+:evtshape', 'gamma:evtshape'],
        default_cleanup=False,  # do not want any clean up
        foxWolfram=True,
        cleoCones=False,
        collisionAxis=False,
        harmonicMoments=False,
        jets=False,
        sphericity=False,
        thrust=False)

    dqm = b2.register_module('PhysicsObjectsDQM')
    dqm.param('PI0PListName', 'pi0:physDQM')
    dqm.param('KS0PListName', 'K_S0:merged')
    dqm.param('UpsPListName', 'Upsilon:physDQM')
    # bhabha,hadrons
    dqm.param('UpsBhabhaPListName', 'Upsilon:ephysDQM')
    dqm.param('UpsHadPListName', 'pi+:hadbphysDQM')

    path.add_module(dqm)


def add_mirabelle_dqm(path):
    """Add the mirabelle DQM modules to the ``path``.
    Runs on conditional paths depending on the software trigger results.
    Building D*'s or dimuons on the conditional paths.

    Parameters:
        path (basf2.Path): modules are loaded onto this path
    """
    # Software Trigger to divert the path
    MiraBelleMumu_path = b2.create_path()
    MiraBelleDst1_path = b2.create_path()
    MiraBelleNotDst1_path = b2.create_path()
    MiraBelleDst2_path = b2.create_path()
    # bhabha,hadrons
    MiraBelleBhabha_path = b2.create_path()
    MiraBellehadronb2_path = b2.create_path()

    trigger_skim_mumutight = path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_mumutight"],
        resultOnMissing=0,
    )
    trigger_skim_mumutight.if_value("==1", MiraBelleMumu_path, b2.AfterConditionPath.CONTINUE)

    trigger_skim_dstar_1 = path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_dstar_1"],
        resultOnMissing=0,
    )
    trigger_skim_dstar_1.if_value("==1", MiraBelleDst1_path, b2.AfterConditionPath.CONTINUE)

    trigger_skim_not_dstar_1 = path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_dstar_1"],
        expectedResult=0,
        resultOnMissing=0,
    )
    trigger_skim_not_dstar_1.if_value("==1", MiraBelleNotDst1_path, b2.AfterConditionPath.CONTINUE)
    trigger_skim_dstar_2 = MiraBelleNotDst1_path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_dstar_2"],
        resultOnMissing=0,
    )
    trigger_skim_dstar_2.if_value("==1", MiraBelleDst2_path, b2.AfterConditionPath.CONTINUE)
    # bhabha,hadrons
    trigger_skim_bhabhaall = path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_bhabha_all"],
        resultOnMissing=0,
    )
    trigger_skim_bhabhaall.if_value("==1", MiraBelleBhabha_path, b2.AfterConditionPath.CONTINUE)
    trigger_skim_hadronb2 = path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_hadronb2"],
        resultOnMissing=0,
    )
    trigger_skim_hadronb2.if_value("==1", MiraBellehadronb2_path, b2.AfterConditionPath.CONTINUE)

    # MiraBelle di-muon path
    ma.fillParticleList('mu+:physMiraBelle', '', path=MiraBelleMumu_path)
    ma.reconstructDecay('Upsilon:physMiraBelle -> mu+:physMiraBelle mu-:physMiraBelle', '9 < M < 12', path=MiraBelleMumu_path)
    MiraBelleMumu = b2.register_module('PhysicsObjectsMiraBelle')
    MiraBelleMumu.param('MuPListName', 'mu+:physMiraBelle')
    MiraBelleMumu.param('MuMuPListName', 'Upsilon:physMiraBelle')
    MiraBelleMumu_path.add_module(MiraBelleMumu)

    # MiraBelle D* (followed by D0 -> K pi) path
    ma.fillParticleList('pi+:MiraBelleDst1', 'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst1_path)
    ma.fillParticleList('K+:MiraBelleDst1', 'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst1_path)
    ma.reconstructDecay('D0:MiraBelleDst1_kpi -> K-:MiraBelleDst1 pi+:MiraBelleDst1', '1.7 < M < 2.1', path=MiraBelleDst1_path)
    ma.reconstructDecay('D*+:MiraBelleDst1_kpi -> D0:MiraBelleDst1_kpi pi+:MiraBelleDst1',
                        'useCMSFrame(p) > 2.5 and massDifference(0) < 0.16', path=MiraBelleDst1_path)
    MiraBelleDst1 = b2.register_module('PhysicsObjectsMiraBelleDst')
    MiraBelleDst1.param('DstListName', 'D*+:MiraBelleDst1_kpi')
    MiraBelleDst1_path.add_module(MiraBelleDst1)

    # MiraBelle D* (followed by D0 -> K pi pi0) path
    ma.fillParticleList('pi+:MiraBelleDst2', 'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst2_path)
    ma.fillParticleList('K+:MiraBelleDst2', 'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst2_path)
    stdPi0s(listtype='eff60_May2020', path=MiraBelleDst2_path)
    ma.reconstructDecay(
        'D0:MiraBelleDst2_kpipi0 -> K-:MiraBelleDst2 pi+:MiraBelleDst2 pi0:eff60_May2020',
        '1.7 < M < 2.1',
        path=MiraBelleDst2_path)
    ma.reconstructDecay('D*+:MiraBelleDst2_kpipi0 -> D0:MiraBelleDst2_kpipi0 pi+:MiraBelleDst2',
                        'useCMSFrame(p) > 2.5 and massDifference(0) < 0.16', path=MiraBelleDst2_path)
    MiraBelleDst2 = b2.register_module('PhysicsObjectsMiraBelleDst2')
    MiraBelleDst2.param('DstListName', 'D*+:MiraBelleDst2_kpipi0')
    MiraBelleDst2_path.add_module(MiraBelleDst2)
    # bhabha,hadrons
    ma.fillParticleList(
        'e+:physMiraBelle',
        'pt>0.2 and abs(d0) < 2 and abs(z0) < 4 and thetaInCDCAcceptance',
        path=MiraBelleBhabha_path)
    ma.reconstructDecay('Upsilon:ephysMiraBelle -> e+:physMiraBelle e-:physMiraBelle', '4 < M < 12', path=MiraBelleBhabha_path)
    MiraBelleBhabha = b2.register_module('PhysicsObjectsMiraBelleBhabha')
    MiraBelleBhabha.param('ePListName', 'e+:physMiraBelle')
    MiraBelleBhabha.param('bhabhaPListName', 'Upsilon:ephysMiraBelle')
    MiraBelleBhabha_path.add_module(MiraBelleBhabha)
    ma.fillParticleList(
        'pi+:hadb2physMiraBelle',
        'p>0.1 and abs(d0) < 2 and abs(z0) < 4 and thetaInCDCAcceptance',
        path=MiraBellehadronb2_path)
    MiraBellehadronb = b2.register_module('PhysicsObjectsMiraBelleHadron')
    MiraBellehadronb.param('hadronb2piPListName', 'pi+:hadb2physMiraBelle')
    MiraBellehadronb2_path.add_module(MiraBellehadronb)


#  Selection for the EcmsBB analysis
def get_hadB_path():
    """ Selects the hadronic B decays, function returns corresponding path  """

    # module to be run prior the collector
    rec_path_1 = b2.create_path()
    trigger_skim_BB = rec_path_1.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_hadronb2"],  # TODO what to put there
        resultOnMissing=0,
    )
    trigger_skim_BB.if_value("==1", rec_path_1, b2.AfterConditionPath.CONTINUE)

    stdCharged.stdPi(listtype='loose', path=rec_path_1)
    stdCharged.stdK(listtype='good', path=rec_path_1)
    stdPi0s.stdPi0s(listtype='eff40_May2020', path=rec_path_1)

    ma.cutAndCopyList("pi+:my", "pi+:loose", "[abs(dz)<2.0] and [abs(dr)<0.5]", path=rec_path_1)
    ma.cutAndCopyList("K+:my", "K+:good", "[abs(dz)<2.0] and [abs(dr)<0.5]", path=rec_path_1)

    ma.cutAndCopyList("pi0:my", "pi0:eff40_May2020", "", path=rec_path_1)

    #####################################################
    # Reconstructs the signal B0 candidates from Dstar
    #####################################################

    DcutLoose = '1.7 < M < 2.1'
    Dcut = '1.830 < M < 1.894'
    # Reconstructs D0s and sets decay mode identifiers
    ma.reconstructDecay(decayString='D0:Kpi -> K-:my pi+:my', cut=DcutLoose, dmID=1, path=rec_path_1)
    ma.reconstructDecay(decayString='D0:Kpipi0 -> K-:my pi+:my pi0:my',
                        cut=DcutLoose, dmID=2, path=rec_path_1)
    ma.reconstructDecay(decayString='D0:Kpipipi -> K-:my pi+:my pi-:my pi+:my',
                        cut=DcutLoose, dmID=3, path=rec_path_1)

    # Performs mass constrained fit for all D0 candidates
    vertex.kFit(list_name='D0:Kpi', conf_level=0.0, fit_type='mass', path=rec_path_1)
    # vertex.kFit(list_name='D0:Kpipi0',  conf_level=0.0, fit_type='mass', path=rec_path_1)
    vertex.kFit(list_name='D0:Kpipipi', conf_level=0.0, fit_type='mass', path=rec_path_1)

    ma.applyCuts("D0:Kpi",     Dcut, path=rec_path_1)
    ma.applyCuts("D0:Kpipi0",  Dcut, path=rec_path_1)
    ma.applyCuts("D0:Kpipipi", Dcut, path=rec_path_1)

    DStarcutLoose = 'massDifference(0) < 0.16'

    # Reconstructs D*-s and sets decay mode identifiers
    ma.reconstructDecay(decayString='D*+:D0pi_Kpi -> D0:Kpi pi+:my', cut=DStarcutLoose, dmID=1, path=rec_path_1)
    ma.reconstructDecay(decayString='D*+:D0pi_Kpipi0 -> D0:Kpipi0 pi+:my',
                        cut=DStarcutLoose, dmID=2, path=rec_path_1)
    ma.reconstructDecay(decayString='D*+:D0pi_Kpipipi -> D0:Kpipipi pi+:my',
                        cut=DStarcutLoose, dmID=3, path=rec_path_1)

    BcutLoose = '[ useCMSFrame(p) < 1.6 ] and [abs(dM) < 0.25]'
    Bcut = '[ useCMSFrame(p) < 1.2 ] and [abs(dM) < 0.05]'

    # Reconstructs the signal B0 candidates from Dstar
    ma.reconstructDecay(decayString='B0:Dstpi_D0pi_Kpi -> D*-:D0pi_Kpi pi+:my',
                        cut=BcutLoose,
                        dmID=1, path=rec_path_1)
    ma.reconstructDecay(decayString='B0:Dstpi_D0pi_Kpipi0 -> D*-:D0pi_Kpipi0 pi+:my',
                        cut=BcutLoose,
                        dmID=2, path=rec_path_1)
    ma.reconstructDecay(decayString='B0:Dstpi_D0pi_Kpipipi -> D*-:D0pi_Kpipipi pi+:my',
                        cut=BcutLoose,
                        dmID=3, path=rec_path_1)

    vertex.treeFit('B0:Dstpi_D0pi_Kpi', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)
    vertex.treeFit('B0:Dstpi_D0pi_Kpipi0', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)
    vertex.treeFit('B0:Dstpi_D0pi_Kpipipi', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)

    #####################################################
    # Reconstructs the signal B0 candidates from D-
    #####################################################

    # Reconstructs charged D mesons and sets decay mode identifiers
    ma.reconstructDecay(decayString='D-:Kpipi -> K+:my pi-:my pi-:my',
                        cut=DcutLoose, dmID=4, path=rec_path_1)

    vertex.kFit(list_name='D-:Kpipi', conf_level=0.0, fit_type='mass', path=rec_path_1)
    ma.applyCuts("D-:Kpipi",  '1.844 < M < 1.894', path=rec_path_1)

    # Reconstructs the signal B candidates
    ma.reconstructDecay(decayString='B0:Dpi_Kpipi -> D-:Kpipi pi+:my',
                        cut=BcutLoose, dmID=4, path=rec_path_1)

    #####################################################
    # Reconstruct the signal B- candidates
    #####################################################

    # Reconstructs the signal B- candidates
    ma.reconstructDecay(decayString='B-:D0pi_Kpi -> D0:Kpi pi-:my',
                        cut=BcutLoose,
                        dmID=5, path=rec_path_1)
    ma.reconstructDecay(decayString='B-:D0pi_Kpipi0 -> D0:Kpipi0 pi-:my',
                        cut=BcutLoose,
                        dmID=6, path=rec_path_1)
    ma.reconstructDecay(decayString='B-:D0pi_Kpipipi -> D0:Kpipipi pi-:my',
                        cut=BcutLoose,
                        dmID=7, path=rec_path_1)

    vertex.treeFit('B-:D0pi_Kpi', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)
    vertex.treeFit('B-:D0pi_Kpipi0', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)
    vertex.treeFit('B-:D0pi_Kpipipi', updateAllDaughters=True, ipConstraint=True, path=rec_path_1)

    ma.copyLists(
        outputListName='B0:merged',
        inputListNames=[
            'B0:Dstpi_D0pi_Kpi',
            'B0:Dstpi_D0pi_Kpipi0',
            'B0:Dstpi_D0pi_Kpipipi',
            'B0:Dpi_Kpipi'
        ],
        path=rec_path_1)

    ma.copyLists(
        outputListName='B-:merged',
        inputListNames=[
            'B-:D0pi_Kpi',
            'B-:D0pi_Kpipi0',
            'B-:D0pi_Kpipipi',
        ],
        path=rec_path_1)

    # Builds the rest of event object, which contains all particles not used in the reconstruction of B0 candidates.
    ma.buildRestOfEvent(target_list_name='B0:merged', path=rec_path_1)

    # Calculates the continuum suppression variables
    cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
    ma.appendROEMasks(list_name='B0:merged', mask_tuples=[cleanMask], path=rec_path_1)
    ma.buildContinuumSuppression(list_name='B0:merged', roe_mask='cleanMask', path=rec_path_1)

    # Builds the rest of event object, which contains all particles not used in the reconstruction of B- candidates.
    ma.buildRestOfEvent(target_list_name='B-:merged', path=rec_path_1)

    # Calculates the continuum suppression variables
    cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
    ma.appendROEMasks(list_name='B-:merged', mask_tuples=[cleanMask], path=rec_path_1)
    ma.buildContinuumSuppression(list_name='B-:merged', roe_mask='cleanMask', path=rec_path_1)

    ma.applyCuts("B0:merged", "[R2 < 0.3] and " + Bcut, path=rec_path_1)
    ma.applyCuts("B-:merged", "[R2 < 0.3] and " + Bcut, path=rec_path_1)

    MiraBelleEcmsBB = b2.register_module('PhysicsObjectsMiraBelleEcmsBB')
    MiraBelleEcmsBB.param('B0ListName', 'B0:merged')
    MiraBelleEcmsBB.param('BmListName', 'Bp:merged')
    rec_path_1.add_module(MiraBelleEcmsBB)

    return rec_path_1
