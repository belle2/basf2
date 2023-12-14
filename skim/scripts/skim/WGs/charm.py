#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Skim list building functions for charm analyses.

.. Note::
    The Hp, Hm and Jm in the function name represent arbitrary charged particles with
    positive or negative charge. The Hp and Jm in `XToD0_D0ToHpJm`, mean a positive particle
    and a negative particle that could be either conjugate or non-conjugate. The Hp and Hm
    in `EarlyData_DstToD0Pi_D0ToHpHmPi0`, mean a positive particle and a negative particle that
    must be conjugate.

.. Note::
    Although reconstructDecay() would automatically include charged conjugate channel, and D0
    is not distinguishable from anti-D0 in XToD0 skim, both :math:`D^0 \\to f` and :math:`D^0 \\to \\overline{f}`
    mode should be written explicitly in XToD0 skim, because later in the DstToD0Pi skim, the D0 decay strings
    in XToD0 skim would be directly combined with pi+. For example, if one only include D0 -> K+ pi- in XToD0_D0ToHpJm
    skim, then in DstToD0Pi_D0ToHpJm skim, only the D*+ -> [D0 -> K+ pi-] pi+ would be included, and
    D*+ -> [D0 -> K- pi+] pi+ would not be included.
"""

from functools import lru_cache

import basf2 as b2
import modularAnalysis as ma
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr, stdCharged
from stdPhotons import loadStdSkimPhoton
from stdPi0s import loadStdSkimPi0
from stdV0s import stdKshorts


__liaison__ = "Kaikai He <20214008001@stu.suda.edu.cn>"
_VALIDATION_SAMPLE = "mdst14.root"


@lru_cache  # Avoid loading the list multiple time on the same path
def charm_skim_std_charged(particle_type, path):
    """
    Provides a list of charged particles to be used by charm skims named
    ``<particle_type>+:charmSkim``, with basic cuts applied:
    ``dr < 1 and abs(dz) < 3 and thetaInCDCAcceptance``.
    """
    if particle_type not in ['pi', 'K', 'p', 'e', 'mu']:
        raise ValueError(f"Unknown particle type {particle_type!r}")
    stdCharged(particle_type, 'all', path=path)
    ma.cutAndCopyList(
        f"{particle_type}+:charmSkim", f"{particle_type}+:all",
        "dr < 1 and abs(dz) < 3 and thetaInCDCAcceptance", path=path)


@fancy_skim_header
class XToD0_D0ToHpJm(BaseSkim):
    """
    Skims :math:`D^0`'s reconstructed by `XToD0_D0ToHpJm.D0ToHpJm`.
    """

    __authors__ = ["Giulia Casarosa"]
    __description__ = "Skim list for D0 to two charged FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    # Cached static method, so that its contents are only executed once for a single path.
    # Factored out into a separate function here, so it is available to other skims.

    @staticmethod
    @lru_cache()
    def D0ToHpJm(path):
        """
        **Decay Modes**:
            * :math:`D^{0}\\to \\pi^+ \\pi^-`,
            * :math:`D^{0}\\to K^+ \\pi^-`,
            * :math:`D^{0}\\to K^- \\pi^+`,
            * :math:`D^{0}\\to K^+ K^-`,

        **Selection Criteria**:
            * Tracks: ``abs(d0) < 1, abs(z0) < 3, 0.296706 < theta < 2.61799``
            * ``1.66 < M(D0) < 2.06``
            * ``pcms(D0) > 2.0``
            * For more details, please check the source code of this skim.

        **Parameters**:
            * path (basf2.Path): Skim path to be processed.

        **Returns**:
            * List of D0 particle list names.

        """
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:mygood", mySel, path=path)
        ma.fillParticleList("K+:mygood", mySel, path=path)

        charmcuts = "1.66 < M < 2.06 and useCMSFrame(p)>2.0"
        D0_Channels = [
            "pi+:mygood pi-:mygood",
            "K+:mygood pi-:mygood",
            "K-:mygood pi+:mygood",
            "K+:mygood K-:mygood",
        ]

        D0List = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpJm" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            D0List.append("D0:HpJm" + str(chID))

        return D0List

    def build_lists(self, path):
        """Builds :math:`D^0` skim lists defined in `XToD0_D0ToHpJm.D0ToHpJm`."""
        return self.D0ToHpJm(path)


@fancy_skim_header
class XToD0_D0ToNeutrals(BaseSkim):
    """
    Skims :math:`D^0`'s reconstructed by `XToD0_D0ToNeutrals.D0ToNeutrals`.
    """

    __authors__ = ["Giulia Casarosa"]
    __description__ = "Skim list for D0 to neutral FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]

    def load_standard_lists(self, path):
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    # Cached static method, so that its contents are only executed once for a single path
    # Factored out into a separate function here, so it is available to other skims.

    @staticmethod
    @lru_cache()
    def D0ToNeutrals(path):
        """
        **Decay Modes**:
            * :math:`D^{0}\\to \\pi^{0} \\pi^{0}`
            * :math:`D^{0}\\to K_{S} \\pi^{0}`
            * :math:`D^{0}\\to K_{S} K_{S}`

        **Selection Criteria**:
            * Use :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0`
            * Use :math:`K_{S}` from `stdV0s.stdKshorts`
            * ``1.66 < M(D0) < 2.06, pcms(D0) > 2.0``
            * For more details, please check the source code of this skim.
            * (Maybe in the future, we can add a loose ECL cluster timing cut
              to :math:`\\pi^{0}` in this skim.)

        **Parameters**:
            * path (basf2.Path): Skim path to be processed.

        **Returns**:
            * List of D0 particle list names.

        """
        charmcuts = "1.66 < M < 2.06 and useCMSFrame(p)>2.0"
        D0_Channels = ["pi0:skim pi0:skim",
                       "K_S0:merged pi0:skim",
                       "K_S0:merged K_S0:merged",
                       ]

        D0List = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:2Nbdy" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            D0List.append("D0:2Nbdy" + str(chID))

        return D0List

    def build_lists(self, path):
        """Builds :math:`D^0` skim lists defined in `XToD0_D0ToNeutrals.D0ToNeutrals`."""
        return self.D0ToNeutrals(path)


@fancy_skim_header
class DstToD0Pi_D0ToRare(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\gamma \\gamma`,
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to ee`,
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to e^+ \\mu^-`,
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to e^- \\mu^+`,
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\mu \\mu`,
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^-`,

    **Selection Criteria**:
        * Use photons from `stdPhotons.loadStdSkimPhoton`
        * Use electrons, muons and pions from loose lists in `stdCharged`
        * ``1.66 < M(D0) < 2.06``
        * No cut on the slow pion
        * ``0 < Q < 0.02``
        * ``pcms(D*) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Doris Yangsoo Kim", "Jaeyeon Kim"]
    __description__ = "Skim list for D*+ to pi+ D0, D0 to rare decay."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]

    def load_standard_lists(self, path):
        stdE("loose", path=path)
        stdMu("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        charmcuts = "1.66 < M < 2.06"
        Dstcuts = "0 < Q < 0.02 and 2.0 < useCMSFrame(p)"

        D0_Channels = ["gamma:skim gamma:skim",
                       "e+:loose e-:loose",
                       "e+:loose mu-:loose",
                       "e-:loose mu+:loose",
                       "mu+:loose mu-:loose",
                       "pi+:loose pi-:loose"]
        DstList = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:Rare" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            ma.reconstructDecay("D*+:" + str(chID) + " -> pi+:all D0:Rare" + str(chID),
                                Dstcuts, chID, path=path)
            DstList.append("D*+:" + str(chID))

        return DstList


@fancy_skim_header
class XToDp_DpToKsHp(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+_{(S)} \\to K_{S} \\pi^+`,
        * :math:`D^+_{(S)} \\to K_{S} K^+`,

    **Selection Criteria**:
        * Tracks not from :math:`K_{S}`: ``abs(d0) < 1, abs(z0) < 3,
          0.296706 < theta < 2.61799``
        * Use :math:`K_{S}` from `stdV0s.stdKshorts` and require
          ``flightDistance/flightDistanceErr > 2``
        * ``1.67 < M(D+) < 2.17, pcms(D+) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Guanda Gong"]
    __description__ = "Skim list for D_(s)+ to Ks h+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:kshp", mySel, path=path)
        ma.fillParticleList("K+:kshp", mySel, path=path)
        ma.cutAndCopyList('K_S0:kshp', 'K_S0:merged', 'formula(flightDistance/flightDistanceErr) > 2', path=path)

        Dpcuts = "1.67 < M < 2.17 and useCMSFrame(p) > 2.0"
        Dp_Channels = ["K_S0:kshp pi+:kshp",
                       "K_S0:kshp K+:kshp",
                       ]

        DpList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:KsHp" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            DpList.append("D+:KsHp" + str(chID))

        return DpList


@fancy_skim_header
class XToDp_DpToHpHmJp(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+_{(S)} \\to \\pi^+ \\pi^- \\pi^+`,
        * :math:`D^+_{(S)} \\to \\pi^+ \\pi^- K^+`,
        * :math:`D^+_{(S)} \\to \\pi^+ K^- K^+`,
        * :math:`D^+_{(S)} \\to K^+ K^- K^+`,
        * For :math:`D^+_{(S)} \\to K_{S} h^+`, which includes a :math:`K_{S}` vertex,
          please check `XToDp_DpToKsHp`

    **Selection Criteria**:
        * Use tracks from the loose lists in `stdCharged`
        * ``1.67 < M(D+) < 2.17, pcms(D+) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Aman Sangal"]
    __description__ = "Skim list for D_(s)+ to h+ h- j+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)

    def build_lists(self, path):
        Dpcuts = "1.67 < M < 2.17 and useCMSFrame(p) > 2.0"

        Dp_Channels = ["pi+:loose pi-:loose pi+:loose",
                       "pi+:loose pi-:loose K+:loose",
                       "pi+:loose K-:loose K+:loose",
                       "K+:loose K-:loose pi+:loose",
                       ]

        DpList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:HpHmJp" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            DpList.append("D+:HpHmJp" + str(chID))

        return DpList


@fancy_skim_header
class LambdacTopHpJm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`\\Lambda_c^+ \\to p \\pi^- \\pi^+`
        * :math:`\\Lambda_c^+ \\to p K^- \\pi^+`
        * :math:`\\Lambda_c^+ \\to p \\pi^- K^+`
        * :math:`\\Lambda_c^+ \\to p K^- K^+`

    **Selection Criteria**:
        * Use tracks from the loose lists in `stdCharged`
        * ``2.2 < M(Lambda_c) < 2.4, pcms(Lambda_c) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Justin Guilliams"]
    __description__ = "Skim list for Lambda_c+ -> p K- pi+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdPr("loose", path=path)

    def build_lists(self, path):

        LambdacCuts = "2.2 < M < 2.4 and useCMSFrame(p) > 2.0"
        LambdacChannels = ["p+:loose pi-:all pi+:all",
                           "p+:loose K-:all pi+:all",
                           "p+:loose pi-:all K+:all",
                           "p+:loose K-:all K+:all",
                           ]

        LambdacList = []
        for chID, channel in enumerate(LambdacChannels):
            ma.reconstructDecay("Lambda_c+:pHpJm" + str(chID) + " -> " + channel, LambdacCuts, chID, path=path)
            LambdacList.append("Lambda_c+:pHpJm" + str(chID))

        return LambdacList


@fancy_skim_header
class DstToDpPi0_DpToHpPi0(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^{0} D^{+}, D^+ \\to \\pi^+ \\pi^0`

    **Selection Criteria**:
        * Tracks: ``abs(d0) < 1, abs(z0) < 3, 0.296706 < theta < 2.61799``
        * Use :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0`
        * ``1.67 < M(D+) < 2.07, pcms(D+) > 2.0``
        * ``0 < Q < 0.018``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Johannes Brand"]
    __description__ = "Skim list for D*+ to D+ pi0, D+ to h+ pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:hppi0", mySel, path=path)

        Dpcuts = "1.67 < M < 2.07 and useCMSFrame(p) > 2.0"
        Dp_Channels = ["pi+:hppi0 pi0:skim",
                       ]

        DstList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:HpPi0" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            ma.reconstructDecay("D*+:HpPi0" + str(chID) + " -> D+:HpPi0" + str(chID) + " pi0:skim", "0 < Q < 0.018", path=path)
            DstList.append("D*+:HpPi0" + str(chID))

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpJm(XToD0_D0ToHpJm):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to D^{0} \\pi^+`, where :math:`D^{0}` is reconstructed by `XToD0_D0ToHpJm.D0ToHpJm`

    **Selection Criteria**:
        * Apply the cuts in `XToD0_D0ToHpJm`
        * No cut on the slow pion
        * ``0 < Q < 0.018``
        * For more details, please check the source code of this skim.

    """

    __authors__ = "Giulia Casarosa"
    __description__ = "Same as `XToD0_D0ToHpJm`, but requiring the D0 is from D*+ -> D0 pi+ process."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)

    def build_lists(self, path):
        D0List = self.D0ToHpJm(path)

        Dstcuts = '0 < Q < 0.018'

        DstList = []
        for chID, channel in enumerate(D0List):
            ma.reconstructDecay('D*+:HpJm' + str(chID) + ' -> D0:HpJm' + str(chID) + ' pi+:all', Dstcuts, chID, path=path)
            DstList.append('D*+:HpJm' + str(chID))

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpJmPi0(BaseSkim):
    """
    **Decay Modes**: :math:`D^{*\\pm}\\to D^{0} \\pi^\\pm` with
        * :math:`D^{0}\\to K^\\mp \\pi^\\pm \\pi^0` (and CC)
        * :math:`D^{0}\\to \\pi^+ \\pi^- \\pi^0` (and CC)
        * :math:`D^{0}\\to K^+ K^- \\pi^0` (and CC)

    **Selection Criteria**:
        * Tracks: ``charmSkim`` lists from `charm_skim_std_charged`

          * Pions: ``pionIDNN > 0.1``
          * Kaons: ``kaonIDNN > 0.1``

        * :math:`\\pi^{0}`: from `stdPi0s.loadStdSkimPi0`
        * ``1.70 < M(D0) < 2.10``
        * ``M(D*)-M(D0) < 0.16``
        * ``pCM(D*) > 2.0``

    .. Note::
        The neural-network based PID variables ``pionIDNN`` and ``kaonIDNN``
        require the analysis globaltag to be set (e.g. via
        ``b2skim-run --analysis-globaltag ANALYSIS_GT_NAME ...``). In case
        calibrated weights are not ready for the skimming campaign, this skim
        must be modified to use :b2:var:`binaryPID_noSVD` or
        :b2:var:`binaryPID` instead (this will have some impact on skim
        performance, although small, and needs to be tested).
    """

    __authors__ = ["Ludovico Massaccesi", "Emma Oxford"]
    __description__ = (
        "Skim list for D*-tagged D0 to pi0 and two charged hadrons"
        " (any combination of kaons and pions)."
    )
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL(f"The analysis globaltag is not set in the {self.name} skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        D0_cuts = "1.70 < M < 2.10"
        Dst_cuts = "massDifference(0) < 0.16 and useCMSFrame(p) > 2"
        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'kaonIDNN > 0.1', path=path)

        Dst_lists = []
        for h1, h2 in [('pi', 'pi'), ('pi', 'K'), ('K', 'K')]:
            lst = f"{h1}{h2}Pi0"
            ma.reconstructDecay(f"D0:{lst} -> {h1}+:charmSkim_pid {h2}-:charmSkim_pid pi0:skim", D0_cuts, path=path)
            ma.reconstructDecay(f"D*+:{lst}_RS -> D0:{lst} pi+:charmSkim_pid", Dst_cuts, path=path)
            ma.reconstructDecay(f"D*-:{lst}_WS -> D0:{lst} pi-:charmSkim_pid", Dst_cuts, path=path)
            ma.copyLists(f"D*+:{lst}", [f"D*+:{lst}_RS", f"D*+:{lst}_WS"], path=path)
            Dst_lists.append(f"D*+:{lst}")

        return Dst_lists


@fancy_skim_header
class DstToD0Pi_D0ToKsOmega(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} \\pi^+ \\pi^- \\pi^{0}`

    **Selection Criteria**:
        * Tracks from :math:`D^{0}`:
          ``abs(d0) < 1, abs(z0) < 3, 0.296706 < theta < 2.61799``
        * Use :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0`, then require ``0.11 < M(pi0) < 0.15, p(pi0) > 0.25``
        * ``1.66 < M(D0) < 2.06, pcms(D0) > 2.0``
        * ``Q < 0.018``
        * For more details, please check the source code of this skim.

    """

    __authors__ = []
    __description__ = "Skim list for D*+ to pi+ D0, D0 to Ks pi+ pi- pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:ksomega", mySel, path=path)

        ma.cutAndCopyList("pi0:mypi0", "pi0:skim", "0.11 < M < 0.15 and p > 0.25 ", path=path)
        ma.reconstructDecay("omega:3pi -> pi+:ksomega pi-:ksomega pi0:mypi0", "", path=path)

        charmcuts = "1.66 < M < 2.06 and useCMSFrame(p) > 2.0"
        ma.reconstructDecay("D0:KsOmega -> K_S0:merged omega:3pi", charmcuts, path=path)

        DstList = []
        ma.reconstructDecay("D*+:KsOmega -> D0:KsOmega pi+:all", "0 < Q < 0.018", path=path)
        DstList.append("D*+:KsOmega")

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpHmHpJm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^- \\pi^+ \\pi^-`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^- \\pi^+ K^-`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ \\pi^- K^+ K^-`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\pi^+ K^- K^+ K^-`

    **Selection Criteria**:
        * Tracks from :math:`D^{0}`:
          ``abs(d0) < 1, abs(z0) < 3, 0.296706 < theta < 2.61799``
        * ``1.66 < M(D0) < 2.06``
        * No cut on the slow pion
        * ``Q < 0.022, pcms(D*+) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Kavita Lalwani, Chanchal Sharma"]
    __description__ = "Skim list for D*+ to K- pi+ pi- pi+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdK("all", path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:hphmhpjm", mySel, path=path)
        ma.fillParticleList("K+:hphmhpjm", mySel, path=path)

        D0_Channels = [
            "pi+:hphmhpjm pi-:hphmhpjm pi+:hphmhpjm pi-:hphmhpjm",
            "pi+:hphmhpjm pi-:hphmhpjm pi+:hphmhpjm K-:hphmhpjm",
            "pi+:hphmhpjm pi-:hphmhpjm K+:hphmhpjm K-:hphmhpjm",
            "pi+:hphmhpjm K-:hphmhpjm K+:hphmhpjm K-:hphmhpjm",
        ]

        D0cuts = "1.66 < M < 2.06"
        Dstcuts = "0 < Q < 0.022 and useCMSFrame(p) > 2.0"

        DstList = []
        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpHmHpJm" + str(chID) + " -> " + channel, D0cuts, chID, path=path)

            ma.reconstructDecay("D*+:HpHmHpJm" + str(chID) + " -> pi+:all D0:HpHmHpJm" + str(chID), Dstcuts, chID, path=path)
            DstList.append("D*+:HpHmHpJm" + str(chID))

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpJmEta(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^- D^{0}, D^{0}\\to \\pi^+ \\pi^- \\eta, \\eta\\to \\gamma \\gamma`
        * :math:`RS: D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K^- \\pi^+ \\eta, \\eta\\to \\gamma \\gamma`
        * :math:`WS: D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K^+ \\pi^- \\eta, \\eta\\to \\gamma \\gamma`
        * :math:`D^{*+}\\to \\pi^- D^{0}, D^{0}\\to K^+ K^- \\eta, \\eta\\to \\gamma \\gamma`

    **Selection Criteria**:
        * Use tracks from the loose lists in `stdCharged` to reconstruct D^{0}
        * ``0.47 < M(eta) < 0.60, p(eta) > 0.24``
        * ``1.66 < M(D0) < 2.06, pcms(D0) > 2.0``
        * No cut on the slow pion
        * ``M(D*)-M(D0) < 0.16``
        * For more details, please check the source code of this skim.

    """

    __authors__ = []
    __description__ = (
        "Skim list for D*+ to pi+ D0, D0 to eta and two charged FSPs, where the kinds "
        "of two charged FSPs are different. The wrong sign(WS) mode, D*- to pi- D0, is "
        "also included."
    )
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPhoton(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"
        charmcuts = "1.66 < M < 2.06"
        ma.reconstructDecay("eta:myskim -> gamma:loose gamma:loose", "0.47 < M < 0.60 and p > 0.24", path=path)
        D0_Channels = [
            "pi-:loose pi+:loose eta:myskim",
            "K-:loose pi+:loose eta:myskim",
            "pi-:loose K+:loose eta:myskim",
            "K-:loose K+:loose eta:myskim",
        ]

        DstList = []

        for chID, channel in enumerate(D0_Channels):
            # NOTE: renamed to avoid particle list name clashes
            ma.reconstructDecay("D0:HpJmEta" + str(chID) + " -> " + channel, charmcuts, chID, path=path)
            ma.reconstructDecay(
                "D*+:HpJmEta" + str(chID) + " -> D0:HpJmEta" + str(chID) + " pi+:all",
                Dstcuts, chID, path=path)
            DstList.append("D*+:HpJmEta" + str(chID))

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToNeutrals(XToD0_D0ToNeutrals):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^+ D^{0}`, where the :math:`D^{0}` is reconstructed
          by `XToD0_D0ToNeutrals.D0ToNeutrals`.

    **Selection Criteria**:
        * Apply the cuts in `XToD0_D0ToNeutrals`
        * No cut on the slow pion
        * ``Q < 0.02``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Giulia Casarosa", "Emma Oxford"]
    __description__ = "Same as `XToD0_D0ToNeutrals`, but requiring that the D0 is from D* decay."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):

        D0List = self.D0ToNeutrals(path)

        Dstcuts = "0 < Q < 0.02"

        DstList = []
        for chID, channel in enumerate(D0List):
            ma.reconstructDecay("D*+:2Nbdy" + str(chID) + " -> pi+:all " + channel, Dstcuts, chID, path=path)
            DstList.append("D*+:2Nbdy" + str(chID))

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToHpJmKs(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} \\pi^+ \\pi^-`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} \\pi^+ K^-`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} K^+ \\pi^-`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to K_{S} K^+ K^-`

    **Selection Criteria**:
        * Tracks from :math:`D^{0}`:
          ``abs(d0) < 1, abs(z0) < 3, 0.296706 < theta < 2.61799``
        * Use :math:`K_{S}` from `stdV0s.stdKshorts`
        * ``1.66 < M(D0) < 2.06``
        * No cut on the slow pion
        * ``Q < 0.022``
        * ``pcms(D*) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Yeqi Chen"]
    __description__ = "Skim list for D*+ to pi+ D0, D0 to Ks and two conjugate charged FSPs."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdKshorts(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 1 and abs(z0) < 3"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:hpjmks", mySel, path=path)
        ma.fillParticleList("K+:hpjmks", mySel, path=path)

        D0cuts = "1.66 < M < 2.06"
        Dstcuts = "0 < Q < 0.022 and useCMSFrame(p) > 2.0"

        D0_Channels = ["pi-:hpjmks pi+:hpjmks K_S0:merged",
                       "K-:hpjmks pi+:hpjmks K_S0:merged",
                       "pi-:hpjmks K+:hpjmks K_S0:merged",
                       "K-:hpjmks K+:hpjmks K_S0:merged",
                       ]
        DstList = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpJmKs" + str(chID) + " -> " + channel, D0cuts, chID, path=path)

            ma.reconstructDecay("D*+:HpJmKs" + str(chID) + " -> pi+:all D0:HpJmKs" + str(chID), Dstcuts, chID, path=path)
            DstList.append("D*+:HpJmKs" + str(chID))

        return DstList


@fancy_skim_header
class EarlyData_DstToD0Pi_D0ToHpJmPi0(BaseSkim):
    """
    Cut criteria are not finally decided, and could be changed. Please check the
    code in the master branch to get up-to-date information.
    """

    __authors__ = []
    __description__ = "An special version of `DstToD0Pi_D0ToHpJmPi0` to deal with Early Data."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 0.5 and abs(z0) < 1.0"  # IP cut, tighter than previous skims
        mySel += " and 0.296706 < theta < 2.61799"  # CDC acceptance cut
        ma.fillParticleList("pi+:myhjp0", mySel, path=path)
        ma.fillParticleList("K+:myhjp0", mySel, path=path)

        ma.cutAndCopyList("pi0:myhjp0", "pi0:skim", "", path=path)  # see analysis/scripts/stdPi0s.py for cuts

        D0cuts = "1.70 < M < 2.10"
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"

        eventcuts = "nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 1.0) >= 3"
        path = self.skim_event_cuts(eventcuts, path=path)

        DstList = []
        ma.reconstructDecay("D0:HpJmPi0 -> K-:myhjp0 pi+:myhjp0 pi0:myhjp0", D0cuts, path=path)
        ma.reconstructDecay("D*+:HpJmPi0RS -> D0:HpJmPi0 pi+:myhjp0", Dstcuts, path=path)
        ma.reconstructDecay("D*-:HpJmPi0WS -> D0:HpJmPi0 pi-:myhjp0", Dstcuts, path=path)
        ma.copyLists("D*+:HpJmPi0", ["D*+:HpJmPi0RS", "D*+:HpJmPi0WS"], path=path)
        DstList.append("D*+:HpJmPi0")

        return DstList


@fancy_skim_header
class EarlyData_DstToD0Pi_D0ToHpHmPi0(BaseSkim):
    """
    Cut criteria are not finally decided, and could be changed. Please check the
    code in the master branch to get up-to-date information.
    """

    __authors__ = []
    __description__ = "A special version of ``DstToD0Pi_D0ToHpHmPi0`` to deal with Early Data."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 0.5 and abs(z0) < 1.0"  # IP cut, tighter than previous skims
        mySel += " and 0.296706 < theta < 2.61799"  # CDC acceptance cut
        ma.fillParticleList("pi+:myhhp0", mySel, path=path)
        ma.fillParticleList("K+:myhhp0", mySel, path=path)

        ma.cutAndCopyList("pi0:myhhp0", "pi0:skim", "", path=path)  # see analysis/scripts/stdPi0s.py for cuts

        D0cuts = "1.70 < M < 2.10"
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"

        eventcuts = "nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 1.0) >= 3"
        path = self.skim_event_cuts(eventcuts, path=path)

        D0_Channels = ["pi+:myhhp0 pi-:myhhp0 pi0:myhhp0",
                       "K+:myhhp0 K-:myhhp0 pi0:myhhp0",
                       ]

        DstList = []

        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay("D0:HpHmPi0" + str(chID) + " -> " + channel, D0cuts, chID, path=path)
            ma.reconstructDecay("D*+:HpHmPi0" + str(chID) + " -> D0:HpHmPi0" + str(chID) + " pi+:myhhp0",
                                Dstcuts, chID, path=path)
            DstList.append("D*+:HpHmPi0" + str(chID))

        return DstList


@fancy_skim_header
class DstToD0Pi_D0ToVGamma(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\phi \\gamma`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\rho^{0} \\gamma`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\bar K^{*0} \\gamma`
        * :math:`D^{*+}\\to \\pi^+ D^{0}, D^{0}\\to \\omega \\gamma`

    **Selection Criteria**:
        * Use charged tracks from the loose lists in `stdCharged` to reconstruct :math:`D^{0}`
        * Use :math:`\\gamma` from `stdPhotons.loadStdSkimPhoton`
        * Use :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0`
        * Cut on :math:`\\phi`:
          ``abs(dM) < 0.03``
        * Cut on :math:`\\rho^{0}`:
          ``abs(dM) < 0.3675``
        * Cut on :math:`\\bar K^{*0}`:
          ``abs(dM) < 0.33``
        * Cut on :math:`\\omega`:
          ``abs(dM) < 0.045``
        * Cut on :math:`D^{0}`:
          ``abs(dM) < 0.225 and useCMSFrame(p) > 2``
        * Cut on :math:`D^{*+}`:
          ``massDifference(0) < 0.160``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Jaeyoung Kim"]
    __description__ = "A select of DstToD0Pi_D0ToVGamma, V is the vector meson, including phi, rho0, anti-K*0 and omega."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        omegacut = "abs(dM) < 0.045"
        phicut = "abs(dM) < 0.03"
        rhocut = "abs(dM) < 0.3675"
        antiKstarcut = "abs(dM) < 0.33"

        D0cuts = "abs(dM) < 0.225 and useCMSFrame(p) > 2"
        Dstcuts = "massDifference(0) < 0.160"

        DstList = []

        # Omega mode
        ma.reconstructDecay(decayString="omega:myOmega -> pi+:loose pi-:loose pi0:skim", cut=omegacut, path=path)
        ma.reconstructDecay(decayString="D0:ch1 -> omega:myOmega gamma:loose", cut=D0cuts, path=path)
        ma.reconstructDecay(decayString="D*+:ch1 -> D0:ch1 pi+:loose", cut=Dstcuts, dmID=1, path=path)

        # Phi mode
        ma.reconstructDecay(decayString="phi:myPhi -> K+:loose K-:loose", cut=phicut, path=path)
        ma.reconstructDecay(decayString="D0:ch2 -> phi:myPhi gamma:loose", cut=D0cuts,  path=path)
        ma.reconstructDecay(decayString="D*+:ch2 -> D0:ch2 pi+:loose", cut=Dstcuts, dmID=2, path=path)

        # Rho0 mode
        ma.reconstructDecay(decayString="rho0:myRho -> pi+:loose pi-:loose", cut=rhocut, path=path)
        ma.reconstructDecay(decayString="D0:ch3 -> rho0:myRho gamma:loose", cut=D0cuts,  path=path)
        ma.reconstructDecay(decayString="D*+:ch3 -> D0:ch3 pi+:loose", cut=Dstcuts, dmID=3, path=path)

        # anti-K*0 mode
        ma.reconstructDecay(decayString="anti-K*0:myantiKstar -> K-:loose pi+:loose", cut=antiKstarcut, path=path)
        ma.reconstructDecay(decayString="D0:ch4 -> anti-K*0:myantiKstar gamma:loose", cut=D0cuts,  path=path)
        ma.reconstructDecay(decayString="D*+:ch4 -> D0:ch4 pi+:loose", cut=Dstcuts, dmID=4, path=path)

        ma.copyLists(outputListName="D*+:all", inputListNames=["D*+:ch1", "D*+:ch2", "D*+:ch3", "D*+:ch4"], path=path)

        eventCuts = "nParticlesInList(D*+:all) > 0"
        path = self.skim_event_cuts(eventCuts, path=path)

        DstList.append("D*+:all")

        return DstList


@fancy_skim_header
class DstToDpPi0_DpToHpOmega(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\D^{+} \\pi^0` (and CC)
        * :math:`D^{+}\\to \\K^+ \\omega` (and CC)
        * :math:`D^{+}\\to \\pi^+ \\omega` (and CC)
        * :math:`omega\\to \\pi^+ \\pi^- \\pi^0`

    **Selection Criteria**:
        * Tracks :math: loose `\\pi^{+} and K^{+}` selection from `stdPi' and 'stdK'
        * :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0` and p(pi^{0})>0.25
        * ``0.71 < M(omega) < 0.85``
        * ``1.67 < M(D+) < 2.07``
        * ``2.0 < p*(D+) `
        * ``0.0 < Q(D*+) < 0.018 ``
    """

    __authors__ = ["Yongheon Ahn"]
    __description__ = "Skim list for D*+ to D+ pi0, D+ to h+ omega, omega to pi+ pi- pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("loose", path=path)
        stdK("loose", path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):

        ma.cutAndCopyList("pi0:my", "pi0:skim", "p>0.25", path=path)
        ma.reconstructDecay("omega:3pi -> pi+:loose pi-:loose pi0:my", "[0.71 < M < 0.85 ]", path=path)

        Dpcuts = "1.67 < M < 2.07 and useCMSFrame(p) > 2.0"

        ma.reconstructDecay("D+:Kpomega -> K+:loose omega:3pi", Dpcuts, path=path)
        ma.reconstructDecay("D+:pipomega -> pi+:loose omega:3pi", Dpcuts, path=path)

        ma.reconstructDecay("D*+:K -> D+:Kpomega", "0 < Q < 0.018", path=path)
        ma.reconstructDecay("D*+:pi -> D+:pipomega", "0 < Q < 0.018", path=path)

        DstList = []
        DstList.append("D*+:K")
        DstList.append("D*+:pi")

        return DstList


@fancy_skim_header
class DstToDspPi0_DspToHpOmega(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to \\D^{+} \\pi^0` (and CC)
        * :math:`D_{s}^{+}\\to \\K^+ \\omega` (and CC)
        * :math:`D_{s}^{+}\\to \\pi^+ \\omega` (and CC)
        * :math:`omega\\to \\pi^+ \\pi^- \\pi^0`

    **Selection Criteria**:
        * Tracks :math: loose `\\pi^{+} and K^{+}` selection from `stdPi' and 'stdK'
        * :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0` and p(pi^{0})>0.25
        * ``0.71 < M(omega) < 0.85``
        * ``1.77 < M(D_s+) < 2.17``
        * ``2.0 < p*(D_s+) `
        * ``0.0 < Q(D*+) < 0.018 ``
    """

    __authors__ = ["Yongheon Ahn"]
    __description__ = "Skim list for D*+ to D_s+ pi0, D_s+ to h+ omega, omega to pi+ pi- pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdPi("loose", path=path)
        stdK("loose", path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        mySel = "abs(d0) < 0.5 and abs(z0) < 2"
        mySel += " and 0.296706 < theta < 2.61799"
        ma.fillParticleList("pi+:my", mySel, path=path)
        ma.fillParticleList("K+:my", mySel, path=path)

        ma.cutAndCopyList("pi0:my", "pi0:skim", "p>0.25", path=path)

        ma.reconstructDecay("omega:3pi -> pi+:loose pi-:loose pi0:my", "[0.71 < M < 0.85 ]", path=path)

        Dspcuts = "1.77 < M < 2.17 and useCMSFrame(p) > 2.0"
        ma.reconstructDecay("D_s+:Kpomega -> K+:loose omega:3pi", Dspcuts, path=path)
        ma.reconstructDecay("D_s+:pipomega -> pi+:loose omega:3pi", Dspcuts, path=path)

        ma.reconstructDecay("D*+:K -> D_s+:Kpomega", "0 < Q < 0.018", path=path)
        ma.reconstructDecay("D*+:pi -> D_s+:pipomega", "0 < Q < 0.018", path=path)

        DstList = []
        DstList.append("D*+:K")
        DstList.append("D*+:pi")

        return DstList
