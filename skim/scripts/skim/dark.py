#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the dark sector physics working group """

__authors__ = [
    "Sam Cunliffe",
    "Michael De Nuccio",
    "Ilya Komarov",
    "Giacomo De Pietro",
    "Miho Wakai",
    "Savino Longo"
]


import basf2 as b2
import modularAnalysis as ma
import pdg
from skimExpertFunctions import BaseSkim, fancy_skim_header, get_test_file
from stdCharged import stdE, stdMu
from stdPhotons import stdPhotons
import vertex as vertex

__liaison__ = "Sascha Dreyer <sascha.dreyer@desy.de>"
__validation_sample___ = "mdst14.root"


@fancy_skim_header
class SinglePhotonDark(BaseSkim):
    """
    **Physics channel**: ee → A'γ; A' → invisible

    Skim list contains single photon candidates for the dark photon to invisible final
    state analysis.
    """
    __authors__ = ["Sam Cunliffe", "Chris Hearty"]
    __contact__ = __liaison__
    __description__ = "Single photon skim list for the dark photon analysis."
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)

    def build_lists(self, path):

        # start with all photons with E* above 500 MeV in the tracking acceptance
        in_tracking_acceptance = "0.296706 < theta < 2.61799"  # rad = [17, 150] degrees
        ma.cutAndCopyList(
            "gamma:singlePhoton", "gamma:all",
            f"useCMSFrame(E) > 0.5 and {in_tracking_acceptance}", path=path)

        # require a region-dependent minimum energy of the candidate, we have
        # a new 0.5 GeV trigger in the inner barrel: [44.2, 94.8] degrees @ L1
        region_dependent = " [clusterTheta < 1.65457213 and clusterTheta > 0.77143553] or "
        region_dependent += "[clusterReg ==  2 and useCMSFrame(E) > 1.0] or "  # barrel
        region_dependent += "[clusterReg ==  1 and useCMSFrame(E) > 2.0] or "  # fwd
        region_dependent += "[clusterReg ==  3 and useCMSFrame(E) > 2.0] or "  # bwd
        region_dependent += "[clusterReg == 11 and useCMSFrame(E) > 2.0] or "  # between fwd and barrel
        region_dependent += "[clusterReg == 13 and useCMSFrame(E) > 2.0]"      # between bwd and barrel
        ma.applyCuts("gamma:singlePhoton", region_dependent, path=path)

        # require only one single photon candidate and no good tracks in the event
        good_tracks = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15'  # cm, cm, GeV/c
        path = self.skim_event_cuts(
            f"nParticlesInList(gamma:singlePhoton) == 1 and nCleanedTracks({good_tracks}) == 0",
            path=path
        )

        # check the second-most-energetic photon (above 550 MeV is unlikely to
        # be beam-induced background) and veto if it's in time with our signal
        # candidate -- do after the event cuts since it uses a ParticleCombiner
        # and should not be done for all events (save event-processing time)
        not_in_signal_list = "isInList(gamma:singlePhoton) < 1"
        in_time = "maxWeightedDistanceFromAverageECLTime < 1"
        ma.cutAndCopyList("gamma:to_veto", "gamma:all",
                          f"E > 0.55 and {not_in_signal_list}", path=path)
        ma.rankByHighest("gamma:to_veto", "E", numBest=1, path=path)
        ma.reconstructDecay("vpho:veto -> gamma:singlePhoton gamma:to_veto",
                            in_time, path=path)
        veto_additional_in_time_cluster = 'nParticlesInList(vpho:veto) < 1'

        # final signal selection must pass the 'in-time' veto on the
        # second-most-energetic cluster -- this is also an event cut, but apply
        # to the list (which is anyway a maximum one candidate per event) since
        # we can only call skim_event_cuts once
        ma.applyCuts("gamma:singlePhoton", veto_additional_in_time_cluster, path=path)
        self.SkimLists = ["gamma:singlePhoton"]


@fancy_skim_header
class ALP3Gamma(BaseSkim):
    __authors__ = ["Michael De Nuccio"]
    __description__ = (
        "Neutral dark sector skim list for the ALP 3-photon analysis: "
        ":math:`ee\\to a(\\to\\gamma\\gamma)\\gamma`"
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector"

    def addALPToPDG(self):
        """Adds the ALP codes to the basf2 pdg instance """
        pdg.add_particle('beam', 55, 999., 999., 0, 0)
        pdg.add_particle('ALP', 9000006, 999., 999., 0, 0)

    def initialALP(self, path):
        """
        An list builder function for the ALP decays. Part of the `ALP3Gamma` skim.

        Parameters:
            path (basf2.Path): the path to add the skim

        Returns:
            list name of the ALP decays candidates
        """
        # no cuts applied on ALP
        ALPcuts = ''

        # applying a lab frame energy cut to the daughter photons
        ma.fillParticleList(
            'gamma:cdcAndMinimumEnergy',
            'E >= 0.1 and theta >= 0.297 and theta <= 2.618',
            True, path=path
        )

        # defining the decay string
        ALPchannels = ['gamma:cdcAndMinimumEnergy  gamma:cdcAndMinimumEnergy']
        ALPList = []

        # creating an ALP from the daughter photons
        for chID, channel in enumerate(ALPchannels):
            mode = 'ALP:' + str(chID) + ' -> ' + channel
            print(mode)
            ma.reconstructDecay(mode, ALPcuts, chID, path=path)

            ALPList.append('ALP:' + str(chID))

        Lists = ALPList
        return Lists

    def additional_setup(self, path):
        self.addALPToPDG()

    def build_lists(self, path):
        # applying invariant mass cut on the beam particle
        beamcuts = "InvM >= formula(0.8 * Ecms) and InvM <= formula(1.05 * Ecms) and maxWeightedDistanceFromAverageECLTime <= 2"

        ALPList = self.initialALP(path=path)

        # applying a lab frame energy cut to the recoil photon
        ma.fillParticleList("gamma:minimumEnergy", "E >= 0.1", True, path=path)
        beamList = []

        # reconstructing decay using the reconstructed ALP
        # from previous function and adding the recoil photon
        for chID, channel in enumerate(ALPList):
            mode = "beam:" + str(chID) + " -> gamma:minimumEnergy " + channel
            print(mode)
            ma.reconstructDecay(mode, beamcuts, chID, path=path)
            beamList.append("beam:" + str(chID))
        self.SkimLists = beamList


@fancy_skim_header
class DimuonPlusMissingEnergy(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-} \\, +` missing energy.
    """
    __authors__ = ["Giacomo De Pietro"]
    __description__ = (
        "Dimuon + missing energy skim, needed for :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-}"
        "Z^{\\prime}; \\, Z^{\\prime} \\to \\mathrm{invisible}` and other searches."
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdMu("all", path=path)

    def build_lists(self, path):
        dimuon_list = []
        skim_label = "forDimuonMissingEnergySkim"
        dimuon_name = "Z0:" + skim_label

        # Define some cuts
        fromIP_cut = "[abs(dz) < 5.0] and [abs(dr) < 2.0]"
        muonID_cut = "[muonID > 0.3]"
        # We want exactly 2 tracks from IP
        dimuon_cut = "[nCleanedTracks(" + fromIP_cut + ") < 4]"
        # And the pair must have pt > 200 MeV in CMS frame
        dimuon_cut += " and [useCMSFrame(pt) > 0.2]"

        # Reconstruct the dimuon candidate
        ma.cutAndCopyList("mu+:" + skim_label, "mu+:all", fromIP_cut + " and " + muonID_cut, path=path)
        ma.reconstructDecay(dimuon_name + " -> mu+:" + skim_label + " mu-:" + skim_label, dimuon_cut, path=path)

        # And return the dimuon list
        dimuon_list.append(dimuon_name)
        self.SkimLists = dimuon_list


@fancy_skim_header
class ElectronMuonPlusMissingEnergy(BaseSkim):
    __authors__ = ["Giacomo De Pietro"]
    __description__ = (
        "Electron-muon pair + missing energy skim, needed for :math:`e^{+}e^{-} \\to "
        "e^{\\pm}\\mu^{\\mp} Z^{\\prime}; \\, Z^{\\prime} \\to \\mathrm{invisible}` and other "
        "searches."
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        """
        **Physics channel**: :math:`e^{+}e^{-} \\to e^{\\pm}\\mu^{\\mp} \\, +` missing energy
        """
        emu_list = []
        skim_label = "forElectronMuonMissingEnergySkim"
        emu_name = "Z0:" + skim_label

        # Define some basic cuts
        fromIP_cut = "[abs(dz) < 5.0] and [abs(dr) < 2.0]"
        electronID_cut = "[electronID > 0.3]"
        muonID_cut = "[muonID > 0.3]"
        # We require that the electron points to the barrel ECL + 10 degrees
        theta_cut = "[0.387 < theta < 2.421]"
        # We want exactly 2 tracks from IP
        emu_cut = "[nCleanedTracks(" + fromIP_cut + ") < 4]"
        # And the pair must have pt > 200 MeV in CMS frame
        emu_cut += " and [useCMSFrame(pt) > 0.2]"

        # Reconstruct the dimuon candidate
        ma.cutAndCopyList("e+:" + skim_label, "e+:all", fromIP_cut + " and " + electronID_cut + " and " + theta_cut, path=path)
        ma.cutAndCopyList("mu+:" + skim_label, "mu+:all", fromIP_cut + " and " + muonID_cut, path=path)
        ma.reconstructDecay(emu_name + " -> e+:" + skim_label + " mu-:" + skim_label, emu_cut, path=path)

        # And return the dimuon list
        emu_list.append(emu_name)
        self.SkimLists = emu_list


@fancy_skim_header
class LFVZpVisible(BaseSkim):
    __authors__ = ["Ilya Komarov"]
    __description__ = "Lepton flavour violating Z' skim, Z' to visible FS."
    __contact__ = __liaison__
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdE("loose", path=path)

    def build_lists(self, path):
        """
        **Physics channel**: ee --> e mu Z'; Z' --> e mu
        """
        lfvzp_list = []

        # Here we just want four gpood tracks to be reconstructed
        track_cuts = "abs(dz) < 2.0 and abs(dr) < 0.5"
        Event_cuts_vis = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 4"

        ma.cutAndCopyList("e+:lfvzp", "e+:all", track_cuts, path=path)

        # Z' to lfv: fully reconstructed
        LFVZpVisChannel = "e+:lfvzp e+:lfvzp e-:lfvzp e-:lfvzp"

        ma.reconstructDecay("vpho:vislfvzp -> " + LFVZpVisChannel, Event_cuts_vis, path=path)

        lfvzp_list.append("vpho:vislfvzp")

        # Z' to lfv: part reco
        LFVZpVisChannel = "e+:lfvzp e+:lfvzp e-:lfvzp"
        Event_cuts_vis = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 3"

        ma.reconstructDecay("vpho:3tr_vislfvzp -> " + LFVZpVisChannel, Event_cuts_vis, path=path, allowChargeViolation=True)

        lfvzp_list.append("vpho:3tr_vislfvzp")

        # Z' to lfv: two same-sign tracks
        LFVZpVisChannel = "e+:lfvzp e+:lfvzp"
        Event_cuts_vis = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2"
        ma.reconstructDecay("vpho:2tr_vislfvzp -> " + LFVZpVisChannel, Event_cuts_vis, path=path, allowChargeViolation=True)

        lfvzp_list.append("vpho:2tr_vislfvzp")

        self.SkimLists = lfvzp_list


@fancy_skim_header
class EGammaControlDark(BaseSkim):
    """
    **Physics channel**: ee → eγ
    """

    __authors__ = ["Sam Cunliffe", "Torben Ferber"]
    __description__ = (
        "Electron-gamma skim list for study of the ee backgrounds at high dark "
        "photon mass, as part of the dark photon analysis"
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector, control-channel"

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)
        stdE("all", path=path)

    def build_lists(self, path):

        # long-winded names for the particle lists to avoid clash
        internal_skim_label = "forEGammaSkim"
        skim_output_label = "EGammaControl"

        # want exactly 1 good quality track in the event
        # (not one good electron, one good anything)
        phys_perf_good_track = 'abs(dr) < 1 and abs(dz) < 3 and pt > 0.15'  # cm, cm, GeV/c
        one_good_track = f'[nCleanedTracks({phys_perf_good_track}) == 1]'

        # exactly 1 good photon in the event
        photon_energy_cut = '0.45'
        good_photon = 'theta > 0.296706 and theta < 2.61799' +\
            f' and useCMSFrame(E) > {photon_energy_cut}'
        ma.cutAndCopyList(f'gamma:{internal_skim_label}', 'gamma:all', good_photon, path=path)
        one_good_photon = f'[eventCached(nParticlesInList(gamma:{internal_skim_label})) == 1]'

        # apply the event-level cuts
        event_cuts = f'{one_good_photon} and {one_good_track}'
        path = self.skim_event_cuts(event_cuts, path=path)

        # fill electron lists (tighter than previous selection)
        good_track_w_hie_cluster_match = '%s and clusterE > 2.0' % phys_perf_good_track
        ma.cutAndCopyList(f'e+:{internal_skim_label}', 'e+:all', good_track_w_hie_cluster_match, path=path)

        # reconstruct decay
        ma.reconstructDecay(
            f'vpho:{skim_output_label} -> e+:{internal_skim_label} gamma:{internal_skim_label}',
            '', 1, allowChargeViolation=True, path=path)
        self.SkimLists = [f"vpho:{skim_output_label}"]


@fancy_skim_header
class GammaGammaControlKLMDark(BaseSkim):
    """
    **Physics channel**: ee → γγ

    .. Note::
        This skim can retain a lot of γγ events.
        In case this becomes unacceptable, we provide prescale parameters.
        Prescales are given in standard trigger convention (reciprocal),
        so prescale of 100 is 1% of events kept, etc.

    .. Tip::
        To prescale the higher-energy probe photons by 10%:

        >>> from skim.dark import GammaGammaControlKLMDark
        >>> Skim = GammaGammaControlKLMDark(prescale_high=10)
        >>> Skim(path)  # Add list-building function and uDST output module to path
        >>> b2.process(path)
    """

    __authors__ = ["Sam Cunliffe", "Miho Wakai"]
    __description__ = (
        "Gamma gamma skim list for study of the KLM efficiency as part of "
        "the dark photon analysis"
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector, control-channel"

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)

    TestFiles = [get_test_file("MC13_ggBGx1")]

    def __init__(self, prescale_high=1, prescale_low=1, **kwargs):
        """
        Parameters:
            prescale_high (int): the prescale for more energetic probe photon
            prescale_low (int): the prescale for a less energetic probe photon
            **kwargs: Passed to constructor of `BaseSkim`.
        """
        # Redefine __init__ to allow for additional optional arguments
        super().__init__(**kwargs)
        self.prescale_high = prescale_high
        self.prescale_low = prescale_low

    def build_lists(self, path):
        # unpack prescales and convert from trigger convention to a number we can
        # compare with a float
        prescale_high, prescale_low = self.prescale_high, self.prescale_low
        if (prescale_high, prescale_low) != (1, 1):
            b2.B2INFO(
                "GammaGammaControlKLMDarkList is prescaled. "
                f"prescale_high={prescale_high}, prescale_low={prescale_low}"
            )
        prescale_high = str(float(1.0 / prescale_high))
        prescale_low = str(float(1.0 / prescale_low))

        # no good (IP-originating) tracks in the event
        good_tracks = "abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.2"  # cm, cm, GeV/c
        no_good_tracks = f"nCleanedTracks({good_tracks}) < 1"

        # get two most energetic photons in the event (must be at least 100 MeV
        # and not more than 7 GeV)
        ma.cutAndCopyList(
            "gamma:controlKLM", "gamma:all", "0.1 < useCMSFrame(clusterE) < 7", path=path)
        ma.rankByHighest("gamma:controlKLM", "useCMSFrame(clusterE)", numBest=2, path=path)

        # will build pairwise candidates from the gamma:controlKLM list:
        # vpho -> gamma gamma

        # the more energetic must be at least 4.5 GeV
        tag_daughter = "daughterHighest(useCMSFrame(clusterE)) > 4.5"
        # note that sometimes the probe will also fulfill this criteria, but the
        # candidate list will *not* be double-counted: these extra candidates need
        # to be added back offline

        # apply prescales to the less energetic daughter: compare to the eventwise random number
        probe_high = f"[daughterLowest(useCMSFrame(clusterE)) > 4.5] and [eventRandom < {prescale_high}]"
        probe_low = f"[daughterLowest(useCMSFrame(clusterE)) < 4.5] and [eventRandom < {prescale_low}]"
        prescale = f"[ {probe_high} ] or [ {probe_low} ]"

        # ~back-to-back in phi in the CMS (3.1066... radians = 178 degrees)
        delta_phi_cut = "abs(daughterDiffOfPhiCMS(0, 1)) > 3.1066860685499065"

        # sum theta in the cms 178 --> 182 degrees
        sum_th = "daughterSumOf(useCMSFrame(theta))"
        sum_th_cut = f"3.1066860685499065 < {sum_th} < 3.1764992386296798"

        # now build and return the candidates passing the AND of our cuts
        cuts = [no_good_tracks, tag_daughter, prescale, delta_phi_cut, sum_th_cut]
        cuts = " and ".join([f"[ {cut} ]" for cut in cuts])

        ma.reconstructDecay(
            "vpho:singlePhotonControlKLM -> gamma:controlKLM gamma:controlKLM",
            cuts, path=path)
        self.SkimLists = ["vpho:singlePhotonControlKLM"]


@fancy_skim_header
class DielectronPlusMissingEnergy(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}`

    Warning:
        This skim is currently deactivated, since the retention rate is too high.
    """

    __authors__ = "Giacomo De Pietro"
    __description__ = (
        "Dielectron skim, needed for :math:`e^{+}e^{-} \\to A^{\\prime} h^{\\prime};`"
        ":math:`A^{\\prime} \\to e^{+}e^{-}; \\, h^{\\prime} \\to \\mathrm{invisible}` and other searches."
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdE("all", path=path)

    TestFiles = [get_test_file("MC13_mumuBGx1")]

    def build_lists(self, path):
        dielectron_list = []
        skim_label = "forDielectronMissingEnergySkim"
        dielectron_name = f"Z0:{skim_label}"

        # Define some basic cuts
        fromIP_cut = "[abs(dz) < 5.0] and [abs(dr) < 2.0]"
        electronID_cut = "[electronID > 0.2]"
        # We require that the electron points to the barrel ECL + 10 degrees
        theta_cut = "[0.387 < theta < 2.421]"
        # We want exactly 2 tracks from IP
        dielectron_cut = f"[nCleanedTracks({fromIP_cut}) == 2]"
        # And the pair must have pt > 200 MeV in CMS frame
        dielectron_cut += " and [useCMSFrame(pt) > 0.2]"

        # Reconstruct the dielectron candidate
        electron_cuts = " and ".join([fromIP_cut, electronID_cut, theta_cut])
        ma.cutAndCopyList(f"e+:{skim_label}", "e+:all", electron_cuts, path=path)
        ma.reconstructDecay(f"{dielectron_name} -> e+:{skim_label} e-:{skim_label}", dielectron_cut, path=path)

        # And return the dielectron list
        dielectron_list.append(dielectron_name)
        self.SkimLists = dielectron_list


@fancy_skim_header
class RadBhabhaV0Control(BaseSkim):

    """
    Control sample: :math:`e^{+}e^{-} \\to e^{+}e^{-}V^{0};`"
    """

    __authors__ = "Savino Longo"
    __description__ = (
        "iDM control sample skim. :math:`e^{+}e^{-} \\to e^{+}e^{-}V^{0};`"
    )
    __contact__ = __liaison__
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)
        stdE("all", path=path)

    def build_lists(self, path):

        # require Bhabha tracks are high p and E/p is consitent with e+/e-
        BhabhaTrackCuts = 'abs(dr)<0.5 and abs(dz)<2 and pt>0.2 and 0.8<clusterEoP<1.2 and p>1.0 and clusterReg==2 and nCDCHits>4'
        BhabhaSystemCuts = '4<M<10 and 0.5<pRecoilTheta<2.25'
        V0TrackCuts = 'nCDCHits>4 and p<3.0'
        V0Cuts = 'dr>0.5'
        PhotonVetoCuts = 'p>1.0'  # event should have no high E photons

        ma.cutAndCopyList("gamma:HighEGammaVeto", "gamma:all", PhotonVetoCuts, path=path)
        ma.cutAndCopyList("e+:BhabhaTrack", "e+:all", BhabhaTrackCuts, path=path)
        ma.cutAndCopyList("e+:V0Track", "e+:all", V0TrackCuts, path=path)

        ma.reconstructDecay("vpho:BhabhaSysyem -> e+:BhabhaTrack e-:BhabhaTrack", BhabhaSystemCuts, path=path)

        ma.reconstructDecay("vpho:V0System -> e+:V0Track e-:V0Track", '', path=path)
        vertex.treeFit('vpho:V0System', conf_level=0.0, path=path)
        ma.applyCuts('vpho:V0System', V0Cuts, path=path)

        ma.reconstructDecay('vpho:Total -> vpho:BhabhaSysyem vpho:V0System', '', path=path)

        eventCuts = ('nParticlesInList(gamma:HighEGammaVeto)<1 and '
                     'nParticlesInList(vpho:Total)>0')

        path = self.skim_event_cuts(eventCuts, path=path)

        self.SkimLists = ["vpho:Total"]


@fancy_skim_header
class InelasticDarkMatter(BaseSkim):
    """
    Skim list contains events with no tracks from IP, no high E tracks and only one high E photon.
    """
    __authors__ = ["Savino Longo"]
    __contact__ = __liaison__
    __description__ = "iDM list for the iDM analysis."
    __category__ = "physics, dark sector"

    def load_standard_lists(self, path):
        stdPhotons("all", path=path)
        stdE("all", path=path)

    def build_lists(self, path):

        IPtrack = 'abs(dr) < 0.05'  # cm
        HighEtrack = 'useCMSFrame(p)>3.0'  # GeV
        ma.cutAndCopyList("e+:TrackFromIP", "e+:all", IPtrack, path=path)
        ma.cutAndCopyList("e+:HighEnergyTrack", "e+:all", HighEtrack, path=path)

        signalPhoton = "[clusterReg==2 and useCMSFrame(E) > 1.0] or "
        signalPhoton += "[clusterReg ==  1 and useCMSFrame(E) > 2.0] or "  # fwd
        signalPhoton += "[clusterReg ==  3 and useCMSFrame(E) > 2.0] or "  # bwd
        signalPhoton += "[clusterReg == 11 and useCMSFrame(E) > 2.0] or "  # between fwd and barrel
        signalPhoton += "[clusterReg == 13 and useCMSFrame(E) > 2.0] "     # between bwd and barrel

        photonVetoHE1 = 'useCMSFrame(p) > 0.6'
        photonVetoHE3 = 'p>0.5'

        ma.cutAndCopyList("gamma:ISR", "gamma:all", signalPhoton, path=path)
        ma.cutAndCopyList("gamma:HighEnergyPhotons", "gamma:all", photonVetoHE1, path=path)
        ma.cutAndCopyList("gamma:MediumEnergyPhotons", "gamma:all", photonVetoHE3, path=path)

        idmEventCuts = ('nParticlesInList(gamma:ISR)==1 and '
                        'nParticlesInList(e+:TrackFromIP)==0 and '
                        'nParticlesInList(e+:HighEnergyTrack) == 0 and '
                        'nParticlesInList(gamma:HighEnergyPhotons) == 1 and '
                        'nParticlesInList(gamma:MediumEnergyPhotons) < 4 and '
                        'HighLevelTrigger == 1')

        path = self.skim_event_cuts(idmEventCuts, path=path)

        self.SkimLists = ['gamma:ISR']
