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
from variables import variables


def B2XgammaList(path):
    """Build the skim list for B --> X(s,d) gamma decays"""

    # event level cuts: R2 and require a minimum number of tracks + decent photons
    ma.fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

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

    # Apply event cuts R2 < 0.5 and nTracks >= 3
    ma.applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

    # Apply gamma cuts clusterE9E21 > 0.9 and 1.4 < E_gamma < 3.4 GeV (in CMS frame)
    ma.cutAndCopyList('gamma:ewp', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

    ma.reconstructDecay('B+:gamma -> gamma:ewp', '', path=path, allowChargeViolation=True)

    BtoXgammaList = ['B+:gamma']

    return BtoXgammaList


def B2XllList(path):
    """Build the skim list for B --> X(s,d) l+ l- decays"""

    # event level cuts: R2 and require a minimum number of tracks
    ma.fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
    ma.fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

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

    # Apply event cuts R2 < 0.5 and nTracks >= 3
    ma.applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

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

    BptoXllList = ['B+:xll']

    return BptoXllList


def B2XllListLFV(path):
    # Build the skim list for B -> X ll decays (LFV modes only, inclusive)

    # Create lists for buildEventShape (basically all tracks and clusters)
    ma.cutAndCopyList('pi+:eventShapeForSkims', 'pi+:all', 'pt> 0.1', path=path)
    ma.cutAndCopyList('gamma:eventShapeForSkims', 'gamma:all', 'E > 0.1', path=path)

    # buildEventShape to access R2
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

    # Apply event cuts R2 < 0.5 and nTracks >= 3
    ma.applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

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

    BtoXllListLFV = ['B+:lfv']

    return BtoXllListLFV


@fancy_skim_header
class BtoXgamma(BaseSkim):
    """"""
    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\gamma` inclusive skim."
    __contact__ = ""
    __category__ = "physics, electroweak penguins, radiative decays"

    RequiredStandardLists = {
        "stdCharged": {
            "stdPi": ["all"],
        },
        "stdPhotons": {
            "stdPhotons": ["all", "loose"],
        },
    }

    def build_lists(self, path):
        """Build the skim list for :math:`B \\to X_{(s,d)}\\gamma` decays."""
        # event level cuts: R2 and require a minimum number of tracks + decent photons
        ma.fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
        ma.fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

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

        # Apply event cuts R2 < 0.5 and nTracks >= 3
        path = self.skim_event_cuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

        # Apply gamma cuts clusterE9E21 > 0.9 and 1.4 < E_gamma < 3.4 GeV (in CMS frame)
        ma.cutAndCopyList('gamma:ewp', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

        ma.reconstructDecay('B+:gamma -> gamma:ewp', '', path=path, allowChargeViolation=True)

        self.SkimLists = ['B+:gamma']


@fancy_skim_header
class BtoXll(BaseSkim):
    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\ell\\ell` (no LFV modes) inclusive skim."
    __contact__ = ""
    __category__ = "physics, electroweak penguins, radiative decays"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdMu": ["all"],
            "stdPi": ["all"],
        },
        "stdPhotons": {
            "stdPhotons": ["all"],
        },
    }

    def build_lists(self, path):
        """Build the skim list for :math:`B \\to X_{(s,d)}\\ell^+\\ell^-` decays."""

        # event level cuts: R2 and require a minimum number of tracks
        ma.fillParticleList(decayString='pi+:eventShapeForSkims', cut='pt > 0.1', path=path)
        ma.fillParticleList(decayString='gamma:eventShapeForSkims', cut='E > 0.1', path=path)

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
    __authors__ = ["Trevor Shillington"]
    __description__ = ":math:`B\\to X\\ell\\ell` (LFV modes only) inclusive skim."
    __contact__ = ""
    __category__ = "physics, electroweak penguins, radiative decays"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdMu": ["all"],
            "stdPi": ["all"],
        },
        "stdPhotons": {
            "stdPhotons": ["all"],
        },
    }

    def build_lists(self, path):
        """Build the skim list for B -> X ll decays (LFV modes only, inclusive)."""
        # Create lists for buildEventShape (basically all tracks and clusters)
        ma.cutAndCopyList('pi+:eventShapeForSkims', 'pi+:all', 'pt> 0.1', path=path)
        ma.cutAndCopyList('gamma:eventShapeForSkims', 'gamma:all', 'E > 0.1', path=path)

        # buildEventShape to access R2
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
    __authors__ = ["Cyrille Praz"]
    __description__ = "Inclusive skim for :math:`B\\to K\\nu\\nu` analysis"
    __contact__ = "Cyrille Praz <cyrille.praz@desy.de>"
    __category__ = "physics, electroweak penguins, radiative decays"

    NoisyModules = ["ParticleCombiner"]

    RequiredStandardLists = None

    def build_lists(self, path):

        # Default cleanup also used in and ma.buildEventShape
        track_cleanup = 'pt > 0.1'
        track_cleanup += ' and thetaInCDCAcceptance'
        track_cleanup += ' and abs(dz) < 3.0'
        track_cleanup += ' and dr < 0.5'

        # Min 3 tracks and Max 10 tracks per event.
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
