#!/usr/bin/env python3

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
import variables as va
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdK, stdPi, stdCharged
from stdPhotons import loadStdSkimPhoton
from stdPi0s import loadStdSkimPi0, stdPi0s
from stdV0s import stdKshorts, stdLambdas
from variables import variables as vm


__liaison__ = "Jaeyoung Kim <jaeyoung_kim@yonsei.ac.kr>"
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

    __authors__ = ["Dinura Hettiarachchi"]
    __description__ = "Skim list for inclusive D0 two body decays."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL("The analysis globaltag is not set in the charm D0 -> HpJm skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)

    # Cached static method, so that its contents are only executed once for a single path.
    # Factored out into a separate function here, so it is available to other skims.

    @staticmethod
    @lru_cache()
    def D0ToHpJm(path):
        """
        **Decay Modes**:
            * :math:`D^{0} \\to \\pi^+ \\pi^-`,
            * :math:`D^{0} \\to K^+ \\pi^-`,
            * :math:`D^{0} \\to K^- \\pi^+`,
            * :math:`D^{0} \\to K^+ K^-`,

        **Selection Criteria**:
            * Use tracks from the charm_skim_std_charged
            * ``1.70 < M(D0) < 2.00, pcms(D0) > 2.0``
            * `` K/pi binary ID > 0.2, pi_pionIDNN > 0.1``
            * For more details, please check the source code of this skim.

        """

        va.variables.addAlias('binaryID', 'formula(kaonID_noSVD/(pionID_noSVD+kaonID_noSVD))')
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'binaryID > 0.2', path=path)
        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)

        D0Cuts = "1.70 < M < 2.00 and useCMSFrame(p) > 2.0"
        D0Channels = ["pi+:charmSkim_pid pi-:charmSkim_pid",
                      "pi+:charmSkim_pid K-:charmSkim_pid",
                      "K+:charmSkim_pid pi-:charmSkim_pid",
                      "K+:charmSkim_pid K-:charmSkim_pid",
                      ]

        D0List = []
        for chID, channel in enumerate(D0Channels):
            ma.reconstructDecay("D0:HpJm" + str(chID) + " -> " + channel, D0Cuts, chID, path=path)
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
    ApplyHLTHadronCut = False

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
            * ``1.62 < M(D0) < 2.1, pcms(D0) > 2.0``
            * For more details, please check the source code of this skim.
            * (Maybe in the future, we can add a loose ECL cluster timing cut
              to :math:`\\pi^{0}` in this skim.)

        **Parameters**:
            * path (basf2.Path): Skim path to be processed.

        **Returns**:
            * List of D0 particle list names.

        """
        charmcuts = "1.6 < M < 2.1 and useCMSFrame(p)>2.0"
        ma.cutAndCopyList('pi0:D0', 'pi0:skim', 'p>0.4', path=path)
        D0_Channels = ["pi0:D0 pi0:D0",
                       "K_S0:merged pi0:D0",
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
        * Use electrons, muons, and pions from the `charm_skim_std_charged`
        * ``1.66 < M(D0) < 2.06``
        * No cut on the slow pion
        * ``0 < Q < 0.02``
        * ``pcms(D*) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Jaeyoung Kim", "Doris Yangsoo Kim", "Jaeyeon Kim"]
    __description__ = "Skim list for D*+ to pi+ D0, D0 to rare decay."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        charm_skim_std_charged('e', path=path)
        charm_skim_std_charged('mu', path=path)
        charm_skim_std_charged('pi', path=path)
        loadStdSkimPhoton(path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        charmcuts = "1.66 < M < 2.06"
        Dstcuts = "0 < Q < 0.02 and 2.0 < useCMSFrame(p)"

        D0_Channels = ["gamma:skim gamma:skim",
                       "e+:charmSkim e-:charmSkim",
                       "e+:charmSkim mu-:charmSkim",
                       "e-:charmSkim mu+:charmSkim",
                       "mu+:charmSkim mu-:charmSkim",
                       "pi+:charmSkim pi-:charmSkim"]
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
class DpToPipepem(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+ \\to \\pi^+ e^- e^+`,

    **Selection Criteria**:
        * Use tracks from the charm lists in `charm_skim_std_charged`
        * ``1.67 < M(D+) < 2.17, pcms(D+) > 2.0`` and loose cuts for ee
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Andrej Lozar"]
    __description__ = "Skim list for D+ to pi+ e+ e-."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        charm_skim_std_charged("pi", path=path)
        charm_skim_std_charged("e", path=path)

        ma.cutAndCopyList("e+:mySel", "e+:charmSkim", "electronID_noSVD_noTOP > 0.01", path=path)

    def build_lists(self, path):
        Dpcuts = "1.67 < M < 2.17 and useCMSFrame(p) > 2.0"

        DpList = []
        ma.reconstructDecay("D+:Pipepem -> pi+:charmSkim e+:mySel e-:mySel", Dpcuts, 1, path=path)
        DpList.append("D+:Pipepem")

        return DpList


@fancy_skim_header
class DpToPipmupmum(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+ \\to \\pi^+ \\mu^- \\mu^+`

    **Selection Criteria**:
        * Use tracks from the charm lists in `charm_skim_std_charged`
        * ``1.67 < M(D+) < 2.17, pcms(D+) > 2.0`` and loose cuts for mumu
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Andrej Lozar"]
    __description__ = "Skim list for D+ to pi+ mu+ mu-."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        charm_skim_std_charged("pi", path=path)
        charm_skim_std_charged("mu", path=path)

        ma.cutAndCopyList("mu+:mySel", "mu+:charmSkim", "muonID_noSVD > 0.01", path=path)

    def build_lists(self, path):
        Dpcuts = "1.67 < M < 2.17 and useCMSFrame(p) > 2.0"

        DpList = []
        ma.reconstructDecay("D+:Pipmupmum -> pi+:charmSkim mu+:mySel mu-:mySel", Dpcuts, 1, path=path)
        DpList.append("D+:Pipmupmum")

        return DpList


@fancy_skim_header
class DpToPipKpKm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+ \\to \\pi^+ K^- K^+`

    **Selection Criteria**:
        * Use tracks from the charm lists in `charm_skim_std_charged`
        * ``1.67 < M(D+) < 2.17, pcms(D+) > 2.0`` and loose cuts for KK
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Andrej Lozar"]
    __description__ = "Skim list for D+ to pi+ K+ K-."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        charm_skim_std_charged("pi", path=path)
        charm_skim_std_charged("K", path=path)

        ma.cutAndCopyList("K+:mySel", "K+:charmSkim", "kaonID > 0.1", path=path)

    def build_lists(self, path):
        Dpcuts = "1.67 < M < 2.17 and useCMSFrame(p) > 2.0"

        DpList = []
        ma.reconstructDecay("D+:PipKpKm -> pi+:charmSkim K+:mySel K-:mySel", Dpcuts, 1, path=path)
        DpList.append("D+:PipKpKm")

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
        * Use traacks from the `charm_skim_std_charged`
        * Additional cut: global PID > 0.5(Temporary solution for Run1 re-skimming campaign. It might need to be modified.)
        * ``1.67 < M(D+) < 2.17, pcms(D+) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Jaeyoung Kim", "Aman Sangal"]
    __description__ = "Skim list for D_(s)+ to h+ h- j+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        charm_skim_std_charged('K', path=path)
        charm_skim_std_charged('pi', path=path)

    def build_lists(self, path):
        Dpcuts = "1.67 < M < 2.17 and useCMSFrame(p) > 2.0"
        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionID > 0.5', path=path)
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'kaonID > 0.5', path=path)

        Dp_Channels = ["pi+:charmSkim_pid pi-:charmSkim_pid pi+:charmSkim_pid",
                       "pi+:charmSkim_pid pi-:charmSkim_pid K+:charmSkim_pid",
                       "pi+:charmSkim_pid K-:charmSkim_pid K+:charmSkim_pid",
                       "K+:charmSkim_pid K-:charmSkim_pid pi+:charmSkim_pid",
                       ]

        DpList = []
        for chID, channel in enumerate(Dp_Channels):
            ma.reconstructDecay("D+:HpHmJp" + str(chID) + " -> " + channel, Dpcuts, chID, path=path)
            DpList.append("D+:HpHmJp" + str(chID))

        return DpList


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
    ApplyHLTHadronCut = False

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
class DpToHpPi0(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+ \\to \\pi^+ \\pi^0`

    **Selection Criteria**:
        * Tracks: ``dr < 1, abs(dz) < 3, 0.296706 < theta < 2.61799, pcms(\\pi^{+}) > 0.5``
        * Use :math:`\\pi^{0}` from `stdPi0s` require ``pcms(\\pi^{0}) > 0.5``
        * ``1.57 < M(D+) < 2.17, pcms(D+) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Yifan Jin"]
    __description__ = "Skim list for D+ to h+ pi0 without D* tag."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        # loadStdSkimPi0(path=path)
        stdPi0s(listtype='eff50_May2020Fit', path=path)

    def build_lists(self, path):
        ma.cutAndCopyList('pi+:HpPi0', 'pi+:charmSkim', 'pt > 0.1 and useCMSFrame(p) > 0.5', path=path)
        ma.cutAndCopyList('pi0:HpPi0', 'pi0:eff50_May2020Fit', 'useCMSFrame(p) > 0.5', path=path)

        Dpcuts = "1.57 < M < 2.17 and useCMSFrame(p) > 2.0"

        DList = []
        ma.reconstructDecay("D+:HpPi0 -> pi+:HpPi0 pi0:HpPi0", Dpcuts, path=path)
        DList.append("D+:HpPi0")

        return DList


@fancy_skim_header
class DpToKsHp(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^+ \\to K_S \\pi^+`

    **Selection Criteria**:
        * Tracks: ``dr < 1, abs(dz) < 3, 0.296706 < theta < 2.61799``
        * Use Ks from `stdKshorts`
        * ``1.77 < M(D+) < 1.97, pcms(D+) > 2.0``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Yifan Jin"]
    __description__ = "Skim list for D+ to Ks pi+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        ma.cutAndCopyList('pi+:KsPi+', 'pi+:charmSkim', 'pt > 0.1 and useCMSFrame(p) > 0.5', path=path)

        Dpcuts = "1.77 < M < 1.97 and useCMSFrame(p) > 2.0"

        DList = []
        ma.reconstructDecay("D+:KsPi+ -> K_S0:merged pi+:KsPi+", Dpcuts, path=path)
        DList.append("D+:KsPi+")

        return DList


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
        * Use tracks from the charm lists in `charm_skim_std_charged`
        * ``0.47 < M(eta) < 0.60, p(eta) > 0.24``
        * ``1.66 < M(D0) < 2.06, pcms(D0) > 2.0``
        * No cut on the slow pion
        * ``M(D*)-M(D0) < 0.16``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Jaeyoung Kim"]
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
        charm_skim_std_charged('K', path=path)
        charm_skim_std_charged('pi', path=path)
        loadStdSkimPhoton(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        Dstcuts = "massDifference(0) < 0.160 and useCMSFrame(p) > 2.0"
        charmcuts = "1.66 < M < 2.06"
        ma.reconstructDecay("eta:myskim -> gamma:loose gamma:loose", "0.47 < M < 0.60 and p > 0.24", path=path)
        D0_Channels = [
            "pi-:charmSkim pi+:charmSkim eta:myskim",
            "K-:charmSkim pi+:charmSkim eta:myskim",
            "pi-:charmSkim K+:charmSkim eta:myskim",
            "K-:charmSkim K+:charmSkim eta:myskim",
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
        * ``deltaM<0.2``
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Giulia Casarosa", "Emma Oxford"]
    __description__ = "Same as `XToD0_D0ToNeutrals`, but requiring that the D0 is from D* decay."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):

        D0List = self.D0ToNeutrals(path)

        Dstcuts = "daughterMotherDiffOf(1,M)<0.2"

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
        * Use tracks from the charm lists in `charm_skim_std_charged`
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
        charm_skim_std_charged('K', path=path)
        charm_skim_std_charged('pi', path=path)
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
        ma.reconstructDecay(decayString="omega:myOmega -> pi+:charmSkim pi-:charmSkim pi0:skim", cut=omegacut, path=path)

        ma.reconstructDecay(decayString="D0:ch1 -> omega:myOmega gamma:loose", cut=D0cuts, path=path)
        ma.reconstructDecay(decayString="D*+:ch1 -> D0:ch1 pi+:charmSkim", cut=Dstcuts, dmID=1, path=path)

        # Phi mode
        ma.reconstructDecay(decayString="phi:myPhi -> K+:charmSkim K-:charmSkim", cut=phicut, path=path)
        ma.reconstructDecay(decayString="D0:ch2 -> phi:myPhi gamma:loose", cut=D0cuts,  path=path)
        ma.reconstructDecay(decayString="D*+:ch2 -> D0:ch2 pi+:charmSkim", cut=Dstcuts, dmID=2, path=path)

        # Rho0 mode
        ma.reconstructDecay(decayString="rho0:myRho -> pi+:charmSkim pi-:charmSkim", cut=rhocut, path=path)
        ma.reconstructDecay(decayString="D0:ch3 -> rho0:myRho gamma:loose", cut=D0cuts,  path=path)
        ma.reconstructDecay(decayString="D*+:ch3 -> D0:ch3 pi+:charmSkim", cut=Dstcuts, dmID=3, path=path)

        # anti-K*0 mode
        ma.reconstructDecay(decayString="anti-K*0:myantiKstar -> K-:charmSkim pi+:charmSkim", cut=antiKstarcut, path=path)
        ma.reconstructDecay(decayString="D0:ch4 -> anti-K*0:myantiKstar gamma:loose", cut=D0cuts,  path=path)
        ma.reconstructDecay(decayString="D*+:ch4 -> D0:ch4 pi+:charmSkim", cut=Dstcuts, dmID=4, path=path)

        ma.copyLists(outputListName="D*+:all", inputListNames=["D*+:ch1", "D*+:ch2", "D*+:ch3", "D*+:ch4"], path=path)

        eventCuts = "nParticlesInList(D*+:all) > 0"
        path = self.skim_event_cuts(eventCuts, path=path)

        DstList.append("D*+:all")

        return DstList


@fancy_skim_header
class LambdacTopHpJm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`\\Lambda_c^+ \\to p \\pi^+ \\pi^-`
        * :math:`\\Lambda_c^+ \\to p \\pi^+ K^-`
        * :math:`\\Lambda_c^+ \\to p K^+ \\pi^-`
        * :math:`\\Lambda_c^+ \\to p K^+ K^-`

    **Selection Criteria**:
        * Use tracks from the charm_skim_std_charged
        * ``2.2 < M(Lambda_c) < 2.4, pcms(Lambda_c) > 2.0``
        * K/pi binary ID > 0.2, p/K/pi trinary ID > 0.2, pi_pionIDNN > 0.1
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Suravinda Kospalage"]
    __description__ = "Skim list for Lambda_c+ three body decays including a proton."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL("The analysis globaltag is not set in the charm Lambda_c+ -> proton skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        charm_skim_std_charged('p', path=path)

    def build_lists(self, path):
        va.variables.addAlias('binaryID', 'formula(kaonID_noSVD/(pionID_noSVD+kaonID_noSVD))')
        va.variables.addAlias('trinaryID', 'formula(protonID_noSVD/(pionID_noSVD+kaonID_noSVD+protonID_noSVD))')

        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'binaryID > 0.2', path=path)
        ma.cutAndCopyList('p+:charmSkim_pid', 'p+:charmSkim', 'trinaryID > 0.2', path=path)

        LambdacCuts = "2.2 < M < 2.4 and useCMSFrame(p) > 2.0"
        LambdacChannels = ["p+:charmSkim_pid pi+:charmSkim_pid pi-:charmSkim_pid",
                           "p+:charmSkim_pid pi+:charmSkim_pid K-:charmSkim_pid",
                           "p+:charmSkim_pid K+:charmSkim_pid pi-:charmSkim_pid",
                           "p+:charmSkim_pid K+:charmSkim_pid K-:charmSkim_pid",
                           ]

        LambdacList = []
        for chID, channel in enumerate(LambdacChannels):
            ma.reconstructDecay("Lambda_c+:pHpJm" + str(chID) + " -> " + channel, LambdacCuts, chID, path=path)
            LambdacList.append("Lambda_c+:pHpJm" + str(chID))

        return LambdacList


@fancy_skim_header
class LambdacToSHpJm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`\\Lambda_c^+ \\to \\Sigma^+ \\pi^+ \\pi^-`
        * :math:`\\Lambda_c^+ \\to \\Sigma^+ \\pi^+ K^-`
        * :math:`\\Lambda_c^+ \\to \\Sigma^+ K^+ \\pi^-`
        * :math:`\\Lambda_c^+ \\to \\Sigma^+ K^+ K^-`
        * :math:`\\Lambda_c^+ \\to \\Sigma^+ K_S`

    **Selection Criteria**:
        * Use tracks from the charm_skim_std_charged
        * ``2.2 < M(Lambda_c) < 2.4, pcms(Lambda_c) > 2.0``
        * K/pi binary ID > 0.2, p/K/pi trinary ID > 0.2, pi_pionIDNN > 0.1
        * loose mass window for :math:`\\pi^{0}` and skim selections from stdPi0s
        * ``0.44 < M(K_s) < 0.55, significanceOfDistance > 2.0``
        * :math:`\\pm 3\\sigma` mass windows for :math:`\\Sigma^+`
        * lower bound on significance of distance for :math:`\\Sigma^+` > 2
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Suravinda Kospalage"]
    __description__ = "Skim list for Lambda_c+ three body decays including a Sigma+."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL("The analysis globaltag is not set in the charm Lambdac+ -> Sigma+ skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        va.variables.addAlias('binaryID', 'formula(kaonID_noSVD/(pionID_noSVD+kaonID_noSVD))')
        va.variables.addAlias('trinaryID', 'formula(protonID_noSVD/(pionID_noSVD+kaonID_noSVD+protonID_noSVD))')

        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'binaryID > 0.2', path=path)

        ma.fillParticleList('p+:loose', 'trinaryID > 0.2', path=path)

        ma.cutAndCopyList(
            'K_S0:charmSkim',
            'K_S0:merged',
            'significanceOfDistance > 2.0 and daughter(0,pionIDNN) > 0.1 and daughter(1,pionIDNN) > 0.1',
            path=path)

        ma.cutAndCopyList('pi0:charmSkim', 'pi0:skim', '0.120<InvM<0.145', True, path=path)

        ma.reconstructDecay('Sigma+:charmSkim -> p+:loose pi0:charmSkim',
                            cut='1.166 < M < 1.211 and significanceOfDistance > 2', path=path)

        LambdacCuts = "2.2 < M < 2.4 and useCMSFrame(p) > 2.0"
        LambdacChannels = ["Sigma+:charmSkim pi+:charmSkim_pid pi-:charmSkim_pid",
                           "Sigma+:charmSkim pi+:charmSkim_pid K-:charmSkim_pid",
                           "Sigma+:charmSkim K+:charmSkim_pid pi-:charmSkim_pid",
                           "Sigma+:charmSkim K+:charmSkim_pid K-:charmSkim_pid",
                           "Sigma+:charmSkim K_S0:charmSkim",
                           ]

        LambdacList = []
        for chID, channel in enumerate(LambdacChannels):
            ma.reconstructDecay("Lambda_c+:SHpJm" + str(chID) + " -> " + channel, LambdacCuts, chID, path=path)
            LambdacList.append("Lambda_c+:SHpJm" + str(chID))

        return LambdacList


@fancy_skim_header
class XicpTopHpJm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`\\Xi_c^+ \\to p \\pi^+ K^-`

    **Selection Criteria**:
        * Use tracks from the charm_skim_std_charged
        * ``2.3 < M(Xi_c) < 2.65, pcms(Xi_c) > 2.0``
        * K/pi binary ID > 0.2, p/K/pi trinary ID > 0.2, pi_pionIDNN > 0.1
        * For more details, please check the source code of this skim.

    """

    __authors__ = ["Suravinda Kospalage"]
    __description__ = "Skim list for Xi_c+ three body decays including a proton."
    __contact__ = __liaison__
    __category__ = "physics, charm, cascade"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL("The analysis globaltag is not set in the charm Xi_c+ -> proton skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        charm_skim_std_charged('p', path=path)

    def build_lists(self, path):
        va.variables.addAlias('binaryID', 'formula(kaonID_noSVD/(pionID_noSVD+kaonID_noSVD))')
        va.variables.addAlias('trinaryID', 'formula(protonID_noSVD/(pionID_noSVD+kaonID_noSVD+protonID_noSVD))')

        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'binaryID > 0.2', path=path)
        ma.cutAndCopyList('p+:charmSkim_pid', 'p+:charmSkim', 'trinaryID > 0.2', path=path)

        XicCuts = "2.2 < M < 2.4 and useCMSFrame(p) > 2.0"
        XicChannels = ["p+:charmSkim_pid pi+:charmSkim_pid K-:charmSkim_pid",
                       ]

        XicList = []
        for chID, channel in enumerate(XicChannels):
            ma.reconstructDecay("Xi_c+:pHpJm" + str(chID) + " -> " + channel, XicCuts, chID, path=path)
            XicList.append("Xi_c+:pHpJm" + str(chID))

        return XicList


@fancy_skim_header
class XictoXimpippim(BaseSkim):
    """
    **Decay Modes**

    * :math:`\\Xi_c^+ \\to \\Sigma^+ \\pi^+ K^-`
    * :math:`\\Xi_c^+ \\to \\Sigma^+ K^+ K^-`
    * :math:`\\Xi_c^+ \\to \\Sigma^+ \\pi^+ \\pi^-`
    * :math:`\\Xi_c^+ \\to \\Xi^- \\pi^+ \\pi^-`
    * :math:`\\Xi_c^+ \\to \\Xi^0 \\pi^+ \\pi^+ \\pi^-`

    **Selection Criteria**

    * standard track quality constraints on final state particles from charm_skim_std_charged
    * K/pi binary ID > 0.2, p/K/pi trinary ID > 0.2, pi_pionIDNN > 0.1
    * :math:`\\pm 3\\sigma` mass windows for all intermediate hyperons
    * :math:`\\pm 3\\sigma` mass window for pi0 and skim selections from stdPi0s
    * lower bound on significance of distance for all intermediate hyperons, 40% of expected value to be used in analysis
    * loose mass window and lower bound to pCMS for Xic+
    """

    __authors__ = ["Paul Gebeline"]
    __description__ = "Skim for Xi_c+ to hyperons."
    __contact__ = __liaison__
    __category__ = "physics, charm, cascade"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL("The analysis globaltag is not set in the charm Xic+ -> hyperon skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        loadStdSkimPi0(path=path)
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        stdLambdas(path=path)

    def build_lists(self, path):
        va.variables.addAlias('binaryID', 'formula(kaonID_noSVD/(pionID_noSVD+kaonID_noSVD))')
        va.variables.addAlias('trinaryID', 'formula(protonID_noSVD/(pionID_noSVD+kaonID_noSVD+protonID_noSVD))')

        ma.cutAndCopyList('pi+:charmSkim_pid', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)
        ma.cutAndCopyList('K+:charmSkim_pid', 'K+:charmSkim', 'binaryID > 0.2', path=path)

        ma.fillParticleList('pi+:loose', 'pionIDNN > 0.1', path=path)
        ma.fillParticleList('p+:loose', 'trinaryID > 0.2', path=path)

        ma.cutAndCopyList('pi0:charmSkim', 'pi0:skim', '0.120<InvM<0.145', path=path)

        ma.cutAndCopyList(
            'Lambda0:charmSkim',
            'Lambda0:merged',
            '1.114 < M < 1.118 and significanceOfDistance > 3 and daughter(0,trinaryID) > 0.2 and daughter(1,pionIDNN) > 0.1',
            path=path)

        ma.reconstructDecay("Sigma+:charmSkim -> p+:loose pi0:charmSkim",
                            cut="1.166 < M < 1.211 and significanceOfDistance > 2", path=path)
        ma.reconstructDecay(
            "Xi-:Lambda0pi -> Lambda0:charmSkim pi-:loose",
            cut="1.318 < M < 1.325 and significanceOfDistance > 1.4",
            path=path)
        ma.reconstructDecay("Xi0:Lambda0pi0 -> Lambda0:charmSkim pi0:charmSkim",
                            cut="1.294 < M < 1.335 and significanceOfDistance > 2", path=path)

        XicCuts = "2.3 < M < 2.65 and useCMSFrame(p) > 2.0"
        XicChannels = ["Sigma+:charmSkim pi+:charmSkim_pid K-:charmSkim_pid",
                       "Sigma+:charmSkim pi+:charmSkim_pid pi-:charmSkim_pid",
                       "Sigma+:charmSkim K+:charmSkim_pid K-:charmSkim_pid",
                       "Xi-:Lambda0pi pi+:charmSkim_pid pi+:charmSkim_pid",
                       "Xi0:Lambda0pi0 pi+:charmSkim_pid pi+:charmSkim_pid pi-:charmSkim_pid"
                       ]

        XicList = []
        for chID, channel in enumerate(XicChannels):
            ma.reconstructDecay("Xi_c+:HpJm" + str(chID) + " -> " + channel, XicCuts, chID, path=path)
            XicList.append("Xi_c+:HpJm" + str(chID))

        return XicList


@fancy_skim_header
class Xic0ToLHpJm(BaseSkim):
    """
    **Decay Modes**:
        * :math:`\\Xi_c^0 \\to \\Lambda^0 \\pi^+ K^-`
        * :math:`\\Xi_c^0 \\to \\Lambda^0 \\pi^+ \\pi^-`
        * :math:`\\Xi_c^0 \\to \\Lambda^0 K^+ K^-`

    **Selection Criteria**:
        * Use tracks from the charm_skim_std_charged
        * ``2.3 < M(Xi_c) < 2.65, pcms(Xi_c) > 2.0``
        * p/K/pi trinary ID
        * For more details, please check the source code of this skim.
    """

    __authors__ = ["Saroj Pokharel"]
    __description__ = "Skim list for three-body Xi_c0 decays including a Lambda0"
    __contact__ = __liaison__
    __category__ = "physics, charm, cascade"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def additional_setup(self, path):
        if self.analysisGlobaltag is None:
            b2.B2FATAL("The analysis globaltag is not set in the charm Xic0 -> Lambda0 skim.")
        b2.conditions.prepend_globaltag(self.analysisGlobaltag)

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        stdLambdas(path=path)

    def build_lists(self, path):
        va.variables.addAlias('trinaryID', 'formula(protonID_noSVD/(pionID_noSVD+kaonID_noSVD+protonID_noSVD))')

        ma.cutAndCopyList(
            'Lambda0:charmSkim',
            'Lambda0:merged',
            '1.114 < M < 1.118 and significanceOfDistance > 3 and daughter(0,trinaryID) > 0.2',
            path=path)

        XicCuts = "2.3 < M < 2.65 and useCMSFrame(p) > 2.0"
        XicChannels = ["Lambda0:charmSkim pi+:charmSkim K-:charmSkim",
                       "Lambda0:charmSkim pi+:charmSkim pi-:charmSkim",
                       "Lambda0:charmSkim K+:charmSkim K-:charmSkim",
                       ]

        Xic0List = []
        for chID, channel in enumerate(XicChannels):
            ma.reconstructDecay("Xi_c0:LHpJm" + str(chID) + " -> " + channel, XicCuts, chID, path=path)
            Xic0List.append("Xi_c0:LHpJm" + str(chID))

        return Xic0List


@fancy_skim_header
class DstToD0Pi_D0ToGeneric(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{*+}\\to D^{0} \\pi^{+}` (signal side)
        * :math:`D^{*+}\\to D^0 \\pi^{+}` and :math:`D^{*+}\\to D^{+} \\pi^{0}` (tag side)
        * :math:`D^{*0}\\to D^0 \\pi^{0}` and :math:`D^{*0}\\to D^{0} \\gamma` (tag side)
        * :math:`D_s^{*+}\\to D_s^{+} \\gamma` (tag side)
        * 15 hadronic channels for tag :math:`D^{0}` reconstruction
        * 11 hadronic channels for tag :math:`D^{+}` reconstruction
        * 10 hadronic channels for tag :math:`D_{s}^{+}` reconstruction
        * 19 hadronic channels for tag :math:`\\Lambda_{c}^{+}` reconstruction

    **Selection Criteria**:
        * Cut on :math:`\\pi^{+}, K^{+}` : charm_skim_std_charged pion and kaon with PIDNN > 0.1
        * Cut on :math:`p^{+}` : abs(dr) < 1.0 and abs(dz) < 3.0 and protonID > 0.1
        * Cut on :math:`\\gamma` : E > 0.05
        * Cut on :math:`\\pi^{0}\\to \\gamma \\gamma` : 0.115 < M < 0.160
        * Cut on tag side :math:`D^{0}` : 1.72 < M < 2.02 and p* > 2.0
        * Cut on tag side :math:`D^{+}` : 1.72 < M < 2.02 and p* > 2.0
        * Cut on tag side :math:`D_{s}^{+}` :  1.82 < M < 2.12 and p* > 2.0
        * Cut on tag side :math:`\\Lambda_{c}^{+}` : 2.18 < M < 2.38 and p* > 2.0
        * 0.135 < massDifference(0) < 0.155 on decay with :math:`\\pi_{s}^{+}` on :math:`D_{tag}^{*}`
        * 0.130 < massDifference(0) < 0.160 on decay with :math:`\\pi^{0}` on :math:`D_{tag}^{*}`
        * 0.120 < massDifference(0) < 0.165 on decay with :math:`\\gamma` on :math:`D_{tag}^{*}`
        * 1.81 < M < 2.21 on signal side :math:`D^{*+}`
        * 0.115 < :math:`M_{D^{*}}` - :math:`M_{D}` < 0.220 and p* > 2.0 on signal side :math:`D^{0}`
    """
    vm.addAlias('reco_px', 'formula(daughter(0,pxRecoil)-daughter(1,px))')
    vm.addAlias('reco_py', 'formula(daughter(0,pyRecoil)-daughter(1,py))')
    vm.addAlias('reco_pz', 'formula(daughter(0,pzRecoil)-daughter(1,pz))')
    vm.addAlias('reco_E', 'formula(daughter(0,eRecoil)-daughter(1,E))')
    vm.addAlias('reco_p', 'formula((reco_px^2 + reco_py^2 + reco_pz^2)^0.5)')
    vm.addAlias('reco_M', 'formula((reco_E^2-reco_p^2)^0.5)')
    vm.addAlias('DelM', 'formula(daughter(0,mRecoil)-reco_M)')
    vm.addAlias('cms_p', 'useCMSFrame(reco_p)')

    __authors__ = ["Chanho Kim"]
    __description__ = "Skim list for Inclusive D0 from charm tagger"
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
        stdKshorts(path=path)
        stdLambdas(path=path)

    def build_lists(self, path):
        ma.cutAndCopyList('pi+:hadtag', 'pi+:charmSkim', 'pionIDNN > 0.1', path=path)
        ma.cutAndCopyList('K+:hadtag', 'K+:charmSkim', 'kaonIDNN > 0.1', path=path)
        ma.fillParticleList("p+:hadtag", "protonID > 0.1  and abs(dr) < 1.0 and abs(dz) < 3.0", path=path)
        ma.fillParticleList("gamma:tag", "E > 0.05", path=path)
        ma.reconstructDecay("pi0:mypion -> gamma:tag gamma:tag", "0.115 < M < 0.160", path=path)
        d0cuts = "1.72 < M < 2.02 and useCMSFrame(p) > 2.0"

        # tag charm hadrons reconstruction (D0/D+/Lambda_c+/D_s+/D*0/D*+/D_s*+)

        D0_channels = [
            "K-:hadtag pi+:hadtag",
            "K-:hadtag pi+:hadtag pi0:mypion",
            "K-:hadtag pi+:hadtag pi0:mypion pi0:mypion",
            "K-:hadtag pi+:hadtag pi+:hadtag pi-:hadtag",
            "K-:hadtag pi+:hadtag pi+:hadtag pi-:hadtag pi0:mypion",
            "pi-:hadtag pi+:hadtag",
            "pi-:hadtag pi+:hadtag pi+:hadtag pi-:hadtag",
            "pi-:hadtag pi+:hadtag pi0:mypion",
            "pi-:hadtag pi+:hadtag pi0:mypion pi0:mypion",
            "K_S0:merged pi+:hadtag pi-:hadtag",
            "K_S0:merged pi+:hadtag pi-:hadtag pi0:mypion",
            "K_S0:merged pi0:mypion",
            "K-:hadtag K+:hadtag",
            "K-:hadtag K+:hadtag pi0:mypion",
            "K-:hadtag K+:hadtag K_S0:merged"]

        D0List = []
        for chID, channel in enumerate(D0_channels):
            ma.reconstructDecay("D0:skimDm" + str(chID) + " -> " + channel, d0cuts, chID, path=path)
            D0List.append("D0:skimDm" + str(chID))

        Dp_channels = [
            "K-:hadtag pi+:hadtag pi+:hadtag",
            "K-:hadtag pi+:hadtag pi+:hadtag pi0:mypion",
            "K-:hadtag K+:hadtag pi+:hadtag",
            "K-:hadtag K+:hadtag pi+:hadtag pi0:mypion",
            "pi+:hadtag pi0:mypion",
            "pi+:hadtag pi+:hadtag pi-:hadtag",
            "pi+:hadtag pi+:hadtag pi-:hadtag pi0:mypion",
            "K_S0:merged pi+:hadtag",
            "K_S0:merged pi+:hadtag pi0:mypion",
            "K_S0:merged pi+:hadtag pi+:hadtag pi-:hadtag",
            "K+:hadtag K_S0:merged K_S0:merged"]

        dpcuts = "1.72 < M < 2.02 and useCMSFrame(p) > 2.0"
        DpList = []
        for chID, channel in enumerate(Dp_channels):
            ma.reconstructDecay("D+:skimDm" + str(chID) + " -> " + channel, dpcuts, chID, path=path)
            DpList.append("D+:skimDm" + str(chID))

        ma.reconstructDecay("Sigma+:hadtag -> p+:hadtag pi0:mypion", "1.08 < M < 1.3", path=path)

        LC_channels = [
                "p+:hadtag K-:hadtag pi+:hadtag",
                "p+:hadtag pi-:hadtag pi+:hadtag",
                "p+:hadtag K-:hadtag K+:hadtag",
                "p+:hadtag K-:hadtag pi+:hadtag pi0:mypion",
                "p+:hadtag K-:hadtag pi+:hadtag pi0:mypion pi0:mypion",
                "p+:hadtag pi+:hadtag pi+:hadtag pi-:hadtag pi-:hadtag",
                "p+:hadtag K_S0:merged",
                "p+:hadtag K_S0:merged pi0:mypion",
                "p+:hadtag K_S0:merged pi+:hadtag pi-:hadtag",
                "Lambda0:merged pi+:hadtag",
                "Lambda0:merged pi+:hadtag pi0:mypion",
                "Lambda0:merged pi+:hadtag pi-:hadtag pi+:hadtag",
                "Lambda0:merged pi+:hadtag gamma:tag",
                "Lambda0:merged pi+:hadtag pi0:mypion gamma:tag",
                "Lambda0:merged pi+:hadtag pi-:hadtag pi+:hadtag gamma:tag",
                "Sigma+:hadtag pi+:hadtag pi-:hadtag",
                "Sigma+:hadtag pi+:hadtag pi-:hadtag pi0:mypion",
                "Sigma+:hadtag pi0:mypion"]

        LCcuts = "2.18 < M < 2.38 and useCMSFrame(p) > 2.0"

        LambdacList = []
        for chID, channel in enumerate(LC_channels):
            ma.reconstructDecay("Lambda_c+:skimDm" + str(chID) + " -> " + channel, LCcuts, chID, path=path)
            LambdacList.append("Lambda_c+:skimDm" + str(chID))

        Ds_channels = [
                "K+:hadtag K-:hadtag pi+:hadtag",
                "K+:hadtag K_S0:merged",
                "K_S0:merged K_S0:merged pi+:hadtag",
                "K+:hadtag K-:hadtag pi+:hadtag pi0:mypion",
                "K_S0:merged K-:hadtag pi+:hadtag pi+:hadtag",
                "K_S0:merged K+:hadtag pi+:hadtag pi-:hadtag",
                "pi+:hadtag pi+:hadtag pi-:hadtag",
                "K_S0:merged pi+:hadtag",
                "K_S0:merged pi+:hadtag pi0:mypion",
                "K+:hadtag K-:hadtag pi+:hadtag pi+:hadtag pi-:hadtag"]

        DScuts = "1.82 < M < 2.12 and useCMSFrame(p) > 2.0"

        DsList = []
        for chID, channel in enumerate(Ds_channels):
            ma.reconstructDecay("D_s+:skimDm" + str(chID) + " -> " + channel, DScuts, chID, path=path)
            DsList.append("D_s+:skimDm" + str(chID))

        ma.copyLists("D0:skim", D0List, path=path)
        ma.copyLists("D+:skim", DpList, path=path)
        ma.copyLists("Lambda_c+:skim", LambdacList, path=path)
        ma.copyLists("D_s+:skim", DsList, path=path)

        ma.reconstructDecay("D*+:skim1 -> D0:skim pi+:hadtag", "0.135 < massDifference(0) < 0.155", 1, path=path)
        ma.reconstructDecay("D*+:skim2 -> D+:skim pi0:mypion", "0.130 < massDifference(0) < 0.160", 2, path=path)
        ma.copyLists("D*+:skim", ["D*+:skim1", "D*+:skim2"], path=path)

        ma.reconstructDecay("D*0:skim1 -> D0:skim pi0:mypion", "0.130 < massDifference(0) < 0.160", 1, path=path)
        ma.reconstructDecay("D*0:skim2 -> D0:skim gamma:tag", "0.120 < massDifference(0) < 0.165", 2, path=path)
        ma.copyLists("D*0:skim", ["D*0:skim1", "D*0:skim2"], path=path)

        ma.reconstructDecay("D_s*+:skim -> D_s+:skim gamma:tag", "0.120 < massDifference(0) < 0.165", path=path)

        # ==============================================================================================
        # ============================ fragmentation part ==============================================
        # ==============================================================================================
        DstP_Xfrag = [
            "", "pi0:mypion", "pi+:hadtag pi-:hadtag", "pi+:hadtag pi-:hadtag pi0:mypion"]

        sigCuts = "1.81 < mRecoil < 2.21"

        sigDst_fromDstP = []
        for chID, channel in enumerate(DstP_Xfrag):
            n = len(DstP_Xfrag)
            ma.reconstructDecay("D*+:dsp_" + str(chID) + " -> D*+:skim " + channel, sigCuts, chID, path=path)
            ma.reconstructDecay("D*+:dsp_" + str(chID + n) + " -> D*+:skim " + channel +
                                " K+:hadtag K-:hadtag", sigCuts, chID + n, path=path)
            sigDst_fromDstP.append("D*+:dsp_" + str(chID))
            sigDst_fromDstP.append("D*+:dsp_" + str(chID + n))
        ma.copyLists("D*+:fromDstP", sigDst_fromDstP, path=path)

        sigDst_fromDp = []
        for chID, channel in enumerate(DstP_Xfrag):
            n = len(DstP_Xfrag)
            ma.reconstructDecay("D*+:dp_" + str(chID) + " -> D*+:skim " + channel, sigCuts, chID, path=path)
            ma.reconstructDecay("D*+:dp_" + str(chID + n) + " -> D*+:skim " + channel +
                                " K+:hadtag K-:hadtag", sigCuts, chID + n, path=path)
            sigDst_fromDp.append("D*+:dp_" + str(chID))
            sigDst_fromDp.append("D*+:dp_" + str(chID + n))
        ma.copyLists("D*+:fromDp", sigDst_fromDp, path=path)

        Dst0_Xfrag = [
            "pi+:hadtag", "pi+:hadtag pi0:mypion", "pi+:hadtag pi-:hadtag pi+:hadtag"]

        sigDst_fromDst0 = []
        for chID, channel in enumerate(Dst0_Xfrag):
            n = len(Dst0_Xfrag)
            ma.reconstructDecay("D*+:dsz_" + str(chID) + " -> D*0:skim " + channel, sigCuts, chID, path=path)
            ma.reconstructDecay("D*+:dsz_" + str(chID + n) + " -> D*0:skim " + channel +
                                " K+:hadtag K-:hadtag", sigCuts, chID + n, path=path)
            sigDst_fromDst0.append("D*+:dsz_" + str(chID))
            sigDst_fromDst0.append("D*+:dsz_" + str(chID + n))
        ma.copyLists("D*+:fromDstz", sigDst_fromDst0, path=path)

        sigDst_fromD0 = []
        for chID, channel in enumerate(Dst0_Xfrag):
            n = len(Dst0_Xfrag)
            ma.reconstructDecay("D*+:dsz_" + str(chID) + " -> D*0:skim " + channel, sigCuts, chID, path=path)
            ma.reconstructDecay("D*+:dsz_" + str(chID + n) + " -> D*0:skim " + channel +
                                " K+:hadtag K-:hadtag", sigCuts, chID + n, path=path)
            sigDst_fromDst0.append("D*+:dsz_" + str(chID))
            sigDst_fromDst0.append("D*+:dsz_" + str(chID + n))
        ma.copyLists("D*+:fromDstz", sigDst_fromDst0, path=path)

        sigDst_fromD0 = []
        for chID, channel in enumerate(Dst0_Xfrag):
            n = len(Dst0_Xfrag)
            ma.reconstructDecay("D*+:dz_" + str(chID) + " -> D0:skim " + channel, sigCuts, chID, path=path)
            ma.reconstructDecay("D*+:dz_" + str(chID + n) + " -> D0:skim " + channel +
                                " K+:hadtag K-:hadtag", sigCuts, chID, path=path)
            sigDst_fromD0.append("D*+:dz_" + str(chID))
            sigDst_fromD0.append("D*+:dz_" + str(chID))
        ma.copyLists("D*+:fromDz", sigDst_fromD0, path=path)

        LC_Xfrag = [
            "pi+:hadtag anti-p-:hadtag",
            "pi+:hadtag pi0:mypion anti-p-:hadtag",
            "pi+:hadtag pi-:hadtag pi+:hadtag anti-p-:hadtag"]

        sigDst_fromLC = []
        for chID, channel in enumerate(LC_Xfrag):
            ma.reconstructDecay("D*+:LC" + str(chID) + " -> Lambda_c+:skim " + channel, sigCuts, chID, path=path)
            sigDst_fromLC.append("D*+:LC" + str(chID))

        ma.copyLists("D*+:fromLC", sigDst_fromLC, path=path)

        DstS_Xfrag = [
            "K_S0:merged", "pi0:mypion K_S0:merged",
            "pi+:hadtag K-:hadtag", "pi+:hadtag pi0:mypion K-:hadtag",
            "pi+:hadtag pi-:hadtag K_S0:merged", "pi+:hadtag pi-:hadtag pi0:mypion K_S0:merged",
            "pi+:hadtag pi-:hadtag pi+:hadtag K-:hadtag"]

        sigDst_fromDstS = []
        for chID, channel in enumerate(DstS_Xfrag):
            ma.reconstructDecay("D*+:Dsts" + str(chID) + " -> D_s*+:skim " + channel, sigCuts, chID, path=path)
            sigDst_fromDstS.append("D*+:Dsts" + str(chID))
        ma.copyLists("D*+:fromDstS", sigDst_fromDstS, path=path)

        sigDst_fromcDS = []
        for chID, channel in enumerate(DstS_Xfrag):
            ma.reconstructDecay("D*+:Ds" + str(chID) + " -> D_s+:skim " + channel, sigCuts, chID, path=path)
            sigDst_fromcDS.append("D*+:Ds" + str(chID))
        ma.copyLists("D*+:fromDs", sigDst_fromcDS, path=path)

        sigDstList = ["D*+:fromDstP", "D*+:fromDstz", "D*+:fromLC", "D*+:fromDstS", "D*+:fromDp", "D*+:fromDz", "D*+:fromDs"]

        ma.copyLists("D*+:skimSig", sigDstList, path=path)

        ma.reconstructDecay("D0:skimSig -> D*+:skimSig pi-:hadtag", "cms_p > 2.0 and 0.115 < DelM < 0.220", path=path)

        sigDzList = ["D0:skimSig"]
        return sigDzList


@fancy_skim_header
class DpToHpOmega(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D^{+}\\to K^{+} \\omega` (and CC)
        * :math:`D^{+}\\to \\pi^+ \\omega` (and CC)
        * :math:`\\omega\\to \\pi^+ \\pi^- \\pi^0`

    **Selection Criteria**:
        * Track cuts are `charm_skim_std_charged` pion and Kaon with ``PID > 0.1``
        * Use :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0`
        * ``p(pi0) > 0.25 and 0.11 < InvM(pi0) < 0.15``
        * ``0.71 < M(omega) < 0.85``
        * ``1.67 < M(D+) < 2.07``
        * ``p*(D+) > 2.0``
    """

    __authors__ = ["Yongheon Ahn"]
    __description__ = "Skim list for D+ to h+ omega, omega to pi+ pi- pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        ma.cutAndCopyList("pi+:my", "pi+:charmSkim", "pionID>0.1", path=path)
        ma.cutAndCopyList("K+:my", "K+:charmSkim", "kaonID>0.1", path=path)
        ma.cutAndCopyList("pi0:my", "pi0:skim", "p>0.25 and [0.11 < InvM < 0.15]", path=path)

        ma.reconstructDecay("omega:3pi -> pi+:my pi-:my pi0:my", "[0.71 < M < 0.85 ]", path=path)

        Dpcuts = "1.67 < M < 2.07 and useCMSFrame(p) > 2.0"

        ma.reconstructDecay("D+:Kpomega -> K+:my omega:3pi", Dpcuts, path=path)
        ma.reconstructDecay("D+:pipomega -> pi+:my omega:3pi", Dpcuts, path=path)

        DList = []
        DList.append("D+:Kpomega")
        DList.append("D+:pipomega")

        return DList


@fancy_skim_header
class DspToHpOmega(BaseSkim):
    """
    **Decay Modes**:
        * :math:`D_{s}^{+}\\to K^{+} \\omega` (and CC)
        * :math:`D_{s}^{+}\\to \\pi^+ \\omega` (and CC)
        * :math:`\\omega\\to \\pi^+ \\pi^- \\pi^0`

    **Selection Criteria**:
        * Track cuts are `charm_skim_std_charged` pion and Kaon with ``PID > 0.1``
        * Use :math:`\\pi^{0}` from `stdPi0s.loadStdSkimPi0`
        * ``p(pi0) > 0.25 and 0.11 < InvM(pi0) < 0.15``
        * ``0.71 < M(omega) < 0.85``
        * ``1.77 < M(D_s+) < 2.17``
        * ``p*(D_s+) > 2.0``
    """

    __authors__ = ["Yongheon Ahn"]
    __description__ = "Skim list for D_s+ to h+ omega, omega to pi+ pi- pi0."
    __contact__ = __liaison__
    __category__ = "physics, charm"

    NoisyModules = ["ParticleLoader", "RootOutput"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        charm_skim_std_charged('pi', path=path)
        charm_skim_std_charged('K', path=path)
        loadStdSkimPi0(path=path)

    def build_lists(self, path):
        ma.cutAndCopyList("pi+:my", "pi+:charmSkim", "pionID>0.1", path=path)
        ma.cutAndCopyList("K+:my", "K+:charmSkim", "kaonID>0.1", path=path)
        ma.cutAndCopyList("pi0:my", "pi0:skim", "p>0.25 and [0.11 < InvM < 0.15]", path=path)

        ma.reconstructDecay("omega:3pi -> pi+:my pi-:my pi0:my", "[0.71 < M < 0.85 ]", path=path)

        Dspcuts = "1.77 < M < 2.17 and useCMSFrame(p) > 2.0"
        ma.reconstructDecay("D_s+:Kpomega -> K+:my omega:3pi", Dspcuts, path=path)
        ma.reconstructDecay("D_s+:pipomega -> pi+:my omega:3pi", Dspcuts, path=path)

        DsList = []
        DsList.append("D_s+:Kpomega")
        DsList.append("D_s+:pipomega")

        return DsList
