#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""

Skim list building functions for EWP inclusive skims:
B->Xgamma, B->Xll, B->Xll (LFV modes)

"""

__authors__ = [
    "Trevor Shillington"
]

import basf2 as b2
import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdMu, stdPi
from stdPhotons import stdPhotons
from variables import variables

__liaison__ = "Trevor Shillington <trshillington@hep.physics.mcgill.ca>"


@fancy_skim_header
class BtoXgamma(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^+ \\to X\\gamma` inclusive

    Event-level cuts:

    * :math:`\\text{foxWolframR2} < 0.5` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} \\geq 3`

    Cuts on photons:

    * :math:`\\text{clusterE9E21}>0.9`
    * :math:`1.4\\,\\text{GeV}<\\E_{\\gamma}<3.4\\,\\text{GeV}` in CMS frame
    """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\gamma` inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        stdPhotons("loose", path=path)

    def build_lists(self, path):
        """Build the skim list for :math:`B \\to X\\gamma` decays."""
        # event level cuts: R2 and require a minimum number of tracks + decent photons
        ma.fillParticleList(decayString='pi+:BtoXgamma_eventshape', cut='pt > 0.1', path=path)
        ma.fillParticleList(decayString='gamma:BtoXgamma_eventshape', cut='E > 0.1', path=path)

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

        # Apply event cuts R2 < 0.5 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

        # Apply gamma cuts clusterE9E21 > 0.9 and 1.4 < E_gamma < 3.4 GeV (in CMS frame)
        ma.cutAndCopyList('gamma:ewp', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

        ma.reconstructDecay('B+:gamma -> gamma:ewp', '', path=path, allowChargeViolation=True)

        self.SkimLists = ['B+:gamma']


@fancy_skim_header
class BtoXll(BaseSkim):
    """
        Reconstructed decay modes:

      * :math:`B^+ \\to X e^+ e^-`
      * :math:`B^+ \\to X e^+ e^+`
      * :math:`B^+ \\to X \\mu^+ \\mu^-`
      * :math:`B^+ \\to X \\mu^+ \\mu^+`


      Event-level cuts:

      * :math:`\\text{foxWolframR2} < 0.5` constructed using tracks with
        :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
      * :math:`n_{\\text{tracks}} \\geq 3`

      Cuts on electrons:

      * :math:`\\text{electronID} > 0.1`
      * :math:`p > 0.395\\,\\text{GeV}` in lab frame
      * :math:`dr<0.5 and abs(dz)<2`

      Cuts on muons:

      * :math:`\\text{muonID} > 0.5`
      * :math:`p > 0.395\\,\\text{GeV}` in lab frame
      * :math:`dr<0.5 and abs(dz)<2`


      Cut on dilepton energy:

      * :math:`E_{\\ell\\ell}>1.5\\,\\text{GeV}` in CMS frame.
      """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\ell\\ell` (no LFV modes) inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

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

        # Apply event cuts R2 < 0.5 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

        # Apply electron cut p > 0.395 GeV, electronID > 0.1 + fairTrack
        # Apply muon cuts p > 0.395 GeV, muonID > 0.5 + fairTrack
        fairTrack = 'dr < 0.5 and abs(dz) < 2'

        ma.cutAndCopyList('e+:ewp', 'e+:all', 'p > 0.395 and electronID > 0.1 and ' + fairTrack, path=path)
        ma.cutAndCopyList('mu+:ewp', 'mu+:all', 'p > 0.395 and muonID > 0.5 and ' + fairTrack, path=path)

        # Apply dilepton cut E_ll > 1.5 GeV (in CMS frame)
        E_dilep_cut = 'formula(daughter(0, useCMSFrame(E))+daughter(1, useCMSFrame(E))) > 1.5'

        # B+ reconstruction:
        # oppositely charged leptons
        ma.reconstructDecay('B+:ch1 -> e+:ewp e-:ewp', E_dilep_cut, dmID=1, path=path, allowChargeViolation=True)
        ma.reconstructDecay('B+:ch2 -> mu+:ewp mu-:ewp', E_dilep_cut, dmID=2, path=path, allowChargeViolation=True)
        # same charge leptons
        ma.reconstructDecay('B+:ch3 -> e+:ewp e+:ewp', E_dilep_cut, dmID=3, path=path, allowChargeViolation=True)
        ma.reconstructDecay('B+:ch4 -> mu+:ewp mu+:ewp', E_dilep_cut, dmID=4, path=path, allowChargeViolation=True)

        ma.copyLists('B+:xll', ['B+:ch1', 'B+:ch2', 'B+:ch3', 'B+:ch4'], path=path)

        self.SkimLists = ['B+:xll']


@fancy_skim_header
class BtoXll_LFV(BaseSkim):
    """
      Reconstructed decay modes:

    * :math:`B^+ \\to X e^+ \\mu^-`
    * :math:`B^+ \\to X \\mu^+ e^-`
    * :math:`B^+ \\to X e^+ \\mu^+`


    Event-level cuts:

    * :math:`\\text{foxWolframR2} < 0.5` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} \\geq 3`

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.1`
    * :math:`p > 0.395\\,\\text{GeV}` in lab frame
    * :math:`dr<0.5 and abs(dz)<2`

    Cuts on muons:

    * :math:`\\text{muonID} > 0.5`
    * :math:`p > 0.395\\,\\text{GeV}` in lab frame
    * :math:`dr<0.5 and abs(dz)<2`


    Cut on dilepton energy:

    * :math:`E_{\\ell\\ell}>1.5\\,\\text{GeV}` in CMS frame.
    """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\ell\\ell` (LFV modes only) inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

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

        # Apply event cuts R2 < 0.5 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

        # Apply electron cut p > 0.395 GeV, electronID > 0.1 + fairTrack
        # Apply muon cuts p > 0.395 GeV, muonID > 0.5 + fairTrack
        fairTrack = 'dr < 0.5 and abs(dz) < 2'

        ma.cutAndCopyList('e+:ewp', 'e+:all', 'p > 0.395 and electronID > 0.1 and ' + fairTrack, path=path)
        ma.cutAndCopyList('mu+:ewp', 'mu+:all', 'p > 0.395 and muonID > 0.5 and ' + fairTrack, path=path)

        # Apply dilepton cut E_ll > 1.5 GeV (in CMS frame)
        E_dilep_cut = 'formula(daughter(0, useCMSFrame(E))+daughter(1, useCMSFrame(E))) > 1.5'

        # B+ reconstruction:
        # oppositely charged leptons
        ma.reconstructDecay('B+:lfvch1 -> e+:ewp mu-:ewp', E_dilep_cut, dmID=1, path=path, allowChargeViolation=True)
        ma.reconstructDecay('B+:lfvch2 -> mu+:ewp e-:ewp', E_dilep_cut, dmID=2, path=path, allowChargeViolation=True)
        # same charge leptons
        ma.reconstructDecay('B+:lfvch3 -> e+:ewp mu+:ewp', E_dilep_cut, dmID=3, path=path, allowChargeViolation=True)

        ma.copyLists('B+:lfv', ['B+:lfvch1', 'B+:lfvch2', 'B+:lfvch3'], path=path)

        self.SkimLists = ['B+:lfv']


class inclusiveBplusToKplusNuNu(BaseSkim):
    """
      Reconstructed decay modes:

    * :math:`B^+ \\to K\\nu\\nu` inclusive

    Track cleanup:
    * :math:`p_t > 0.1`
    * :math:`thetaInCDCAcceptance`
    * :math:`dr<0.5 and abs(dz)<3.0`

    Event cleanup:
    * :math:`3 < nCleanedTracks < 11`

    Kaon cuts:
    * :math:`track cleanup + event cleanup + nPXDHits > 0`
    * :math:`p_t rank=1`
    * :math:`kaonID>0.01`

    MVA info and cuts:
    * mva_identifier: MVAFastBDT_InclusiveBplusToKplusNuNu_Skim
    * Global Tag: mva_inclusiveBplusToKplusNuNu
    * :math:`mva\\_identifier > 0.5`
    """

    __authors__ = ["Cyrille Praz"]
    __description__ = "Inclusive skim for :math:`B\\to K\\nu\\nu` analysis"
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

    NoisyModules = ["ParticleCombiner"]

    def build_lists(self, path):

        # Default cleanup also used in and ma.buildEventShape
        track_cleanup = 'pt > 0.1'
        track_cleanup += ' and thetaInCDCAcceptance'
        track_cleanup += ' and abs(dz) < 3.0'
        track_cleanup += ' and dr < 0.5'

        # Min 4 tracks and Max 10 tracks per event.
        event_cleanup = 'nCleanedTracks({}) > 3'.format(track_cleanup)
        event_cleanup += ' and nCleanedTracks({}) < 11'.format(track_cleanup)

        # Define the signal
        total_cleanup = track_cleanup + ' and ' + event_cleanup + ' and ' + 'nPXDHits>0'
        ma.fillParticleList('K+:inclusiveBplusToKplusNuNu', cut=total_cleanup, path=path)
        ma.rankByHighest('K+:inclusiveBplusToKplusNuNu', 'pt', path=path)
        ma.applyCuts('K+:inclusiveBplusToKplusNuNu', 'extraInfo(pt_rank)==1', path=path)
        ma.applyCuts('K+:inclusiveBplusToKplusNuNu', 'kaonID>1e-2', path=path)
        ma.reconstructDecay(decayString='B+:inclusiveBplusToKplusNuNu -> K+:inclusiveBplusToKplusNuNu', cut='', path=path)

        # Build the event-based variables that we need
        ma.buildEventShape(inputListNames=[],
                           default_cleanup=True,
                           allMoments=False,
                           cleoCones=True,
                           collisionAxis=False,
                           foxWolfram=True,
                           harmonicMoments=True,
                           jets=False,
                           sphericity=True,
                           thrust=True,
                           checkForDuplicates=False,
                           path=path)

        # Apply a MVA by reading from the DB
        mva_identifier = 'MVAFastBDT_InclusiveBplusToKplusNuNu_Skim'
        b2.conditions.append_globaltag('mva_inclusiveBplusToKplusNuNu')
        path.add_module('MVAExpert', listNames=['B+:inclusiveBplusToKplusNuNu'],
                        extraInfoName=mva_identifier, identifier=mva_identifier)
        variables.addAlias(mva_identifier, 'extraInfo({})'.format(mva_identifier))
        ma.applyCuts('B+:inclusiveBplusToKplusNuNu', mva_identifier+'>0.5', path=path)

        self.SkimLists = ['B+:inclusiveBplusToKplusNuNu']
