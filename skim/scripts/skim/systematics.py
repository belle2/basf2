#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for systematics studies """

__authors__ = [
    "Sam Cunliffe",
    "Torben Ferber",
    "Ilya Komarov",
    "Yuji Kato"
]

import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header, get_test_file
import vertex


# TODO: Add liaison name and email address
__liaison__ = ""


@fancy_skim_header
class Systematics(BaseSkim):
    """
    Lists in this skim are those defined in `JpsimumuTagProbe`, `JpsieeTagProbe`, and
    `PiKFromDstarList`.
    """
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato", "Racha Cheaib"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "systematics"

    RequiredStandardLists = {
        "stdCharged": {
            "stdK": ["all"],
            "stdPi": ["all"],
        },
    }

    TestFiles = [get_test_file("MC13_ccbarBGx1")]

    def build_lists(self, path):
        lists = [
            # self.JpsimumuTagProbe(path),
            # self.JpsieeTagProbe(path),
            self.PiKFromDstarList(path),
        ]

        # Flatten the list of lists
        self.SkimLists = [s for l in lists for s in l]

    def JpsimumuTagProbe(self, path):
        """Build JpsimumuTagProbe lists for systematics skims."""
        #   Cuts = "2.8 < M < 3.4"
        Cuts = "2.7 < M < 3.4 and useCMSFrame(p) < 2.0"
        Channel = "mu+:all mu-:loose"
        jpsiList = []
        chID = 0
        ma.reconstructDecay("J/psi:mumutagprobe" + str(chID) + " -> " + Channel, Cuts, chID, path=path)
        jpsiList.append("J/psi:mumutagprobe" + str(chID))
        ma.matchMCTruth("J/psi:mumutagprobe0", path=path)
        return jpsiList

    def JpsieeTagProbe(self, path):
        """Build JpsieeTagProbe lists for systematics skims."""
        #   Cuts = "2.7 < M < 3.4"
        Cuts = "2.7 < M < 3.4 and useCMSFrame(p) < 2.0"
        Channel = "e+:all e-:loose"
        jpsiList = []
        chID = 0
        ma.reconstructDecay("J/psi:eetagprobe" + str(chID) + " -> " + Channel, Cuts, chID, path=path)
        jpsiList.append("J/psi:eetagprobe" + str(chID))
        ma.matchMCTruth("J/psi:eetagprobe0", path=path)
        return jpsiList

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

    RequiredStandardLists = {
        "stdCharged": {
            "stdK": ["loose"],
            "stdPi": ["loose"],
        },
        "stdPi0s": {
            "stdPi0s": ["eff40_Jan2020"]
        }
    }

    def build_lists(self, path):
        lists = [
            self.BtoDStarPiList(path),
            self.DstarToD0PiPartList(path)
        ]

        # Flatten the list of lists
        self.SkimLists = [s for l in lists for s in l]

    def BtoDStarPiList(self, path):
        """Build BtoDStarPiList lists for systematics skims."""
        D0Cuts = "1.835 < M < 1.895"
        DstarCuts = "massDifference(0)<0.16"
        B0Cuts = "Mbc > 5.2 and abs(deltaE) < 0.3"

        # D0
        D0Channel = ["K+:loose pi-:loose", "K+:loose pi-:loose pi-:loose pi+:loose", "K+:loose pi-:loose pi0:eff40_Jan2020"]

        D0List = []
        for chID, channel in enumerate(D0Channel):
            resonanceName = "anti-D0:loose" + str(chID)
            ma.reconstructDecay(resonanceName + " -> " + channel, D0Cuts, chID, path=path)
            # vertex.raveFit(resonanceName, 0.0, path=path)
            ma.matchMCTruth(resonanceName, path=path)
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
            ma.matchMCTruth(resonanceName, path=path)

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
            ma.matchMCTruth(resonanceName, path=path)

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
            ma.matchMCTruth(resonanceName, path=path)

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

    RequiredStandardLists = {
        "stdCharged": {
            "stdK": ["loose"],
            "stdMu": ["loose"],
            "stdPi": ["loose"],
            "stdPr": ["loose"],
        },
        "stdPi0s": {
            "stdPi0s": ["eff40_Jan2020Fit"]
        }
    }

    def build_lists(self, path):
        lists = [
            self.getDsList(path),
            self.getDstarList(path),
            self.getSigmacList(path),
            self.getmumugList(path),
            self.getBZeroList(path),
            self.getBPlusList(path),
        ]

        # Flatten the list of lists
        self.SkimLists = [s for l in lists for s in l]

    def getDsList(self, path):
        """Build Ds list for systematics skims."""
        DsCuts = "1.90 < M < 2.04"

        ma.reconstructDecay("phi:res -> K+:loose K-:loose", "1.01 < M < 1.03", path=path)
        ma.reconstructDecay("K*0:res -> K+:loose pi-:loose", "0.7 < M < 1.1", path=path)

        DsChannel = ["phi:res pi+:loose"]
        DsList = []
        for chID, channel in enumerate(DsChannel):
            particlename = "D_s+:Resonance%d" % (chID)
            ma.reconstructDecay(particlename + " -> " + channel, DsCuts, chID, path=path)
            ma.matchMCTruth(particlename, path)
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
            DstarChannel.append(channel + " pi0:eff40_Jan2020")

        DstarList = []
        for chID, channel in enumerate(DstarChannel):
            ma.reconstructDecay("D*+:resonance" + str(chID) + " -> " + channel, DstarCuts, chID, path=path)
            DstarList.append("D*+:resonance" + str(chID))
            ma.matchMCTruth("D*+:resonance0", path=path)

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
            ma.matchMCTruth("Sigma_c++:resonance0", path=path)

        # Sigma_c0
        Sigmac0Channel = []
        for channel in LambdacList:
            Sigmac0Channel.append(channel + " pi-:loose")

        Sigmac0List = []
        for chID, channel in enumerate(Sigmac0Channel):
            ma.reconstructDecay("Sigma_c0:resonance" + str(chID) + " -> " + channel, SigmacCuts, chID, path=path)
            Sigmac0List.append("Sigma_c0:resonance" + str(chID))
            ma.matchMCTruth("Sigma_c0:resonance0", path=path)

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
            ma.matchMCTruth(resonanceName, path=path)
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
            ma.matchMCTruth(resonanceName, path=path)

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
            ma.matchMCTruth("B+:resonance" + str(chID), path=path)

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

    RequiredStandardLists = {
        "stdCharged": {
            "stdMu": ["all"],
        },
    }

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

        self.SkimLists = radmumulist


@fancy_skim_header
class SystematicsEELL(BaseSkim):
    __authors__ = ["Ilya Komarov"]
    __description__ = "Systematics skim of :math:`ee\\to ee\\ell\\ell`"
    __contact__ = __liaison__
    __category__ = "systematics, lepton ID"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
        },
    }

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

        self.SkimLists = eelllist


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

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
        },
    }

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
        goodtrackwithcluster = "%s and clusterE > 3.0" % goodtrack
        ma.cutAndCopyList("e+:skimtight", "e+:all", goodtrackwithcluster, path=path)
        ma.cutAndCopyList("e+:skimloose", "e+:all", goodtrack, path=path)

        # a minimum momentum of 75 MeV/c recoiling against the pair,
        # and require that the recoil is within the CDC acceptance
        recoil = "pRecoil > 0.075 and 0.296706 < pRecoilTheta < 2.61799"  # GeV/c, rad
        ma.reconstructDecay("vpho:radee -> e+:skimtight e-:skimloose", recoil, path=path)

        # apply event cuts (exactly two clean tracks in the event, and prescale
        # the whole event regardless of where the electron went)
        event_cuts = "[nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2]"  # cm, cm
        event_cuts += " and [eventRandom <= %s]" % prescale_all

        # now prescale the *electron* (e-) in the forward endcap (for bhabhas)
        # note this is all done with cut strings to circumnavigate BII-3607
        fwd_encap_border = "0.5480334"  # rad (31.4 deg)
        electron_is_first = "daughter(0, charge) < 0"
        first_in_fwd_endcap = "daughter(0, theta) < %s" % fwd_encap_border
        first_not_in_fwd_endcap = "daughter(0, theta) > %s" % fwd_encap_border
        electron_is_second = "daughter(1, charge) < 0"
        second_in_fwd_endcap = "daughter(1, theta) < %s" % fwd_encap_border
        second_not_in_fwd_endcap = "daughter(1, theta) > %s" % fwd_encap_border
        passes_prescale = "eventRandom <= %s" % prescale_fwd_electron
        #
        # four possible scenarios:
        # 1) electron first in the decaystring and in fwd endcap: prescale these
        prescale_logic = "[%s and %s and %s]" \
            % (electron_is_first, first_in_fwd_endcap, passes_prescale)
        # 2) electron second in string and in fwd endcap: prescale these
        prescale_logic += " or [%s and %s and %s]" \
            % (electron_is_second, second_in_fwd_endcap, passes_prescale)
        # 3) electron first in string and not in fwd endcap (no prescale)
        prescale_logic += " or [%s and %s]" % (electron_is_first, first_not_in_fwd_endcap)
        # 4) electron second in string and not in fwd endcap (no prescale)
        prescale_logic += " or [%s and %s]" % (electron_is_second, second_not_in_fwd_endcap)

        # final candidate building with cuts and prescales
        prescale_logic = "[%s]" % prescale_logic
        ma.applyCuts("vpho:radee", event_cuts + " and " + prescale_logic, path=path)

        self.SkimLists = ["vpho:radee"]


@fancy_skim_header
class SystematicsLambda(BaseSkim):
    __authors__ = ["Sam Cunliffe", "Torben Ferber", "Ilya Komarov", "Yuji Kato"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "systematics"

    RequiredStandardLists = None

    def build_lists(self, path):
        LambdaCuts = "M < 1.2"

        ma.fillParticleList("p+:SystematicsLambda", "", enforceFitHypothesis=True, path=path)
        ma.fillParticleList("pi-:SystematicsLambda", "", enforceFitHypothesis=True, path=path)
        LambdaChannel = ["p+:SystematicsLambda pi-:SystematicsLambda"]

        LambdaList = []
        for chID, channel in enumerate(LambdaChannel):
            ma.reconstructDecay("Lambda0:syst" + str(chID) + " -> " + channel, LambdaCuts, chID, path=path)
            vertex.kFit("Lambda0:syst" + str(chID), 0.002, path=path)
            ma.applyCuts("Lambda0:syst" + str(chID), "1.10<M<1.13", path=path)
            ma.applyCuts("Lambda0:syst" + str(chID), "formula(x*x+y*y)>0.0225", path=path)
            ma.applyCuts("Lambda0:syst" + str(chID), "formula(x*px+y*py)>0", path=path)
            ma.applyCuts(
                "Lambda0:syst" +
                str(chID),
                "formula([x*px*x*px+2*x*px*y*py+y*py*y*py]/[[px*px+py*py]*[x*x+y*y]])>0.994009",
                path=path)
            ma.applyCuts("Lambda0:syst" + str(chID), "p>0.2", path=path)
            ma.matchMCTruth("Lambda0:syst0", path=path)
            LambdaList.append("Lambda0:syst" + str(chID))

        self.SkimLists = LambdaList
