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
import modularAnalysis as ma
import vertex as vx


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
    ma.fillParticleList('pi+:physDQM', 'pt>0.2 and abs(d0) < 2 and abs(z0) < 4', path=path)
    ma.fillParticleList('mu+:physDQM', 'pt>2. and abs(d0) < 2 and abs(z0) < 4', path=path)
    ma.reconstructDecay('pi0:physDQM -> gamma:physDQM gamma:physDQM', '0.10 < M < 0.15', 1, True, path)
    ma.reconstructDecay('K_S0:physDQM -> pi-:physDQM pi+:physDQM', '0.48 < M < 0.52', 1, True, path)
    # remove all K_S0 that fail the fit
    vx.treeFit('K_S0:physDQM', conf_level=0.0, path=path, updateAllDaughters=True)
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
    dqm.param('KS0PListName', 'K_S0:physDQM')
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
