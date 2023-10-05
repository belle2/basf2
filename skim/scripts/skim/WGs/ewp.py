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

__liaison__ = "Rahul Tiwary <rahul.tiwary@tifr.res.in>"
_VALIDATION_SAMPLE = "mdst14.root"


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
    * :math:`1.4\\,\\text{GeV}<\\E_{\\gamma}<3.4\\,\\text{GeV}` in CMS frame
    """

    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\gamma` inclusive skim."
    __contact__ = __liaison__
    __category__ = "physics, electroweak penguins, radiative decays"

    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdPi("all", path=path)
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

        # Apply event cuts R2 < 0.7 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.7 and nTracks >= 3', path=path)

        # Apply gamma cuts clusterE9E21 > 0.9 and 1.4 < E_gamma < 3.4 GeV (in CMS frame)
        ma.cutAndCopyList('gamma:ewp', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

        ma.reconstructDecay('B+:gamma -> gamma:ewp', '', path=path, allowChargeViolation=True)

        return ['B+:gamma']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        histogram_filename = f'{self}_Validation.root'

        stdK('all', path=path)
        stdPhotons('cdc', path=path)
        ma.cutAndCopyList('gamma:sig', 'gamma:cdc', 'clusterNHits > 1.5 and E > 1.5', True, path)

        ma.reconstructDecay('K*0:sig  -> K+:all pi-:all', '0.6 < M < 1.6', path=path)
        ma.reconstructDecay('B0:sig ->  K*0:sig gamma:sig', '5.22 < Mbc < 5.3 and  abs(deltaE)< .5', path=path)

        # the variables that are printed out are: Mbc and deltaE
        create_validation_histograms(
            rootfile=histogram_filename,
            particlelist='B0:sig',
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

    validation_sample = _VALIDATION_SAMPLE

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

        # Apply event cuts R2 < 0.7 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.7 and nTracks >= 3', path=path)

        # Apply electron cut p > 0.395 GeV, electronID > 0.1 + fairTrack
        # Apply muon cuts p > 0.395 GeV, muonID > 0.5 + fairTrack
        fairTrack = 'dr < 0.5 and abs(dz) < 2'

        ma.cutAndCopyList('e+:ewp', 'e+:all', 'p > 0.395 and electronID_noTOP > 0.1 and ' + fairTrack, path=path)
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

        return ['B+:xll']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        histogram_filename = f'{self}_Validation.root'

        stdK(listtype='good', path=path)
        stdMu(listtype='good', path=path)
        ma.reconstructDecay("B+:signal -> K+:good mu+:good mu-:good", "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)

        create_validation_histograms(
            rootfile=histogram_filename,
            particlelist='B+:signal',
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

        # Apply event cuts R2 < 0.7 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.7 and nTracks >= 3', path=path)

        # Apply electron cut p > 0.395 GeV, electronID > 0.1 + fairTrack
        # Apply muon cuts p > 0.395 GeV, muonID > 0.5 + fairTrack
        fairTrack = 'dr < 0.5 and abs(dz) < 2'

        ma.cutAndCopyList('e+:ewp', 'e+:all', 'p > 0.395 and electronID_noTOP > 0.1 and ' + fairTrack, path=path)
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

        return ['B+:lfv']
