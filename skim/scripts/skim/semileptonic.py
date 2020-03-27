#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling",
    "Phil Grace"
]

from basf2 import Path
import modularAnalysis as ma
from skimExpertFunctions import ifEventPasses


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
    >>> stdPi0s('skim', path=path)
    >>> stdPi0s('loose', path=path)
    >>> stdPhotons('loose', path=path)
    >>> stdK('all', path=path)
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
        ``SLLists`` (list(str)): A list containing the names of the skim particle lists.
    """

    __authors__ = [
        "Phillip Urquijo",
        "Racha Cheaib"
    ]

    ma.cutAndCopyList('e+:SLUntagged', 'e+:all', 'p>0.35', True, path=path)
    ma.cutAndCopyList('mu+:SLUntagged', 'mu+:all', 'p>0.35', True, path=path)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    BplusChannels = ['anti-D0:all e+:SLUntagged',
                     'anti-D0:all mu+:SLUntagged',
                     'anti-D*0:all e+:SLUntagged',
                     'anti-D*0:all mu+:SLUntagged'
                     ]

    B0Channels = ['D-:all e+:SLUntagged',
                  'D-:all mu+:SLUntagged',
                  'D*-:all e+:SLUntagged',
                  'D*-:all mu+:SLUntagged'
                  ]

    bplusList = []
    for chID, channel in enumerate(BplusChannels):
        ma.reconstructDecay(f"B+:SLUntagged_{chID} -> {channel}", Bcuts, chID, path=path)
        ma.applyCuts(f"B+:SLUntagged_{chID}", "nTracks>4", path=path)
        bplusList.append(f"B+:SLUntagged_{chID}")

    b0List = []
    for chID, channel in enumerate(B0Channels):
        ma.reconstructDecay(f"B0:SLUntagged_{chID} -> {channel}", Bcuts, chID, path=path)
        ma.applyCuts(f"B0:SLUntagged_{chID}", "nTracks>4", path=path)
        b0List.append(f"B0:SLUntagged_{chID}")

    SLLists = b0List + bplusList
    return SLLists


def PRList(path):
    """
    Note:
        * (Semi-)Leptonic Working Group skim for partial reconstruction analysis.
        * To be used initially for measuring B to D* l v using partial reconstruction.
        * Skim code: 11110100

    **Decay Modes**:

    * :math:`B^0 \\to \\pi^- e^+`
    * :math:`B^0 \\to \\pi^- \\mu^+`

    **Cuts applied**:

    Event-level cuts:

    * :math:`\\text{foxWolframR2} > 0.5` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} > 4`

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.5`
    * :math:`p > 1.5\\,\\text{GeV}` in CMS frame

    Cuts on muons:

    * :math:`\\text{muonID} > 0.5`
    * :math:`p > 1.5\\,\\text{GeV}` in CMS frame

    Cuts on pions:

    * :math:`\\text{pionID}>0.5`
    * :math:`\\text{muonID}<0.2`
    * :math:`0.060\\,\\text{GeV}<p<0.220\\,\\text{GeV}` in CMS frame

    Cuts on partially reconstructed :math:`B` mesons:

    * :math:`\\cos\\theta_{\\ell,\\,\\pi}<0` in CMS frame.
    """

    __authors__ = [
        "Lucien Cremaldi",
        "Racha Cheaib",
        "Romulus Godang"
    ]

    ma.fillParticleList(decayString='pi+:eventShapeForSkims',
                        cut='pt> 0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims',
                        cut='E > 0.1', path=path)

    ma.buildEventShape(inputListNames=['pi+:eventShapeForSkims', 'gamma:eventShapeForSkims'],
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

    path2 = Path()
    ifEventPasses('foxWolframR2<0.5 and nTracks>4', conditional_path=path2, path=path)

    ma.cutAndCopyList('e+:PRSemileptonic_1', 'e+:all', 'useCMSFrame(p) > 1.50 and electronID > 0.5', path=path2)
    ma.cutAndCopyList('mu+:PRSemileptonic_1', 'mu+:all', 'useCMSFrame(p) > 1.50 and muonID > 0.5', path=path2)
    ma.cutAndCopyList('pi-:PRSemileptonic_1', 'pi-:all',
                      'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220', path=path2)

    ma.cutAndCopyList('e+:PRSemileptonic_2', 'e+:all', '0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5', path=path2)
    ma.cutAndCopyList('mu+:PRSemileptonic_2', 'mu+:all', '0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5', path=path2)
    ma.cutAndCopyList('pi-:PRSemileptonic_2', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160', path=path2)

    ma.reconstructDecay('B0:PRSemileptonic_1 ->  pi-:PRSemileptonic_1 e+:PRSemileptonic_1',
                        'useCMSFrame(cos(daughterAngle(0,1)))<0.00', 1, path=path2)
    ma.reconstructDecay('B0:PRSemileptonic_2 ->  pi-:PRSemileptonic_1 mu+:PRSemileptonic_1',
                        'useCMSFrame(cos(daughterAngle(0,1)))<0.00', 2, path=path2)
    ma.reconstructDecay('B0:PRSemileptonic_3 ->  pi-:PRSemileptonic_2 e+:PRSemileptonic_2',
                        'useCMSFrame(cos(daughterAngle(0,1)))<1.00', 3, path=path2)
    ma.reconstructDecay('B0:PRSemileptonic_4 ->  pi-:PRSemileptonic_2 mu+:PRSemileptonic_2',
                        'useCMSFrame(cos(daughterAngle(0,1)))<1.00', 4, path=path2)

    PRList = ['B0:PRSemileptonic_1', 'B0:PRSemileptonic_2']

    return PRList
