#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

""" Skim list building functions for systematics studies """

__authors__ = [
    "Sam Cunliffe",
    "Torben Ferber",
    "Ilya Komarov",
    "Yuji Kato"
]

import basf2 as b2
import modularAnalysis as ma
import vertex
from skim import BaseSkim, CombinedSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s
from stdV0s import stdKshorts, stdLambdas
from variables import variables as vm

# TODO: Add liaison name and email address
__liaison__ = ""
__liaison_leptonID__ = "Marcel Hohmann"
_VALIDATION_SAMPLE = "mdst14.root"


@fancy_skim_header
class SystematicsDstar(BaseSkim):
    """
    Primarily used for hadron and lepton ID studies.
    Lists in this skim are those defined in `PiKFromDstarList`.
    """
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato", "Racha Cheaib"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "systematics"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)

    TestSampleProcess = "ccbar"

    def build_lists(self, path):
        return self.PiKFromDstarList(path)

    def PiKFromDstarList(self, path):
        """Build PiKFromDstarList lists for systematics skims."""
        D0Cuts = "1.75 < M < 2.0"
        DstarCuts = "massDifference(0)<0.16 and useCMSFrame(p) > 1.5"

        ma.cutAndCopyList("K-:syst", "K-:all", "dr<2 and abs(dz)<4", path=path)
        ma.cutAndCopyList("pi+:syst", "pi+:all", "dr<2 and abs(dz)<4", path=path)

        D0Channel = ["K-:syst pi+:syst"]

        D0List = []
        for chID, channel in enumerate(D0Channel):
            ma.reconstructDecay(f"D0:syst{chID} -> {channel}", D0Cuts, chID, path=path)
            D0List.append(f"D0:syst{chID}")

        DstarChannel = []
        for channel in D0List:
            DstarChannel.append(f"{channel} pi+:syst")

        DstarList = []
        for chID, channel in enumerate(DstarChannel):
            ma.reconstructDecay(f"D*+:syst{chID} -> {channel}", DstarCuts, chID, path=path)
            DstarList.append(f"D*+:syst{chID}")

        return DstarList


@fancy_skim_header
class SystematicsTracking(BaseSkim):
    """
    Lists in this skim are those defined in `BtoDStarPiList` and `DstarToD0PiPartList`.
    """
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "systematics"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdPi("loose", path=path)
        stdPi0s("eff40_May2020", path=path)

    def build_lists(self, path):
        return self.BtoDStarPiList(path) + self.DstarToD0PiPartList(path)

    def BtoDStarPiList(self, path):
        """Build BtoDStarPiList lists for systematics skims."""
        D0Cuts = "1.835 < M < 1.895"
        DstarCuts = "massDifference(0)<0.16"
        B0Cuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        # D0
        D0Channel = ["K+:loose pi-:loose", "K+:loose pi-:loose pi-:loose pi+:loose", "K+:loose pi-:loose pi0:eff40_May2020"]

        D0List = []
        for chID, channel in enumerate(D0Channel):
            resonanceName = "anti-D0:loose" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, D0Cuts, chID, path=path)
            # vertex.raveFit(resonanceName, 0.0, path=path)
        ma.copyLists("anti-D0:loose", ["anti-D0:loose0", "anti-D0:loose1", "anti-D0:loose2"], path=path)
        D0List.append("anti-D0:loose")

        # Dstar
        DstarChannel = []
        for channel in D0List:
            DstarChannel.append(channel + " pi-:loose")

        DstarList = []
        for chID, channel in enumerate(DstarChannel):
            resonanceName = "D*-:loose" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, DstarCuts, chID, path=path)
            # vertex.raveFit(resonanceName, 0.0)
            DstarList.append(resonanceName)

        # B0
        B0Channel = []
        for channel in DstarList:
            B0Channel.append(channel + " pi+:loose")

        B0List = []
        for chID, channel in enumerate(B0Channel):
            resonanceName = "B0:sys" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, B0Cuts, chID, path=path)
            B0List.append(resonanceName)
            # vertex.raveFit(resonanceName, 0.0)

        return B0List

    def DstarToD0PiPartList(self, path):
        """Build DstarToD0PiPartList lists for systematics skims."""
        ma.fillParticleList("pi+:fromks", "chiProb > 0.001 and pionID > 0.1 and d0 > 0.1", path=path)

        # D-
        DminusCuts = "1.0 < M < 1.75"
        DminusChannel = ["pi-:fromks pi+:loose pi-:loose"]

        for chID, channel in enumerate(DminusChannel):
            resonanceName = "D-:loose" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, DminusCuts, chID, path=path)

        # Dstar
        DstarCuts = "massDifference(0)<0.2 and useCMSFrame(p) > 2.0"
        DstarChannel = []
        DstarChannel.append("D-:loose0" + " pi+:loose")

        DstarList = []
        for chID, channel in enumerate(DstarChannel):
            resonanceName = "D*0:loose" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, DstarCuts, chID, path=path)
            DstarList.append(resonanceName)

        return DstarList


@fancy_skim_header
class Resonance(BaseSkim):
    """
    Lists in this skim are those defined in `getDsList`, `getDstarList`,
    `getSigmacList`, `getmumugList`, `getBZeroList`, and `getBPlusList`.
    """
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "systematics"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdK("loose", path=path)
        stdMu("loose", path=path)
        stdPi("loose", path=path)
        stdPr("loose", path=path)
        stdPi0s("eff40_May2020Fit", path=path)

    def build_lists(self, path):
        return (
            self.getDsList(path)
            + self.getDstarList(path)
            + self.getSigmacList(path)
            + self.getmumugList(path)
            + self.getBZeroList(path)
            + self.getBPlusList(path)
        )

    def getDsList(self, path):
        """Build Ds list for systematics skims."""
        DsCuts = "1.90 < M < 2.04"

        ma.reconstructDecay("phi:res -> K+:loose K-:loose", "1.01 < M < 1.03", path=path)
        ma.reconstructDecay("K*0:res -> K+:loose pi-:loose", "0.7 < M < 1.1", path=path)

        DsChannel = ["phi:res pi+:loose"]
        DsList = []
        for chID, channel in enumerate(DsChannel):
            particlename = f"D_s+:Resonance{int(chID)}"
            ma.reconstructDecay(particlename + " -> " + channel, DsCuts, chID, path=path)
            DsList.append(particlename)

        return DsList

    def getDstarList(self, path):
        """Build Dstar list for systematics skims."""
        DplusCuts = "1.8 < M < 1.93"
        DstarCuts = "massDifference(0)<0.16 and useCMSFrame(p)>2.0"

        DplusChannel = ["K-:loose pi+:loose pi+:loose"]

        DplusList = []
        for chID, channel in enumerate(DplusChannel):
            ma.reconstructDecay("D+:resonance" + str(chID) + " -> " + channel, DplusCuts, chID, path=path)
            vertex.raveFit("D+:resonance" + str(chID), 0.0, path=path)
            DplusList.append("D+:resonance" + str(chID))

        DstarChannel = []
        for channel in DplusList:
            DstarChannel.append(channel + " pi0:eff40_May2020")

        DstarList = []
        for chID, channel in enumerate(DstarChannel):
            ma.reconstructDecay("D*+:resonance" + str(chID) + " -> " + channel, DstarCuts, chID, path=path)
            DstarList.append("D*+:resonance" + str(chID))

        return DstarList

    def getSigmacList(self, path):
        """Build Sigmac list for systematics skims."""
        LambdacCuts = "2.24 < M < 2.33"
        SigmacCuts = "massDifference(0)<0.28 and useCMSFrame(p) > 2.5"

        LambdacChannel = ["p+:loose K-:loose pi+:loose"]
        LambdacList = []
        for chID, channel in enumerate(LambdacChannel):
            ma.reconstructDecay("Lambda_c+:resonance" + str(chID) + " -> " + channel, LambdacCuts, chID, path=path)
            vertex.raveFit("Lambda_c+:resonance" + str(chID), 0.0, path=path)
            LambdacList.append("Lambda_c+:resonance" + str(chID))

        SigmacList = []
        SigmacPlusChannel = []
        # Sigma_c++
        for channel in LambdacList:
            SigmacPlusChannel.append(channel + " pi+:loose")

        for chID, channel in enumerate(SigmacPlusChannel):
            ma.reconstructDecay("Sigma_c++:resonance" + str(chID) + " -> " + channel, SigmacCuts, chID, path=path)
            SigmacList.append("Sigma_c++:resonance" + str(chID))

        # Sigma_c0
        Sigmac0Channel = []
        for channel in LambdacList:
            Sigmac0Channel.append(channel + " pi-:loose")

        Sigmac0List = []
        for chID, channel in enumerate(Sigmac0Channel):
            ma.reconstructDecay("Sigma_c0:resonance" + str(chID) + " -> " + channel, SigmacCuts, chID, path=path)
            Sigmac0List.append("Sigma_c0:resonance" + str(chID))

        return SigmacList

    def getmumugList(self, path):
        """Build mumug list for systematics skims."""
        vphoChannel = ["mu+:loose mu-:loose"]
        vphocuts = ""
        vphoList = []
        for chID, channel in enumerate(vphoChannel):
            resonanceName = "vpho:resonance" + str(chID)
            ma.reconstructDecay("vpho:resonance" + str(chID) + " -> " + channel, vphocuts, chID, path=path)
            ma.applyCuts(resonanceName, "nTracks == 2 and M < formula(Ecms*0.9877)", path=path)
            vertex.raveFit(resonanceName, 0.0, path=path)
            ma.applyCuts(resonanceName, "M < formula(Ecms*0.9877)", path=path)
            vphoList.append(resonanceName)

        return vphoList

    def getBZeroList(self, path):
        """Build BZero list for systematics skims."""
        BZeroCuts = "Mbc > 5.2 and abs(deltaE) < 0.3"
        BZeroChannel = ["D-:resonance0 pi+:loose"]
        BZeroList = []

        for chID, channel in enumerate(BZeroChannel):
            resonanceName = "B0:resonance" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, BZeroCuts, chID, path=path)
            BZeroList.append(resonanceName)

        return BZeroList

    def getBPlusList(self, path):
        """Build Bplus list for systematics skims."""
        antiDZeroCut = "1.82 < M < 1.90"
        antiDZeroChannel = ["K+:loose pi-:loose"]
        antiDZeroList = []

        for chID, channel in enumerate(antiDZeroChannel):
            resonanceName = "anti-D0:resonance" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, antiDZeroCut, chID, path=path)
            vertex.raveFit(resonanceName, 0.0, path=path)
            antiDZeroList.append(resonanceName)

        BPlusChannel = []
        for channel in antiDZeroList:
            BPlusChannel.append(channel + " pi+:loose")

        BPlusCuts = "Mbc > 5.2 and abs(deltaE) < 0.3"
        BPlusList = []
        for chID, channel in enumerate(BPlusChannel):
            ma.reconstructDecay("B+:resonance" + str(chID) + " -> " + channel, BPlusCuts, chID, path=path)
            BPlusList.append("B+:resonance" + str(chID))

        return BPlusList


@fancy_skim_header
class SystematicsRadMuMu(BaseSkim):
    """
    We require one cluster-matched electron (the other is not required to match a
    cluster). No selection on the photon as the sample must be unbiased.
    """
    __authors__ = ["Torben Ferber"]
    __description__ = (
        "Skim of radiative muon pairs (:math:`ee\\to\\mu\\mu(\\gamma)`) "
        "for photon systematics."
    )
    __contact__ = __liaison__
    __category__ = "systematics, photon calibration"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdMu("all", path=path)

    def build_lists(self, path):
        # the tight selection starts with all muons, but they  must be cluster-matched and not be an electron
        MuonTightSelection = ("abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0 and "
                              "clusterE > 0.0 and clusterE < 1.0")
        ma.cutAndCopyList("mu+:skimtight", "mu+:all", MuonTightSelection, path=path)

        # for the loose selection starts with all muons, but we accept tracks that
        # are not matched to a cluster, but if they are, they must not be an
        # electron
        MuonLooseSelection = "abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0 and clusterE < 1.0"
        ma.cutAndCopyList("mu+:skimloose", "mu+:all", MuonLooseSelection, path=path)

        # create a list of possible selections
        radmumulist = []

        # selection ID0:
        # the radiative muon pair must be selected without looking at the photon.
        # exclude events with more than two good tracks
        RadMuMuSelection = "pRecoil > 0.075 and pRecoilTheta > 0.296706 and pRecoilTheta < 2.61799"
        RadMuMuPairChannel = "mu+:skimtight mu-:skimloose"
        chID = 0
        ma.reconstructDecay("vpho:radmumu" + str(chID) + " -> " + RadMuMuPairChannel,
                            RadMuMuSelection, chID, path=path)
        eventCuts = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2"
        ma.applyCuts("vpho:radmumu" + str(chID), eventCuts, path=path)
        radmumulist.append("vpho:radmumu" + str(chID))

        # selection Id1:
        # todo: include pair conversions?

        return radmumulist


@fancy_skim_header
class SystematicsEELL(BaseSkim):
    __authors__ = ["Ilya Komarov"]
    __description__ = "Systematics skim of :math:`ee\\to ee\\ell\\ell`"
    __contact__ = __liaison__
    __category__ = "systematics, lepton ID"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)

    def build_lists(self, path):
        # At skim level we avoid any PID-like requirements and just select events
        # with two good tracks coming from the interavtion region.
        eLooseSelection = "abs(dz) < 2.0 and abs(dr) < 0.5 and p > 0.3"
        ma.cutAndCopyList("e+:skimloose", "e+:all", eLooseSelection, path=path)

        # create a list of possible selections
        eelllist = []

        # Lepon pair tracks are back-to-back-like
        EELLSelection = "useCMSFrame(pt)<0.3"
        eventCuts = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) < 4"
        ma.reconstructDecay("gamma:eell -> e+:skimloose e-:skimloose",
                            EELLSelection + " and " + eventCuts, path=path)
        eelllist.append("gamma:eell")

        return eelllist


@fancy_skim_header
class SystematicsRadEE(BaseSkim):
    """
    Constructed skim list contains radiative electron pairs for photon systematics. In
    particular this is for the endcaps where we have no track triggers, we require one
    cluster-matched electron (the other is not required to match a cluster). No
    selection on the photon as the sample must be unbiased.

    As this retains a lot of bhabha events (by construction) we allow for prescaling
    (and prefer prescaled rather than a biased sampe by requiring any selection on the
    photon or too much of a cut on the recoil momentum).

    Prescales are given in standard trigger terms (reciprocal), so prescale of 100 is 1%
    of events kept, *etc*.
    """

    __authors__ = ["Sam Cunliffe"]
    __description__ = "Radiative electron pairs for photon systematics"
    __contact__ = __liaison__
    __category__ = "systematics, photon calibration"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)

    def __init__(self, prescale_all=1, prescale_fwd_electron=1, **kwargs):
        """
        Parameters:
            prescale_all (int): the global prescale for this skim
            prescale_fwd_electron (int): the prescale electrons (e-) in
                the forward endcap
            **kwargs: Passed to constructor of `BaseSkim`.
        """
        # Redefine __init__ to allow for additional optional arguments
        super().__init__(**kwargs)
        self.prescale_all = prescale_all
        self.prescale_fwd_electron = prescale_fwd_electron

    def build_lists(self, path):
        # convert prescales from trigger convention
        prescale_all = str(float(1.0 / self.prescale_all))
        prescale_fwd_electron = str(float(1.0 / self.prescale_fwd_electron))

        # require a pair of good electrons one of which must be cluster-matched
        # with 3 GeV of energy
        goodtrack = "abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0"
        goodtrackwithcluster = f"{goodtrack} and clusterE > 3.0"
        ma.cutAndCopyList("e+:skimtight", "e+:all", goodtrackwithcluster, path=path)
        ma.cutAndCopyList("e+:skimloose", "e+:all", goodtrack, path=path)

        # a minimum momentum of 75 MeV/c recoiling against the pair,
        # and require that the recoil is within the CDC acceptance
        recoil = "pRecoil > 0.075 and 0.296706 < pRecoilTheta < 2.61799"  # GeV/c, rad
        ma.reconstructDecay("vpho:radee -> e+:skimtight e-:skimloose", recoil, path=path)

        # apply event cuts (exactly two clean tracks in the event, and prescale
        # the whole event regardless of where the electron went)
        event_cuts = "[nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2]"  # cm, cm
        event_cuts += f" and [eventRandom <= {prescale_all}]"

        # now prescale the *electron* (e-) in the forward endcap (for bhabhas)
        # note this is all done with cut strings to circumnavigate BII-3607
        fwd_encap_border = "0.5480334"  # rad (31.4 deg)
        electron_is_first = "daughter(0, charge) < 0"
        first_in_fwd_endcap = f"daughter(0, theta) < {fwd_encap_border}"
        first_not_in_fwd_endcap = f"daughter(0, theta) > {fwd_encap_border}"
        electron_is_second = "daughter(1, charge) < 0"
        second_in_fwd_endcap = f"daughter(1, theta) < {fwd_encap_border}"
        second_not_in_fwd_endcap = f"daughter(1, theta) > {fwd_encap_border}"
        passes_prescale = f"eventRandom <= {prescale_fwd_electron}"
        #
        # four possible scenarios:
        # 1) electron first in the decaystring and in fwd endcap: prescale these
        prescale_logic = f"[{electron_is_first} and {first_in_fwd_endcap} and {passes_prescale}]"
        # 2) electron second in string and in fwd endcap: prescale these
        prescale_logic += f" or [{electron_is_second} and {second_in_fwd_endcap} and {passes_prescale}]"
        # 3) electron first in string and not in fwd endcap (no prescale)
        prescale_logic += f" or [{electron_is_first} and {first_not_in_fwd_endcap}]"
        # 4) electron second in string and not in fwd endcap (no prescale)
        prescale_logic += f" or [{electron_is_second} and {second_not_in_fwd_endcap}]"

        # final candidate building with cuts and prescales
        prescale_logic = f"[{prescale_logic}]"
        ma.applyCuts("vpho:radee", event_cuts + " and " + prescale_logic, path=path)

        return ["vpho:radee"]


@fancy_skim_header
class SystematicsLambda(BaseSkim):
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato", "Jake Bennett"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "systematics"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdLambdas(path=path)

    def build_lists(self, path):
        vm.addAlias("fsig", "formula(flightDistance/flightDistanceErr)")
        vm.addAlias("pMom", "daughter(0,p)")
        vm.addAlias("piMom", "daughter(1,p)")
        vm.addAlias("daughtersPAsym", "formula((pMom-piMom)/(pMom+piMom))")

        LambdaList = []
        ma.cutAndCopyList("Lambda0:syst0", "Lambda0:merged", "fsig>10 and daughtersPAsym>0.41", path=path)
        LambdaList.append("Lambda0:syst0")

        return LambdaList


@fancy_skim_header
class SystematicsPhiGamma(BaseSkim):
    """
    Uses the ``gamma:loose`` list and a cut on the number of tracks.

    Cuts applied:

    * :math:`E_{\\gamma}> 3\\,\\text{GeV}` AND
    * :math:`E_{\\gamma}< 8\\,\\text{GeV}`
    * :math:`n_{\\text{tracks}} \\geq 2` AND :math:`n_{\\text{tracks}} \\leq 4`
    * at least 1 candidate in the K_S0:merged or in the phi->K+:all K-:all lists
    """
    __authors__ = ["Giuseppe Finocchiaro", "Benjamin Oberhof"]
    __description__ = (
        "Skim for ISR - phi gamma analyses, "
        ":math:`e^+ e^- \\to \\phi \\gamma ` and "
        ":math:`\\phi` decays into two charged tracks "
        "(:math:`K^+K^-` or :math:`K_S K_L` with :math:`K_S\\to \\pi^+\\pi^-`)"
    )
    __contact__ = "Giuseppe Finocchiaro <giuseppe.finocchiaro@lnf.infn.it>"
    __category__ = "systematics"

    ApplyHLTHadronCut = False

    TestSampleProcess = "ccbar"
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdPhotons("loose", path=path)
        stdK("all", path=path)
        stdKshorts(path=path)

    def build_lists(self, path):
        EventCuts = [
            "[nTracks>=2] and [nTracks<=6]",
            "[nParticlesInList(gamma:PhiSystematics) > 0]",
            "[[nParticlesInList(phi:charged) > 0] or [nParticlesInList(K_S0:PhiSystematics) > 0]]"
        ]

        ma.cutAndCopyList("gamma:PhiSystematics", "gamma:loose", "3 < E < 8", path=path)
        ma.reconstructDecay('phi:charged -> K+:all K-:all', '0.9 < M < 1.2', path=path)
        ma.copyList('K_S0:PhiSystematics', 'K_S0:merged', path=path)

        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)
        return ["gamma:PhiSystematics"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        vm.addAlias("gamma_E_CMS", "useCMSFrame(E)")
        vm.addAlias("gamma_E", "E")
        vm.addAlias("K_S0_mass", "M")
        vm.addAlias("phi_mass", "M")

        histoRootFile = f'{self}_Validation.root'
        variableshisto = [('gamma_E', 120, 2.5, 8.5, 'gamma_E', self.__contact__, 'Photon energy', ''),
                          ('gamma_E_CMS', 100, 2.0, 7.0, 'gamma_E_CMS', self.__contact__, 'Photon energy in CMS', ''),
                          ('nTracks', 15, 0, 15, 'nTracks', self.__contact__, 'Number of tracks', ''),
                          ]
        variableshistoKS = [('K_S0_mass', 200, 0.4, 0.6, 'K_S0_mass', self.__contact__, 'Invariant KS0 mass', ''),
                            ]
        variableshistoPhi = [('phi_mass', 200, 0.8, 1.2, 'phi_mass', self.__contact__, 'Invariant phi mass', ''),
                             ]

        create_validation_histograms(path, histoRootFile, 'gamma:PhiSystematics', variableshisto)
        create_validation_histograms(path, histoRootFile, 'K_S0:merged', variableshistoKS)
        create_validation_histograms(path, histoRootFile, 'phi:charged', variableshistoPhi)


@fancy_skim_header
class Random(BaseSkim):
    __authors__ = "Phil Grace"
    __contact__ = "Phil Grace <philip.grace@adelaide.edu.au>"
    __description__ = "Random skim to select a fixed fraction of events."
    __category__ = "systematics, random"

    ApplyHLTHadronCut = False

    def __init__(self, KeepPercentage=10, seed=None, **kwargs):
        """
        Parameters:
            KeepPercentage (float): Percentage of events to be kept.
            seed (int): Set random seed to given number. If this argument is not given,
                this skim will not alter the random seed.
            **kwargs: Passed to constructor of `BaseSkim`.
        """
        super().__init__(**kwargs)
        self.KeepPercentage = KeepPercentage
        self.seed = seed

    def additional_setup(self, path):
        if self.seed is not None:
            b2.set_random_seed(int(self.seed))

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):
        # Select one photon/track per event with no other cuts, so that all events are
        # captured in the skim list if KeepPercentage=100.
        label = "RandomSkim"
        ma.copyList(f"pi+:{label}", "pi+:all", path=path)
        ma.copyList(f"gamma:{label}", "gamma:all", path=path)
        ma.applyRandomCandidateSelection(f"pi+:{label}", path=path)
        ma.applyRandomCandidateSelection(f"gamma:{label}", path=path)

        # Select fraction of events
        path = self.skim_event_cuts(
            f"eventRandom <= {self.KeepPercentage/100}", path=path
        )

        return [f"pi+:{label}", f"gamma:{label}"]


@fancy_skim_header
class SystematicsFourLeptonFromHLTFlag(BaseSkim):
    __authors__ = "Marcel Hohmann"
    __contact__ = __liaison_leptonID__
    __description__ = "Skim to select all events that pass the HLT Four Lepton skim for lepton ID studies"
    __category__ = "systematics, leptonID"
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdPi("all", path=path)

    def build_lists(self, path):
        label = "FourLeptonHLT"
        ma.copyList(f"pi+:{label}", "pi+:all", path=path)
        ma.rankByLowest(f"pi+:{label}", "random", 1, "systematicsFourLeptonHLT_randomRank", path=path)

        path = self.skim_event_cuts(
            "SoftwareTriggerResult(software_trigger_cut&skim&accept_fourlep) == 1", path=path
        )

        return [f"pi+:{label}"]


@fancy_skim_header
class SystematicsRadMuMuFromHLTFlag(BaseSkim):
    __authors__ = "Marcel Hohmann"
    __contact__ = __liaison_leptonID__
    __description__ = "Skim to select all events that pass the HLT RadMuMu skim for lepton ID studies"
    __category__ = "systematics, leptonID"
    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdPi("all", path=path)

    def build_lists(self, path):
        label = "RadMuMuLeptonID"
        ma.copyList(f"pi+:{label}", "pi+:all", path=path)
        ma.rankByLowest(f"pi+:{label}", "random", 1, "systematicsRadMuMuLeptonID_randomRank", path=path)

        path = self.skim_event_cuts(
            "SoftwareTriggerResult(software_trigger_cut&skim&accept_radmumu) == 1", path=path
        )
        return [f"pi+:{label}"]


@fancy_skim_header
class SystematicsJpsi(BaseSkim):
    """
    J/psi skim for lepton ID systematics studies. Lists in this skim are those defined in `JpsimumuTagProbe`, `JpsieeTagProbe`.
    """
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato", "Racha Cheaib", "Marcel Hohmann"]
    __description__ = ""
    __contact__ = __liaison_leptonID__
    __category__ = "systematics, leptonID"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdMu("all", path=path)
        stdE("all", path=path)
        stdPhotons("all", path=path)

    TestSampleProcess = "ccbar"
    ApplyHLTHadronCut = True

    def build_lists(self, path):
        return [
            self.JpsimumuTagProbe(path),
            self.JpsieeTagProbe(path),
        ]

    def JpsimumuTagProbe(self, path):
        """Build JpsimumuTagProbe lists for systematics skims."""
        Cuts = "2.7 < M < 3.4"
        ma.reconstructDecay(
            "J/psi:systematics_mumu -> mu+:all mu-:all",
            f'{Cuts} and [daughter(0,muonID)>0.1 or daughter(1,muonID)>0.1]',
            path=path)
        return "J/psi:systematics_mumu"

    def JpsieeTagProbe(self, path):
        """Build JpsieeTagProbe lists for systematics skims."""

        Cuts = "2.7 < M < 3.4"
        ma.cutAndCopyList('gamma:brems', 'gamma:all', 'E<1', path=path)
        ma.correctBrems('e+:brems_corrected', 'e+:all', 'gamma:brems', path=path)
        ma.reconstructDecay(
            "J/psi:systematics_ee -> e+:brems_corrected e-:brems_corrected",
            f'{Cuts} and [daughter(0,electronID_noTOP)>0.1 or daughter(1,electronID_noTOP)>0.1]',
            path=path)
        return "J/psi:systematics_ee"


@fancy_skim_header
class SystematicsKshort(BaseSkim):
    """
      K-short skim for hadron and lepton ID systematics studies.
      As K-short candidates are abundant this skim has a high retention.
      To meet the retention criteria a prescale is added. The prescale is given in standard trigger terms (reciprocal).
      A prescale of 50 will keep 2% of events, etc.
    """
    __authors__ = ["Marcel Hohmann"]
    __description__ = "Skim for K-short events for performance studies"
    __contact__ = __liaison_leptonID__
    __category__ = "performance, leptonID"

    ApplyHLTHadronCut = True

    def __init__(self, prescale=4, **kwargs):
        """
        Parameters:
            prescale (int): the global prescale for this skim.
            **kwargs: Passed to constructor of `BaseSkim`.
        """
        self.prescale = prescale
        super().__init__(**kwargs)

    def load_standard_lists(self, path):
        stdPi("all", path=path)

    def build_lists(self, path):

        ma.reconstructDecay(
            'K_S0:reco -> pi+:all pi-:all',
            '[0.30 < M < 0.70]',
            path=path)

        vertex.treeFit('K_S0:reco', 0.0, path=path)
        ma.applyCuts('K_S0:reco', '0.4 < M < 0.6', path=path)

        ma.fillParticleList('K_S0:v0 -> pi+ pi-',
                            '[0.30 < M < 0.70]',
                            True,
                            path=path)
        vertex.treeFit('K_S0:v0', 0.0, path=path)
        ma.applyCuts('K_S0:v0', '0.4 < M < 0.6', path=path)

        ma.mergeListsWithBestDuplicate('K_S0:merged', ['K_S0:v0', 'K_S0:reco'],
                                       variable='particleSource', preferLowest=True, path=path)

        KS_cut = '[[cosAngleBetweenMomentumAndVertexVector>0.998] or '\
            ' [formula(flightDistance/flightDistanceErr)>11] or '\
            ' [flightTime>0.007]]'  # and '\
        # '[useAlternativeDaughterHypothesis(M, 0:p+) > 1.13068 and useAlternativeDaughterHypothesis(M, 0:pi-, 1:p+) > 1.13068]'

        ma.cutAndCopyList("K_S0:skim", "K_S0:merged", KS_cut, path=path)
        path = self.skim_event_cuts(f'eventRandom < {(1/self.prescale):.6f}', path=path)
        return ['K_S0:skim']


@fancy_skim_header
class SystematicsBhabha(BaseSkim):
    """
    Skim for selecting Bhabha events for leptonID studies.
    In case the retention exceeds 10% a prescale can be added.
    The prescale is given in standard trigger terms (reciprocal).
    """
    __authors__ = ["Justin Skorupa"]
    __description__ = "Skim for Bhabha events for lepton ID study"
    __contact__ = __liaison_leptonID__
    __category__ = "performance, leptonID"

    ApplyHLTHadronCut = False

    def __init__(self, prescale=1, **kwargs):
        """
        Parameters:
            prescale (int): the global prescale for this skim.
            **kwargs: Passed to constructor of `BaseSkim`.
        """
        self.prescale = prescale
        super().__init__(**kwargs)

    def load_standard_lists(self, path):
        stdE("all", path=path)

    def build_lists(self, path):
        goodtrack = "abs(dz) < 5 and abs(dr) < 2"
        goodtrackwithPID = f"{goodtrack} and electronID_noTOP > 0.95 and clusterTheta > 0.59"\
            " and clusterTheta < 2.15 and useCMSFrame(clusterE) > 2"
        ma.cutAndCopyList("e+:tight", "e+:all", goodtrackwithPID, path=path)
        ma.cutAndCopyList("e+:loose", "e+:all", goodtrack, path=path)

        ma.reconstructDecay(
            "vpho:bhabha -> e+:tight e-:loose", "", path=path)

        event_cuts = "[nCleanedTracks(abs(dz) < 5 and abs(dr) < 2) == 2]"\
                     f" and eventRandom < {(1/self.prescale):.6f}"

        ma.applyCuts("vpho:bhabha", event_cuts, path=path)

        return ["vpho:bhabha"]


@fancy_skim_header
class SystematicsCombinedHadronic(CombinedSkim):
    """
      Combined systematics skim for the four hadronic channels:
            SystematicsKshort,
            SystematicsJpsi,
            SystematicsDstar,
            SystemmaticsLambda.

      This is required for  technical (data production) reasons, as it keeps the number of files low.
      See the definitions of the individual skims for the details.
    """
    __authors__ = ["Marcel Hohmann"]
    __description__ = "Combined Skim of the systematic hadronic skims: Kshort, Jpsi, Dstar, Lambda."
    __contact__ = __liaison_leptonID__
    __category__ = "performance, leptonID"
    __name__ = "SystematicsCombinedHadronic"

    produces_mdst_by_default = True

    def __init__(self, prescale_kshort=4, mdstOutput=True, **kwargs):
        """ Initialiser.

        Args:
            prescale_kshort (Optional[int]): offline prescale factor for KS skim.
            **kwargs: key-worded arguments. See CombinedSkim.__init__()
        """

        kwargs.update(mdstOutput=mdstOutput, CombinedSkimName=self.__name__)
        kwargs.setdefault('udstOutput', False)

        skims_list = [SystematicsKshort(prescale=prescale_kshort), SystematicsDstar(), SystematicsLambda(), SystematicsJpsi()]
        super().__init__(*skims_list, **kwargs)


@fancy_skim_header
class SystematicsCombinedLowMulti(CombinedSkim):
    """
      Combined systematics skim for the four low multi channels:
          SystematicsFourLeptonFromHLTFlag,
          SystematicsRadmumuFromHLTFlag,
          SystematicsBhabha,
          TauThrust.

      This is required for  technical (data production) reasons, as it keeps the number of files low.
      See the definitions of the individual skims for the details.
    """
    __authors__ = ["Marcel Hohmann"]
    __description__ = "Combined Skim of the systematic low multi skims: FourLepton, Radmumu, Bhabha, TauThrust."
    __contact__ = __liaison_leptonID__
    __category__ = "performance, leptonID"
    __name__ = "SystematicsCombinedLowMulti"

    produces_mdst_by_default = True

    def __init__(self, mdstOutput=True, **kwargs):
        """ Initialiser.

        Args:
            **kwargs: key-worded arguments. See CombinedSkim.__init__()
        """

        kwargs.update(mdstOutput=mdstOutput, CombinedSkimName=self.__name__)
        kwargs.setdefault('udstOutput', False)

        from skim.WGs.taupair import TauThrust
        skims_list = [SystematicsFourLeptonFromHLTFlag(), SystematicsRadMuMuFromHLTFlag(), SystematicsBhabha(), TauThrust()]
        super().__init__(*skims_list, **kwargs)
