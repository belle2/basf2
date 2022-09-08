#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

""" Skim list building functions for quarkonium analyses: bottomonium, charmonium, resonance """

import modularAnalysis as ma
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdMu
from stdPhotons import stdPhotons
from stdV0s import stdLambdas
from variables import variables as v

__liaison__ = "Sen Jia <jiasen@buaa.edu.cn>"
_VALIDATION_SAMPLE = "mdst14.root"


@fancy_skim_header
class BottomoniumEtabExclusive(BaseSkim):
    """
    Reconstructed decay modes:

    * ``eta_b -> gamma gamma``

    Selection criteria:

    * ``2 std photon with E > 3.5 GeV``
    * ``7 < M(eta_b) < 10 GeV/c^2``
    * ``foxWolframR2 < 0.995``
    """

    __authors__ = ["Stefano Spataro", "Sen Jia"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdPhotons("loose", path=path, loadPhotonBeamBackgroundMVA=False)

    def build_lists(self, path):
        # create and fill hard photon
        ma.fillParticleList(decayString="pi+:BottomoniumEtab_eventshape", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:BottomoniumEtab_eventshape", cut="E > 0.1", path=path,
                            loadPhotonBeamBackgroundMVA=False)

        ma.buildEventShape(inputListNames=["pi+:BottomoniumEtab_eventshape", "gamma:BottomoniumEtab_eventshape"],
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

        ma.cutAndCopyList("gamma:hard", "gamma:loose", "E>3.5", path=path)
        ma.applyCuts("gamma:hard", "foxWolframR2 < 0.995", path=path)

        # the requirement of 7 < M(eta_b) < 10 GeV/c2
        Etabcuts = "M > 7 and M < 10"

        # eta_b candidates are reconstructed
        Etab_Channels = ["gamma:hard gamma:hard"]

        # define the eta_b decay list
        EtabList = []

        # reconstruct the decay eta_b -> gamma gamma
        for chID, channel in enumerate(Etab_Channels):
            ma.reconstructDecay("eta_b:all" + str(chID) + " -> " + channel, Etabcuts, chID, path=path)
            EtabList.append("eta_b:all" + str(chID))

        return EtabList


@fancy_skim_header
class BottomoniumUpsilon(BaseSkim):
    """
    Reconstructed decay modes:

    * Y(1S,2S) -> l^+ l^{-} (l = e or mu)

    Selection criteria:

    * 2 tracks with momentum ranging between ``3.5 < p < 15``
    * At least 1 track ``p < 1.5`` or 1 std photon with ``E > 150 MeV``
    * ``M(Y(1S,2S)) > 8 GeV/c^2``
    * ``foxWolframR2 < 0.995``
    """
    __authors__ = ["Stefano Spataro", "Sen Jia"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdPhotons("loose", path=path, loadPhotonBeamBackgroundMVA=False)

    def build_lists(self, path):
        Ycuts = ""
        # create and fill e/mu/pi/photon ParticleLists
        ma.fillParticleList("mu+:BottomoniumUpsilon", "p<15 and p>3.5", path=path)
        ma.fillParticleList("e+:BottomoniumUpsilon", "p<15 and p>3.5", path=path)
        ma.fillParticleList("pi+:BottomoniumUpsilon", "p<1.5 and pt>0.05", path=path)
        ma.cutAndCopyList("gamma:soft", "gamma:loose", "E>0.15", path=path)

        # Y(1S,2S) are reconstructed with e^+ e^- or mu^+ mu^-
        ma.reconstructDecay("Upsilon:ee -> e+:BottomoniumUpsilon e-:BottomoniumUpsilon", "M > 8", path=path)
        ma.reconstructDecay("Upsilon:mumu -> mu+:BottomoniumUpsilon mu-:BottomoniumUpsilon", "M > 8", path=path)
        ma.copyLists("Upsilon:all", ["Upsilon:ee", "Upsilon:mumu"], path=path)

        # require foxWolframR2 < 0.995
        ma.fillParticleList(decayString="pi+:BottomoniumUpsilon_eventshape", cut="pt > 0.1", path=path)
        ma.fillParticleList(decayString="gamma:BottomoniumUpsilon_eventshape", cut="E > 0.1", path=path,
                            loadPhotonBeamBackgroundMVA=False)

        ma.buildEventShape(inputListNames=["pi+:BottomoniumUpsilon_eventshape", "gamma:BottomoniumUpsilon_eventshape"],
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

        ma.applyCuts("Upsilon:all", "foxWolframR2 < 0.995", path=path)

        # Y(1S,2S) with pi+ or photon are reconstructed
        Upsilon_Channels = ["Upsilon:all pi+:BottomoniumUpsilon",
                            "Upsilon:all gamma:soft"]

        # define the Y(1S,2S) decay channel list
        UpsilonList = []

        # reconstruct the decay channel
        for chID, channel in enumerate(Upsilon_Channels):
            ma.reconstructDecay("junction:all" + str(chID) + " -> " + channel, Ycuts, chID, path=path, allowChargeViolation=True)
            UpsilonList.append("junction:all" + str(chID))

        # reture the list
        return UpsilonList

    #       *two* sets of validation scripts defined.


@fancy_skim_header
class CharmoniumPsi(BaseSkim):
    """
    Reconstructed decay modes:

    * J/psi -> l^+ l^- (l = e or mu)
    * psi(2S) -> l^+ l^- (l = e or mu)

    Selection criteria:

    * 2 tracks with electronID > 0.1 or muonID > 0.1 and 2.7 < M < 4.
      Track-quality requirements are not applied.
    """
    __authors__ = ["Kirill Chilikin"]
    __description__ = "Selection of J/psi and psi(2S) via leptonic decays."
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdMu('loosepid', path=path)
        stdPhotons("all", path=path, loadPhotonBeamBackgroundMVA=False)

    def build_lists(self, path):

        # Electron list. Exclude TOP
        ma.fillParticleList('e+:loosepid_noTOP', 'electronID_noTOP > 0.1', path=path)

        # Mass cuts.
        jpsi_mass_cut = '2.85 < M < 3.3'
        psi2s_mass_cut = '3.45 < M < 3.9'

        # Electrons with bremsstrahlung correction.
        # Use both correction algorithms as well as uncorrected electrons
        # to allow for algorithm comparison in analysis.
        # The recommeneded list for further reconstruction is J/psi:eebrems.
        # The estimated ratio of efficiencies in B decays in release 5.1.5 is
        # 1.00 (J/psi:eebrems) : 0.95 (J/psi:eebrems2) : 0.82 (J/psi:ee).
        ma.correctBremsBelle('e+:brems', 'e+:loosepid_noTOP', 'gamma:all',
                             angleThreshold=0.05,
                             path=path)
        ma.correctBrems('e+:brems2', 'e+:loosepid_noTOP', 'gamma:all', path=path)

        # Reconstruct J/psi or psi(2S).
        ma.reconstructDecay('J/psi:ee -> e+:loosepid_noTOP e-:loosepid_noTOP',
                            jpsi_mass_cut, path=path)
        ma.reconstructDecay('psi(2S):ee -> e+:loosepid_noTOP e-:loosepid_noTOP',
                            psi2s_mass_cut, path=path)

        ma.reconstructDecay('J/psi:eebrems -> e+:brems e-:brems',
                            jpsi_mass_cut, path=path)
        ma.reconstructDecay('psi(2S):eebrems -> e+:brems e-:brems',
                            psi2s_mass_cut, path=path)

        ma.reconstructDecay('J/psi:eebrems2 -> e+:brems2 e-:brems2',
                            jpsi_mass_cut, path=path)
        ma.reconstructDecay('psi(2S):eebrems2 -> e+:brems2 e-:brems2',
                            psi2s_mass_cut, path=path)

        ma.reconstructDecay('J/psi:mumu -> mu+:loosepid mu-:loosepid',
                            jpsi_mass_cut, path=path)
        ma.reconstructDecay('psi(2S):mumu -> mu+:loosepid mu-:loosepid',
                            psi2s_mass_cut, path=path)

        # Return the lists.
        return ['J/psi:ee', 'psi(2S):ee',
                'J/psi:eebrems', 'psi(2S):eebrems',
                'J/psi:eebrems2', 'psi(2S):eebrems2',
                'J/psi:mumu', 'psi(2S):mumu']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        # [Y(3S) -> pi+pi- [Y(1S,2S) -> mu+mu-]] decay
        ma.reconstructDecay('J/psi:mumu_test -> mu+:loosepid mu-:loosepid', '', path=path)
        ma.reconstructDecay('J/psi:ee_test -> e+:loosepid_noTOP e-:loosepid_noTOP', '', path=path)
        ma.copyList('J/psi:ll', 'J/psi:mumu_test', path=path)
        ma.copyList('J/psi:ll', 'J/psi:ee_test', path=path)

        # Print histograms.
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='J/psi:ll',
            variables_1d=[(
                'InvM', 65, 2.7, 4.0,
                'J/#psi mass',
                __liaison__,
                'J/psi mass',
                'J/psi peak is seen.',
                'M [GeV/c^{2}]', 'Events / (20 MeV/c^{2})',
                'shifter'
            )],
            path=path
        )


@fancy_skim_header
class InclusiveLambda(BaseSkim):
    """
    Reconstructed decay
    * :math:`\\Lambda \\to p \\pi^-` (and charge conjugate)

    Selection criteria:
    * proton
    ``protonID > 0.1``
    * Lambda:
    ``cosAngleBetweenMomentumAndVertexVector > 0.99``
    ``flightDistance/flightDistanceErr > 3.``
    * ``0.6 < p,proton/p,Lambda < 1.0 GeV/c``

    """
    __authors__ = ["Bianca Scavino"]
    __description__ = "Inclusive Lambda skim"
    __contact__ = __liaison__
    __category__ = "physics, quarkonium"

    def load_standard_lists(self, path):
        stdLambdas(path=path)

    def build_lists(self, path):

        # Add useful alias
        v.addAlias("protonID_proton", "daughter(0, protonID)")
        v.addAlias("momRatio_protonLambda", "formula(daughter(0, p)/p)")
        v.addAlias('flightSignificance', 'formula(flightDistance/flightDistanceErr)')

        # Apply selection to Lambdas
        ma.applyCuts("Lambda0:merged", "cosAngleBetweenMomentumAndVertexVector > 0.99", path=path)
        ma.applyCuts("Lambda0:merged", "0.6 < momRatio_protonLambda < 1.", path=path)
        ma.applyCuts("Lambda0:merged", "flightSignificance > 3.", path=path)
        ma.applyCuts("Lambda0:merged", "protonID_proton > 0.1", path=path)

        # Return the lists.
        return ["Lambda0:merged"]
