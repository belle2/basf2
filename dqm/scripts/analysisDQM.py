# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add analysis DQM modules.
"""

from basf2 import *
from stdPi0s import stdPi0s
from modularAnalysis import *


def add_analysis_dqm(path):
    fillParticleList('gamma:physDQM', 'E > 0.15', path=path)
    fillParticleList('pi+:physDQM', 'pt>0.2 and abs(d0) < 2 and abs(z0) < 4', path=path)
    reconstructDecay('pi0:physDQM -> gamma:physDQM gamma:physDQM', '0.10 < M < 0.15', 1, True, path)
    reconstructDecay('K_S0:physDQM -> pi-:physDQM pi+:physDQM', '0.48 < M < 0.52', 1, True, path)
    buildEventShape(path=path)

    dqm = register_module('PhysicsObjectsDQM')
    dqm.param('PI0PListName', 'pi0:physDQM')
    dqm.param('KS0PListName', 'K_S0:physDQM')
    path.add_module(dqm)


def add_mirabelle_dqm(path):
    # Software Trigger to divert the path
    MiraBelleMumu_path = create_path()
    MiraBelleDst1_path = create_path()
    MiraBelleNotDst1_path = create_path()
    MiraBelleDst2_path = create_path()

    trigger_skim_mumutight = path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_mumutight"])
    trigger_skim_mumutight.if_value("==1", MiraBelleMumu_path, AfterConditionPath.CONTINUE)

    trigger_skim_dstar_1 = path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_dstar_1"])
    trigger_skim_dstar_1.if_value("==1", MiraBelleDst1_path, AfterConditionPath.CONTINUE)

    trigger_skim_not_dstar_1 = path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&skim&accept_dstar_1"],
        expectedResult=0)
    trigger_skim_not_dstar_1.if_value("==1", MiraBelleNotDst1_path, AfterConditionPath.CONTINUE)
    trigger_skim_dstar_2 = MiraBelleNotDst1_path.add_module(
        "TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_dstar_2"])
    trigger_skim_dstar_2.if_value("==1", MiraBelleDst2_path, AfterConditionPath.CONTINUE)

    # MiraBelle di-muon path
    fillParticleList('mu+:physMiraBelle', '', path=MiraBelleMumu_path)
    MiraBelleMumu = register_module('PhysicsObjectsMiraBelle')
    MiraBelleMumu.param('MuPListName', 'mu+:physMiraBelle')
    MiraBelleMumu_path.add_module(MiraBelleMumu)

    # MiraBelle D* (followed by D0 -> K pi) path
    fillParticleList('pi+:MiraBelleDst1', 'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst1_path)
    fillParticleList('K+:MiraBelleDst1',  'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst1_path)
    reconstructDecay('D0:MiraBelleDst1_kpi -> K-:MiraBelleDst1 pi+:MiraBelleDst1', '1.7 < M < 2.1', path=MiraBelleDst1_path)
    reconstructDecay('D*+:MiraBelleDst1_kpi -> D0:MiraBelleDst1_kpi pi+:MiraBelleDst1',
                     'useCMSFrame(p) > 2.5 and massDifference(0) < 0.16', path=MiraBelleDst1_path)
    MiraBelleDst1 = register_module('PhysicsObjectsMiraBelleDst')
    MiraBelleDst1.param('DstListName', 'D*+:MiraBelleDst1_kpi')
    MiraBelleDst1_path.add_module(MiraBelleDst1)

    # MiraBelle D* (followed by D0 -> K pi pi0) path
    fillParticleList('pi+:MiraBelleDst2', 'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst2_path)
    fillParticleList('K+:MiraBelleDst2',  'abs(d0)<0.5 and abs(z0)<3', path=MiraBelleDst2_path)
    stdPi0s(listtype='eff60_Jan2020', path=MiraBelleDst2_path)
    reconstructDecay(
        'D0:MiraBelleDst2_kpipi0 -> K-:MiraBelleDst2 pi+:MiraBelleDst2 pi0:eff60_Jan2020',
        '1.7 < M < 2.1',
        path=MiraBelleDst2_path)
    reconstructDecay('D*+:MiraBelleDst2_kpipi0 -> D0:MiraBelleDst2_kpipi0 pi+:MiraBelleDst2',
                     'useCMSFrame(p) > 2.5 and massDifference(0) < 0.16', path=MiraBelleDst2_path)
    MiraBelleDst2 = register_module('PhysicsObjectsMiraBelleDst2')
    MiraBelleDst2.param('DstListName', 'D*+:MiraBelleDst2_kpipi0')
    MiraBelleDst2_path.add_module(MiraBelleDst2)
