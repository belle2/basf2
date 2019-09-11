#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

from basf2 import *
from modularAnalysis import *


def SemileptonicList(path):
    """
    Note:
        * **Skim description**: skim to be used initially for
          semileptonic :math:`B: decays (:math:`B \\to D \\ell\\nu,`
          where :math:`\\ell=e,\\mu`)
        * **Skim LFN code**: 11160200
        * **Working Group**: (Semi-)Leptonic and Missing Energy
          Working Group (WG1)

    Build leptonic untagged skim lists, and supply the names of the
    lists. Uses the standard electron and muon particle lists, so
    these must be added to the path first. Additionally, charm meson
    decays are defined by the charm list functions, and these require
    various photon, pion and kaon lists to be added to the path.

    Example usage:

    >>> from stdCharged import stdPi, stdK, stdE, stdMu
    >>> from stdPi0s import stdPi0s, stdPhotons
    >>> from stdV0s import stdKshorts
    >>> from skim.standardlists.charm import loadStdD0, loadStdDplus, loadStdDstar0, loadStdDstarPlus
    >>> from skim.semileptonic import SemileptonicList
    >>> stdE('all', path=path)
    >>> stdMu('all', path=path)
    >>> stdPi('all', path=path)
    >>> stdPi('loose', path=path)
    >>> stdPi0s('loose', path=path)
    >>> stdPhotons('loose', path=path)
    >>> stdK('loose', path=path)
    >>> stdKshorts(path=path)
    >>> loadStdD0(path)
    >>> loadStdDplus(path)
    >>> loadStdDstar0(path)
    >>> loadStdDstarPlus(path)
    >>> SemileptonicList(path)
    ['B+:SL1', 'B+:SL2', 'B+:SL3', 'B+:SL4', 'B0:SL1', 'B0:SL2', 'B0:SL3', 'B0:SL4']

    Reconstructed decays
        * :math:`B^+ \\to \\overline{D}^{0} e^+`
        * :math:`B^+ \\to \\overline{D}^{0} \\mu^+`
        * :math:`B^+ \\to \\overline{D}^{*0} e^+`
        * :math:`B^+ \\to \\overline{D}^{*0} \\mu^+`
        * :math:`B^0 \\to  D^{-} e^+`
        * :math:`B^0 \\to  D^{-} \\mu^+`
        * :math:`B^0 \\to  D^{*-} e^+`
        * :math:`B^0 \\to  D^{*-} \\mu^+`

    Cuts applied
        * :math:`p_{\\ell} > 0.35\\,\\text{GeV}`
        * :math:`5.24 < M_{\\text{bc}} < 5.29`
        * :math:`|\\Delta E | < 0.5`
        * :math:`n_{\\text{tracks}} > 4`

    Parameters:
        path (`basf2.Path`): the path to add the skim list builders.

    Returns:
        ``SLLists``, a Python list containing the strings
        :code:`B+:SL1`, :code:`B+:SL2`, :code:`B+:SL3`,
        :code:`B+:SL4`, :code:`B0:SL1`, :code:`B0:SL2`,
        :code:`B0:SL3`, and :code:`B0:SL4`, the names of the particle
        lists for semileptonic :math:`B^+` and :math:`B^0` skim
        candidates.
    """

    __authors__ = [
        "Phillip Urquijo",
        "Racha Cheaib"
    ]

    cutAndCopyList('e+:SLB', 'e+:all', 'p>0.35', True, path=path)
    cutAndCopyList('mu+:SLB', 'mu+:all', 'p>0.35', True, path=path)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    BplusChannels = ['anti-D0:all e+:SLB',
                     'anti-D0:all mu+:SLB',
                     'anti-D*0:all e+:SLB',
                     'anti-D*0:all mu+:SLB'
                     ]

    B0Channels = ['D-:all e+:SLB',
                  'D-:all mu+:SLB',
                  'D*-:all e+:SLB',
                  'D*-:all mu+:SLB'
                  ]

    bplusList = []
    for chID, channel in enumerate(BplusChannels):
        reconstructDecay('B+:SL' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        applyCuts('B+:SL' + str(chID), 'nTracks>4', path=path)
        bplusList.append('B+:SL' + str(chID))

    b0List = []
    for chID, channel in enumerate(B0Channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        applyCuts('B+:SL' + str(chID), 'nTracks>4', path=path)
        b0List.append('B0:SL' + str(chID))

    SLLists = b0List + bplusList
    return SLLists


def PRList(path):
    """
    Note:
        * (Semi-)Leptonic Working Group skim for partial reconstruction analysis.
        * To be used initially for measuring B to D* l v using partial reconstruction.
        * Skim code: 11110100

    **Decay Modes**:

    * B0:L1 ->  pi-:PR1 e+:PR1
    * B0:L2 ->  pi-:PR1 mu+:PR1

    **Cuts applied**:

    * electronID>0.5
    * muonID>0.5
    * lepton Momentum>1.5
    * foxWolframR2<0.5
    * nTracks>4
    """

    __authors__ = [
        "Lucien Cremaldi",
        "Racha Cheaib",
        "Romulus Godang"
    ]

    fillParticleList(decayString='pi+:eventShapeForSkims',
                     cut='pt> 0.1', path=path)
    fillParticleList(decayString='gamma:eventShapeForSkims',
                     cut='E > 0.1', path=path)

    buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
                    allMoments=False,
                    foxWolfram=True,
                    harmonicMoments=False,
                    cleoCones=False,
                    thrust=False,
                    collisionAxis=False,
                    jets=False,
                    sphericity=False,
                    checkForDuplicates=False,
                    path=path)

    applyEventCuts('foxWolframR2<0.5 and nTracks>4', path=path)

    cutAndCopyList('e+:PR1', 'e+:all', 'useCMSFrame(p) > 1.50 and electronID > 0.5', path=path)
    cutAndCopyList('mu+:PR1', 'mu+:all', 'useCMSFrame(p) > 1.50 and muonID > 0.5', path=path)
    cutAndCopyList('pi-:PR1', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220', path=path)

    cutAndCopyList('e+:PR2', 'e+:all', '0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5', path=path)
    cutAndCopyList('mu+:PR2', 'mu+:all', '0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5', path=path)
    cutAndCopyList('pi-:PR2', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160', path=path)

    reconstructDecay('B0:L1 ->  pi-:PR1 e+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 1, path=path)
    reconstructDecay('B0:L2 ->  pi-:PR1 mu+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 2, path=path)
    reconstructDecay('B0:L3 ->  pi-:PR2 e+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 3, path=path)
    reconstructDecay('B0:L4 ->  pi-:PR2 mu+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 4, path=path)

    PRList = ['B0:L1', 'B0:L2']

    return PRList
