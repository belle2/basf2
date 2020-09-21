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

    dqm = register_module('PhysicsObjectsDQM')
    dqm.param('PI0PListName', 'pi0:physDQM')
    dqm.param('KS0PListName', 'K_S0:physDQM')
    path.add_module(dqm)


def add_mirabelle_dqm(path):
    # MiraBelle di-muon
    fillParticleList('mu+:physMiraBelle', '', path=path)
    mirabelle = register_module('PhysicsObjectsMiraBelle')
    mirabelle.param('MuPListName', 'mu+:physMiraBelle')
    path.add_module(mirabelle)
    # MiraBelle D*
    fillParticleList('pi+:MiraBelleDst_pion', 'abs(d0)<0.5 and abs(z0)<3', path=path)
    fillParticleList('K+:MiraBelleDst_kaon',  'abs(d0)<0.5 and abs(z0)<3', path=path)
    stdPi0s(listtype='eff60_Jan2020', path=path)
    reconstructDecay('D0:ch1_mirabelle -> K-:MiraBelleDst_kaon pi+:MiraBelleDst_pion', '1.7 < M < 2.1', path=path)
    reconstructDecay('D0:ch2_mirabelle -> K-:MiraBelleDst_kaon pi+:MiraBelleDst_pion pi0:eff60_Jan2020', '1.7 < M < 2.1', path=path)
    reconstructDecay('D*+:kpi -> D0:ch1_mirabelle pi+:MiraBelleDst_pion',
                     'useCMSFrame(p) > 2.5 and massDifference(0) < 0.16', path=path)
    reconstructDecay('D*+:kpipi0 -> D0:ch2_mirabelle pi+:MiraBelleDst_pion',
                     'useCMSFrame(p) > 2.5 and massDifference(0) < 0.16', path=path)
    listmode = ['D*+:kpi', 'D*+:kpipi0']
    copyLists('D*+:MiraBelleDst', listmode, path=path)
    mirabelleDst = register_module('PhysicsObjectsMiraBelleDst')
    mirabelleDst.param('DstListName', 'D*+:MiraBelleDst')
    path.add_module(mirabelleDst)
