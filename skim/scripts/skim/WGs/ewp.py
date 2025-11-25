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

Skim list building functions for EWP inclusive skims:
B->Xgamma, B->Xll, B->Xll (LFV modes)

"""

import modularAnalysis as ma
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons

__liaison__ = "Ihor Prudiiev <Ihor.Prudiiev@ijs.si>"
_VALIDATION_SAMPLE = "mdst16.root"


@fancy_skim_header
class BtoXgamma(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^+ \\to X\\gamma` inclusive

    Event-level cuts:

    * :math:`\\text{foxWolframR2} < 0.7` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} \\geq 3`

    Cuts on photons:

    * :math:`\\text{clusterE9E21}>0.9`
    * :math:`1.4\\,\\text{GeV}< E_{\\gamma}<3.4\\,\\text{GeV}` in CMS frame
    """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\gamma` inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

    validation_sample = _VALIDATION_SAMPLE

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdPhotons("loose", path=path)

    def build_lists(self, path):
        """Build the skim list for :math:`B \\to X\\gamma` decays."""
        # event level cuts: R2 and require a minimum number of tracks + decent photons
        ma.fillParticleList(decayString='pi+:BtoXgamma_eventshape', cut='pt > 0.1', path=path)
        ma.fillParticleList(decayString='gamma:BtoXgamma_eventshape', cut='E > 0.1', path=path)

        # Event cleanup
        Event_cleanup = "nCleanedTracks(abs(dr) < 0.5 and abs(dz) < 2) >= 3"

        ma.buildEventShape(inputListNames=['pi+:BtoXgamma_eventshape', 'gamma:BtoXgamma_eventshape'],
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

        # Apply event cuts R2 < 0.7
        path = self.skim_event_cuts(f'{Event_cleanup} and foxWolframR2 < 0.7', path=path)

        # Apply gamma cuts clusterE9E21 > 0.9 and 1.4 < E_gamma < 3.4 GeV (in CMS frame)
        ma.cutAndCopyList('gamma:ewp_BtoXgamma', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

        ma.reconstructDecay('B+:gamma -> gamma:ewp_BtoXgamma', '', path=path, allowChargeViolation=True)

        return ['B+:gamma']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        histogram_filename = f'{self}_Validation.root'

        stdK('all', path=path)
        stdPhotons('cdc', path=path)
        ma.cutAndCopyList('gamma:sig_btxg', 'gamma:cdc', 'clusterNHits > 1.5 and E > 1.5', True, path)

        ma.reconstructDecay('K*0:sig_btxg  -> K+:all pi-:all', '0.6 < M < 1.6', path=path)
        ma.reconstructDecay('B0:sig_btxg ->  K*0:sig_btxg gamma:sig_btxg', '5.22 < Mbc < 5.3 and  abs(deltaE)< .5', path=path)

        # the variables that are printed out are: Mbc and deltaE
        create_validation_histograms(
            rootfile=histogram_filename,
            particlelist='B0:sig_btxg',
            variables_1d=[
                ('Mbc', 100, 5.2, 5.3, 'Signal B0 Mbc', __liaison__,
                 'Mbc of the signal B0', '', 'Mbc [GeV/c^2]', 'Candidates'),
                ('deltaE', 100, -1, 1, 'Signal B0 deltaE', __liaison__,
                 'deltaE of the signal B0', '', 'deltaE [GeV]', 'Candidates')
            ],
            variables_2d=[],
            path=path)


@fancy_skim_header
class BtoXll(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^+ \\to X e^+ e^-`
    * :math:`B^+ \\to X e^+ e^+`
    * :math:`B^+ \\to X \\mu^+ \\mu^-`
    * :math:`B^+ \\to X \\mu^+ \\mu^+`


    Event-level cuts:

    * :math:`\\text{foxWolframR2} < 0.7` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} \\geq 3`

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.1`
    * :math:`p > 0.395\\,\\text{GeV}` in lab frame
    * :math:`dr<0.5` and :math:`abs(dz)<2`

    Cuts on muons:

    * :math:`\\text{muonID} > 0.5`
    * :math:`p > 0.395\\,\\text{GeV}` in lab frame
    * :math:`dr<0.5` and :math:`abs(dz)<2`


    Cut on dilepton energy:

    * :math:`E_{\\ell\\ell}>1.5\\,\\text{GeV}` in CMS frame.
    """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\ell\\ell` (no LFV modes) inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

    validation_sample = _VALIDATION_SAMPLE

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):
        """Build the skim list for :math:`B \\to X\\ell\\ell` non-LFV decays."""

        # event level cuts: R2 and require a minimum number of tracks
        ma.fillParticleList(decayString='pi+:BtoXll_eventshape', cut='pt > 0.1', path=path)
        ma.fillParticleList(decayString='gamma:BtoXll_eventshape', cut='E > 0.1', path=path)

        # Event cleanup
        Event_cleanup = "nCleanedTracks(abs(dr) < 0.5 and abs(dz) < 2) >= 3"

        ma.buildEventShape(inputListNames=['pi+:BtoXll_eventshape', 'gamma:BtoXll_eventshape'],
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

        # Apply event cuts R2 < 0.7
        path = self.skim_event_cuts(f'{Event_cleanup} and foxWolframR2 < 0.7', path=path)

        # Apply electron cut p > 0.395 GeV, electronID > 0.1 + fairTrack
        # Apply muon cuts p > 0.395 GeV, muonID > 0.5 + fairTrack
        fairTrack = 'dr < 0.5 and abs(dz) < 2'

        ma.cutAndCopyList('e+:ewp_btxll', 'e+:all', 'p > 0.395 and electronID > 0.1 and ' + fairTrack, path=path)
        ma.cutAndCopyList('mu+:ewp_btxll', 'mu+:all', 'p > 0.395 and muonID > 0.5 and ' + fairTrack, path=path)

        # Apply dilepton cut E_ll > 1.5 GeV (in CMS frame)
        E_dilep_cut = 'formula(daughter(0, useCMSFrame(E))+daughter(1, useCMSFrame(E))) > 1.5'

        # B+ reconstruction:
        # oppositely charged leptons
        ma.reconstructDecay('B+:ch1 -> e+:ewp_btxll e-:ewp_btxll', E_dilep_cut, dmID=1, path=path, allowChargeViolation=True)
        ma.reconstructDecay('B+:ch2 -> mu+:ewp_btxll mu-:ewp_btxll', E_dilep_cut, dmID=2, path=path, allowChargeViolation=True)
        # same charge leptons
        ma.reconstructDecay('B+:ch3 -> e+:ewp_btxll e+:ewp_btxll', E_dilep_cut, dmID=3, path=path, allowChargeViolation=True)
        ma.reconstructDecay('B+:ch4 -> mu+:ewp_btxll mu+:ewp_btxll', E_dilep_cut, dmID=4, path=path, allowChargeViolation=True)

        ma.copyLists('B+:xll', ['B+:ch1', 'B+:ch2', 'B+:ch3', 'B+:ch4'], path=path)

        return ['B+:xll']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        histogram_filename = f'{self}_Validation.root'

        stdK(listtype='good', path=path)
        stdMu(listtype='good', path=path)
        ma.reconstructDecay("B+:signal_btxll -> K+:good mu+:good mu-:good",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)

        create_validation_histograms(
            rootfile=histogram_filename,
            particlelist='B+:signal_btxll',
            variables_1d=[
                ('deltaE', 100, -0.5, 0.5, 'Signal B deltaE', __liaison__,
                 'deltaE of the Signal B', '', 'deltaE [GeV]', 'Candidates'),
                ('Mbc', 100, 5.2, 5.3, 'Signal B Mbc', __liaison__,
                 'Mbc of the signal B', '', 'Mbc [GeV/c^2]', 'Candidates')],
            variables_2d=[],
            path=path)


@fancy_skim_header
class BtoXll_LFV(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^+ \\to X e^+ \\mu^-`
    * :math:`B^+ \\to X \\mu^+ e^-`
    * :math:`B^+ \\to X e^+ \\mu^+`


    Event-level cuts:

    * :math:`\\text{foxWolframR2} < 0.7` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} \\geq 3`

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.1`
    * :math:`p > 0.395\\,\\text{GeV}` in lab frame
    * :math:`dr<0.5` and :math:`abs(dz)<2`

    Cuts on muons:

    * :math:`\\text{muonID} > 0.5`
    * :math:`p > 0.395\\,\\text{GeV}` in lab frame
    * :math:`dr<0.5` and :math:`abs(dz)<2`


    Cut on dilepton energy:

    * :math:`E_{\\ell\\ell}>1.5\\,\\text{GeV}` in CMS frame.
    """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\ell\\ell` (LFV modes only) inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):
        """Build the skim list for :math:`B \\to X\\ell\\ell` LFV decays."""
        # Create lists for buildEventShape (basically all tracks and clusters)
        ma.cutAndCopyList('pi+:BtoXllLFV_eventshape', 'pi+:all', 'pt> 0.1', path=path)
        ma.cutAndCopyList('gamma:BtoXllLFV_eventshape', 'gamma:all', 'E > 0.1', path=path)

        # Event cleanup
        Event_cleanup = "nCleanedTracks(abs(dr) < 0.5 and abs(dz) < 2) >= 3"

        # buildEventShape to access R2
        ma.buildEventShape(inputListNames=['pi+:BtoXllLFV_eventshape', 'gamma:BtoXllLFV_eventshape'],
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

        # Apply event cuts R2 < 0.7
        path = self.skim_event_cuts(f'{Event_cleanup} and foxWolframR2 < 0.7', path=path)

        # Apply electron cut p > 0.395 GeV, electronID > 0.1 + fairTrack
        # Apply muon cuts p > 0.395 GeV, muonID > 0.5 + fairTrack
        fairTrack = 'dr < 0.5 and abs(dz) < 2'

        ma.cutAndCopyList('e+:ewp_btxlllfv', 'e+:all', 'p > 0.395 and electronID > 0.1 and ' + fairTrack, path=path)
        ma.cutAndCopyList('mu+:ewp_btxlllfv', 'mu+:all', 'p > 0.395 and muonID > 0.5 and ' + fairTrack, path=path)

        # Apply dilepton cut E_ll > 1.5 GeV (in CMS frame)
        E_dilep_cut = 'formula(daughter(0, useCMSFrame(E))+daughter(1, useCMSFrame(E))) > 1.5'

        # B+ reconstruction:
        # oppositely charged leptons
        ma.reconstructDecay('B+:lfvch1 -> e+:ewp_btxlllfv mu-:ewp_btxlllfv',
                            E_dilep_cut, dmID=1, path=path, allowChargeViolation=True)
        ma.reconstructDecay('B+:lfvch2 -> mu+:ewp_btxlllfv e-:ewp_btxlllfv',
                            E_dilep_cut, dmID=2, path=path, allowChargeViolation=True)
        # same charge leptons
        ma.reconstructDecay('B+:lfvch3 -> e+:ewp_btxlllfv mu+:ewp_btxlllfv',
                            E_dilep_cut, dmID=3, path=path, allowChargeViolation=True)

        ma.copyLists('B+:lfv', ['B+:lfvch1', 'B+:lfvch2', 'B+:lfvch3'], path=path)

        return ['B+:lfv']


@fancy_skim_header
class B0TwoBody(BaseSkim):
    """
    Reconstructed decays
        * :math:`B^0 \\to e^+ e^-`
        * :math:`B^0 \\to e^+ \\mu^-`
        * :math:`B^0 \\to e^- \\mu^+`
        * :math:`B^0 \\to \\mu^+ \\mu^-`
        * :math:`B^0 \\to \\pi^+ \\pi^-`

    Cuts applied
        * :math:`n_{\\text{tracks}} \\geq 3`
        * :math:`|\\delta E| < 0.5 \\text{GeV}`
        * :math:`M_{bc} > 5.2 \\text{GeV}/c^2`
        * :math:`dr < 0.5 \\text{cm}, |dz| < 2 \\text{cm}`
    """

    __authors__ = ["Ryan Mueller and Santi Naylor"]
    __contact__ = __liaison__
    __description__ = "Skim for 2 body B0 decays"
    __category__ = "Physics, 2 Body, no PID"

    validation_sample = _VALIDATION_SAMPLE

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)

    def build_lists(self, path):

        cut_trk = 'dr < 0.5 and abs(dz) < 2'
        ma.cutAndCopyList('e+:ewp_2b', 'e+:all', cut_trk, path=path)
        ma.cutAndCopyList('mu+:ewp_2b', 'mu+:all', cut_trk, path=path)
        ma.cutAndCopyList('pi+:ewp_2b', 'pi+:all', cut_trk, path=path)

        cut_evt = "nCleanedTracks(abs(dr) < 0.5 and abs(dz) < 2)>=3"
        cut_b = "abs(deltaE) < 0.5 and Mbc > 5.2"
        path = self.skim_event_cuts(cut_evt, path=path)
        ma.reconstructDecay("B0:B0TwoBody_1 -> e+:ewp_2b e-:ewp_2b", cut_b, dmID=1, path=path)
        ma.reconstructDecay("B0:B0TwoBody_2 -> e+:ewp_2b mu-:ewp_2b", cut_b, dmID=2, path=path)
        ma.reconstructDecay("B0:B0TwoBody_3 -> e-:ewp_2b mu+:ewp_2b", cut_b, dmID=3, path=path)
        ma.reconstructDecay("B0:B0TwoBody_4 -> mu-:ewp_2b mu+:ewp_2b", cut_b, dmID=4, path=path)
        ma.reconstructDecay("B0:B0TwoBody_5 -> pi-:ewp_2b pi+:ewp_2b", cut_b, dmID=5, path=path)

        return ['B0:B0TwoBody_1', 'B0:B0TwoBody_2', 'B0:B0TwoBody_3', 'B0:B0TwoBody_4', 'B0:B0TwoBody_5']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.cutAndCopyLists("B0:B0TwoBody", "B0:B0TwoBody_5", "", path=path)

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B0:B0TwoBody",
            variables_1d=[
                ("Mbc", 100, 5.19, 5.3, "Signal B Mbc", __liaison__,
                 "Mbc of the Signal B", "", 'Mbc [GeV/c^2]', 'Candidates'),
                ("deltaE", 100, -0.5, 0.5, "Signal B deltaE", __liaison__,
                 "deltaE of the Signal B", "", "deltaE [GeV]", "Candidates"),
            ],
            variables_2d=[
                ("deltaE", 100, -0.6, 0.6, "Mbc", 100, 5.19, 5.3, "Mbc vs deltaE", __liaison__,
                 "", "")
            ],
            path=path,
        )


@fancy_skim_header
class FourLepton(BaseSkim):
    """
    Reconstructed decays
        * :math:`B^0 \\to e^- e^+ e^- e^+`
        * :math:`B^0 \\to e^- e^+ mu^- mu^+`

    Cuts applied
        * :math:`n_{\\text{tracks}} \\geq 5`
        * :math:`-1.5 < \\delta E < 0.5 \\text{GeV}`
        * :math:`5.2 < M_{bc} < 5.3 \\text{GeV}/c^2`
        * :math:`dr < 0.5 \\text{cm}, |dz| < 2 \\text{cm}`
        * :math:`muonID>0.1`
        * :math:`electronID>0.1`
    """

    __authors__ = ["Santi Naylor and Ryan Mueller"]
    __contact__ = __liaison__
    __description__ = "Skim for 4 body leptonic analyses"
    __category__ = "Physics, Leptonic, 4 Body"

    validation_sample = _VALIDATION_SAMPLE

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        cut_trk = 'dr < 0.5 and abs(dz) < 2 '

        ma.cutAndCopyList('e+:ewp_4b', 'e+:all', 'pt > 0.1 and electronID > 0.1 and ' + cut_trk, path=path)
        ma.cutAndCopyList('mu+:ewp_4b', 'mu+:all', 'pt > 0.1 and muonID > 0.1 and ' + cut_trk, path=path)

        cut_evt = "nCleanedTracks(abs(dr) < 0.5 and abs(dz) < 2)>=5"
        cut_b = "deltaE < 0.5 and deltaE > -1.5 and Mbc > 5.2 and Mbc < 5.3"
        path = self.skim_event_cuts(cut_evt, path=path)

        ma.reconstructDecay("B0:FourLepton_1 -> e+:ewp_4b e-:ewp_4b e+:ewp_4b e-:ewp_4b", cut_b, dmID=1, path=path)
        ma.reconstructDecay("B0:FourLepton_2 -> e+:ewp_4b e-:ewp_4b mu+:ewp_4b mu-:ewp_4b", cut_b, dmID=2, path=path)
        return ["B0:FourLepton_1", "B0:FourLepton_2"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.cutAndCopyLists("B0:FourLepton", "B0:FourLepton_1", "", path=path)

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B0:FourLepton",
            variables_1d=[
                ("Mbc", 100, 5.19, 5.3, "Signal B Mbc", __liaison__,
                 "Mbc of the Signal B", "", 'Mbc [GeV/c^2]', 'Candidates'),
                ("deltaE", 100, -1.5, 0.5, "Signal B deltaE", __liaison__,
                 "deltaE of the Signal B", "", "deltaE [GeV]", "Candidates"),
            ],
            variables_2d=[
                ("deltaE", 100, -0.6, 0.6, "Mbc", 100, 5.19, 5.3, "Mbc vs deltaE", __liaison__,
                 "", "")
            ],
            path=path,
        )


@fancy_skim_header
class RadiativeDilepton(BaseSkim):
    """
    Reconstructed decays
        * :math:`B^0 \\to e^- e^+ \\gamma`
        * :math:`B^0 \\to mu^- mu^+ \\gamma`

    Cuts applied
        * :math:`n_{\\text{tracks}} \\geq 3`
        * :math:`-1.0 < \\delta E < 0.5 \\text{GeV}`
        * :math:` 5.2 < M_{bc} < 5.3 \\text{GeV}/c^2`
        * :math:`dr < 0.5 \\text{cm}, |dz| < 2 \\text{cm}`
        * :math:`muonID>0.1`
        * :math:`electronID>0.1`
    """

    __authors__ = ["Santi Naylor and Ryan Mueller"]
    __contact__ = __liaison__
    __description__ = "Skim for 3 body leptonic analyses "
    __category__ = "Physics, Leptonic, 3 body"

    validation_sample = _VALIDATION_SAMPLE

    ApplyHLTHadronCut = False

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):

        cut_trk = 'dr < 0.5 and abs(dz) < 2'
        ma.cutAndCopyList('e+:ewp_radll', 'e+:all', 'pt > 0.1 and electronID > 0.1 and ' + cut_trk, path=path)
        ma.cutAndCopyList('mu+:ewp_radll', 'mu+:all', 'pt > 0.1 and muonID > 0.1 and ' + cut_trk, path=path)

        cut_evt = "nCleanedTracks(abs(dr) < 0.5 and abs(dz) < 2)>=3"
        cut_b = "deltaE < 0.5 and deltaE > -1.0 and Mbc > 5.2 and Mbc < 5.3"
        path = self.skim_event_cuts(cut_evt, path=path)

        ma.reconstructDecay("B0:RadiativeDilepton_1 -> e+:ewp_radll e-:ewp_radll gamma:all", cut_b, dmID=1, path=path)
        ma.reconstructDecay("B0:RadiativeDilepton_2 -> mu+:ewp_radll mu-:ewp_radll gamma:all", cut_b, dmID=1, path=path)
        return ["B0:RadiativeDilepton_1", "B0:RadiativeDilepton_2"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.cutAndCopyLists("B0:RadiativeDilepton", "B0:RadiativeDilepton_1", "", path=path)

        histogramFilename = f"{self}_Validation.root"

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist="B0:RadiativeDilepton",
            variables_1d=[
                ("Mbc", 100, 5.19, 5.3, "Signal B Mbc", __liaison__,
                 "Mbc of the Signal B", "", 'Mbc [GeV/c^2]', 'Candidates'),
                ("deltaE", 100, -1.0, 0.5, "Signal B deltaE", __liaison__,
                 "deltaE of the Signal B", "", "deltaE [GeV]", "Candidates"),
            ],
            variables_2d=[
                ("deltaE", 100, -0.6, 0.6, "Mbc", 100, 5.19, 5.3, "Mbc vs deltaE", __liaison__,
                 "", "")
            ],
            path=path,
        )
