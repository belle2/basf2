#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
List of functions to skim events containing :math:`B\\to X_c + h` decays,
where :math:`X_c` stays for :math:`D^0`, :math:`D^{\\pm}`, :math:`D^{*0}` and :math:`D^{*\\pm}`,
and :math:`h` stays for :math:`\\pi^{\\pm}`, :math:`K^{\\pm}`, :math:`\\rho^{\\pm}` and :math:`a_1^{\\pm}`.
"""

__authors__ = [
    "Fernando Abudinen", "Chiara La Licata", "Niharika Rout",
]


from basf2 import *
from modularAnalysis import *
import sys
import glob
import os.path
import basf2


def BsigToD0hToKpiList(path):
    """
    Skim list definitions for all charged  B to charm 3 body decays.

    **Skim Author**: Niharika Rout

    **Skim Name**: BtoD0h_Kpi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140101

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^-) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^-) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.7 < M_D0 < 2.0
    2. Mbc > 5.2
    3. abs(deltaE) < 0.5

    Note:
    This skim uses loadD0_Kpi() from the ``hadrons_for_validation.py`` script
    where D0 channels are defined.


    """

    __author__ = "Niharika Rout"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BsigChannels = ['D0:Kpi pi-:all',
                    'D0:Kpi K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoD0h' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B-:BtoD0h' + str(chID))

    Lists = BsigList
    return Lists


def BsigToD0hToKpipipiList(path):
    """
    **Skim Author**: N. Rout

    **Skim Name**: BtoD0h_Kpipipi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140102

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:


    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^- pi^- pi^+) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^- pi^- pi^+) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.7 < M_D0 < 2.0
    2. Mbc > 5.2
    3. abs(deltaE) < 0.5

    Note:
    This skim uses loadD0_Kpipipi() from the ``hadrons_for_validation.py`` script
    where D0 channels are defined.

    """
    __author__ = "Niharika Rout"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BsigChannels = ['D0:Kpipipi pi-:all',
                    'D0:Kpipipi K-:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        reconstructDecay('B-:BtoDhkpipipi' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B-:BtoDhkpipipi' + str(chID))

    Lists = BsigList
    return Lists
