#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
D* Veto reconstruction for ModeSelector.

This module reconstructs D* candidates from D mesons (daughters of B candidates)
combined with soft pions or pi0s from the Rest of Event. The reconstructed
D* mass difference (deltaMassDiff) and vertex fit chi2 probability are stored
as ExtraInfo on the B candidates.

This helps identify B -> D* X decays that were reconstructed as B -> D X
by the FEI, which the ModeSelector uses as input features (feature blocks 5-8).
"""

import math

import basf2 as b2
import modularAnalysis as ma
from ROOT import Belle2
from variables import variables as vm
from vertex import kFit, treeFit


class _SetDstarVetoDefaults(b2.Module):
    """Set missing D* veto ExtraInfo keys to NaN on all B candidates.

    This ensures every B candidate has Dstp_deltaMassDiff and Dst0_deltaMassDiff
    set (even when the corresponding D* type is not its first daughter and no veto
    candidate is reconstructed).
    """

    def __init__(self, particle_lists, keys):
        """Set up with lists of particle list names and ExtraInfo key names."""
        super().__init__()
        #: Particle list names to iterate over
        self._particle_lists = particle_lists
        #: ExtraInfo key names to set to NaN if missing
        self._keys = keys

    def event(self):
        """Set missing ExtraInfo keys to NaN for every candidate in each list."""
        for list_name in self._particle_lists:
            plist = Belle2.PyStoreObj(list_name)
            if not plist.isValid():
                continue
            for i in range(plist.obj().getListSize()):
                p = plist.obj().getParticle(i)
                for key in self._keys:
                    if not p.hasExtraInfo(key):
                        p.addExtraInfo(key, math.nan)


def add_dstar_veto_aliases():
    """
    Add variable aliases needed for D* veto reconstruction.
    """
    # D* - D mass difference using invariant mass
    vm.addAlias('trueM', 'M - dM')
    vm.addAlias('massDiffInvM', 'formula(InvM - daughter(0, M))')
    vm.addAlias('trueMassDiff', 'trueM - daughter(0,trueM)')
    vm.addAlias('deltaMassDiffInvM', 'massDiffInvM - trueMassDiff')
    vm.addAlias('deltaMassDiff', 'massDifference(0) - trueMassDiff')

    vm.addAlias('dmID', 'extraInfo(decayModeID)')


def addDstarVeto(
    particleLists,
    path: b2.Path = None,
    deltaMassDiffCut: tuple = (-0.02, 0.02),
    dMassCut: tuple = (-0.03, 0.03),
    writeExtraInfo: bool = True,
    skipTreeFit: bool = True
):
    """
    Add D* veto reconstruction to the path for B meson particle lists.

    This function reconstructs D* candidates by combining D mesons (first daughter
    of B candidates) with soft pions or pi0s from the Rest of Event, to identify
    cases where the FEI reconstructed B -> D X but the true decay was B -> D* X.

    For B candidates with D0 as first daughter:
        - D*+ -> D0 pi+ (from ROE)
        - D*0 -> D0 pi0 (from ROE)

    For B candidates with D+ as first daughter:
        - D*+ -> D+ pi0 (from ROE)

    Parameters:
        particleLists (str or list): Name(s) of B meson particle list(s)
            (e.g., 'B+:feiHadronic' or ['B+:feiHadronic', 'B0:feiHadronic'])
        path (basf2.Path): The basf2 path to add modules to.
        deltaMassDiffCut (tuple): Cut on deltaMassDiff (D* mass diff - true mass diff) in GeV.
        dMassCut (tuple): Cut on D and D* mass deviation (dM) in GeV.
        writeExtraInfo (bool): Whether to write ExtraInfo to particles.
        skipTreeFit (bool): If True, skip the vertex TreeFit (significant speedup). The
            deltaMassDiff will use InvM-based computation instead of fit-based, and chiProb
            will not be available (stored as NaN). Candidates are ranked by
            abs(deltaMassDiffInvM) instead of chiProb. Default: True

    The following ExtraInfo fields are added to B candidates:

    For D0 daughter (D*+ and D*0 veto):
        - Dstp_deltaMassDiff: Delta mass difference for D*+ -> D0 pi+
        - Dstp_chiProb: Vertex fit chi2 probability for D*+ (NaN if skipTreeFit)
        - Dst0_deltaMassDiff: Delta mass difference for D*0 -> D0 pi0
        - Dst0_chiProb: Vertex fit chi2 probability for D*0 (NaN if skipTreeFit)

    For D+ daughter (D*+ veto only):
        - Dstp_deltaMassDiff: Delta mass difference for D*+ -> D+ pi0
        - Dstp_chiProb: Vertex fit chi2 probability for D*+ (NaN if skipTreeFit)
    """
    if path is None:
        b2.B2FATAL("Path is required for addDstarVeto")

    if isinstance(particleLists, str):
        particleLists = [particleLists]

    add_dstar_veto_aliases()

    # ExtraInfo variable mappings
    extra_info_dstp = {
        'daughter(0,chiProb)': 'Dstp_chiProb',
        'daughter(0,deltaMassDiff)': 'Dstp_deltaMassDiff',
    }
    extra_info_dst0 = {
        'daughter(0,chiProb)': 'Dst0_chiProb',
        'daughter(0,deltaMassDiff)': 'Dst0_deltaMassDiff',
    }

    # For veto reconstruction, use InvM-based deltaMassDiff
    extra_info_veto_dstp = {
        'daughter(0,deltaMassDiffInvM)': 'Dstp_deltaMassDiff',
    }
    extra_info_veto_dst0 = {
        'daughter(0,deltaMassDiffInvM)': 'Dst0_deltaMassDiff',
    }
    if not skipTreeFit:
        extra_info_veto_dstp['daughter(0,chiProb)'] = 'Dstp_chiProb'
        extra_info_veto_dst0['daughter(0,chiProb)'] = 'Dst0_chiProb'

    # Create pi+ list for D*+ -> D0 pi+ reconstruction
    from_ip = "[[dr < 2] and [abs(dz) < 4]]"
    p_cut = " and [p > 0.05] and [useCMSFrame(p) < 0.5]"
    ma.fillParticleList('pi+:forDstVeto', from_ip + p_cut, path=path)

    # Create pi0 list for D* veto reconstruction. The lists are private to the veto,
    # so they cannot collide with a list of the same name created elsewhere, e.g. one
    # without the photon MVA, whose suppression cuts would then silently reject all pi0s.
    # The selection reproduces the 50% efficiency pi0 selection optimised in May 2020
    # (photon and pi0 cuts, mass-constrained fit) that the models were trained with.
    photon_cuts = '[clusterNHits > 1.5] and thetaInCDCAcceptance and ' \
        '[[clusterReg == 1 and E > 0.025] or [clusterReg == 2 and E > 0.025] or [clusterReg == 3 and E > 0.040]]'
    ma.fillParticleList('gamma:forDstVeto', photon_cuts, path=path)
    # Photon MVA weights the training was done with (MC16rd)
    ma.getBeamBackgroundProbability('gamma:forDstVeto', weight='MC16rd', path=path)
    ma.getFakePhotonProbability('gamma:forDstVeto', weight='MC16rd', path=path)
    ma.reconstructDecay('pi0:forDstVeto -> gamma:forDstVeto gamma:forDstVeto', '0.105 < InvM < 0.150', dmID=1, path=path)
    kFit('pi0:forDstVeto', 0.0, 'mass', path=path)

    # Apply additional pi0 cuts (matching training preprocessing)
    pi0Cuts = '[useCMSFrame(p) < 0.5]'
    pi0Cuts += ' and [daughter(0,beamBackgroundSuppression) > 0.5] and [daughter(0,fakePhotonSuppression) > 0.1]'
    pi0Cuts += ' and [daughter(1,beamBackgroundSuppression) > 0.5] and [daughter(1,fakePhotonSuppression) > 0.1]'

    ma.cutAndCopyList('pi0:dstarVeto', 'pi0:forDstVeto', pi0Cuts, path=path)

    # --- Process each particle list ---
    for particleList in particleLists:
        particle_type = particleList.split(':')[0]  # e.g., 'B+' or 'B0'
        list_label = particleList.split(':')[1] if ':' in particleList else ''

        # --- Process B candidates with D*+ or D*0 as first daughter ---
        # These already have the correct mass difference, just store it
        dstp_daughter_list = f'{particle_type}:Dstp_daughter_{list_label}'
        dst0_daughter_list = f'{particle_type}:Dst0_daughter_{list_label}'

        ma.cutAndCopyList(dstp_daughter_list, particleList,
                          '[abs(daughter(0,PDG)) == 413]', path=path)
        ma.cutAndCopyList(dst0_daughter_list, particleList,
                          '[abs(daughter(0,PDG)) == 423]', path=path)

        if writeExtraInfo:
            ma.variablesToExtraInfo(dstp_daughter_list, extra_info_dstp, option=0, path=path)
            ma.variablesToExtraInfo(dst0_daughter_list, extra_info_dst0, option=0, path=path)

        # --- Process B candidates with D0 or D+ as first daughter (veto) ---
        for d_pdg, d_str in [(421, 'D0'), (411, 'Dp')]:
            channel_name = f'{particle_type}:{d_str}_daughter_{list_label}'
            d_particle = 'D0' if d_pdg == 421 else 'D+'

            ma.cutAndCopyList(channel_name, particleList,
                              f'abs(daughter(0,PDG)) == {d_pdg}', path=path)

            # Build ROE for these candidates
            ma.buildRestOfEvent(channel_name, path=path)

            # Create ROE path
            roe_path = b2.Path()
            dead_end_path = b2.Path()

            ma.signalSideParticleFilter(channel_name, '', roe_path, dead_end_path)

            # Get particles from ROE or direct B daughters
            roe_condition = f'[isInRestOfEvent == 1] or [isDescendantOfList({channel_name},1) == 1]'
            ma.cutAndCopyList('pi0:roe', 'pi0:dstarVeto', roe_condition, path=roe_path)

            if d_str == 'D0':
                # D0 can form D*+ (with pi+) or D*0 (with pi0)
                dst_daughters = ['pi+', 'pi0']
                ma.cutAndCopyList('pi+:roe', 'pi+:forDstVeto', roe_condition, path=roe_path)
            else:
                # D+ can only form D*+ (with pi0)
                dst_daughters = ['pi0']

            # Fill signal side D
            ma.fillSignalSideParticleList(f'{d_particle}:sig', f'{particle_type} -> ^{d_particle}',
                                          path=roe_path)

            dstp_lists = []
            dst0_lists = []

            for i, dst_daughter in enumerate(dst_daughters):
                if d_str == 'Dp' or (d_str == 'D0' and dst_daughter == 'pi+'):
                    dst_list = f'D*+:veto_{i}'
                else:
                    dst_list = f'D*0:veto_{i}'

                ma.reconstructDecay(f'{dst_list} -> {d_particle}:sig {dst_daughter}:roe',
                                    '', dmID=i, path=roe_path)

                # Mass window cuts
                cut_str = f'[{deltaMassDiffCut[0]} < deltaMassDiffInvM < {deltaMassDiffCut[1]}]'
                cut_str += f' and [{dMassCut[0]} < dM < {dMassCut[1]}]'
                cut_str += f' and [{dMassCut[0]} < daughter(0,dM) < {dMassCut[1]}]'
                ma.applyCuts(dst_list, cut_str, path=roe_path)

                # Rank by best candidate
                if dst_daughter == 'pi0':
                    ma.rankByHighest(dst_list, 'daughter(1,chiProb)', 1, path=roe_path)
                elif dst_daughter == 'pi+':
                    if skipTreeFit:
                        ma.rankByLowest(dst_list, 'abs(deltaMassDiffInvM)', 1, path=roe_path)
                    # else: will rank by vertex fit quality after fit

                if not skipTreeFit:
                    # Vertex fit with mass constraints
                    treeFit(
                        list_name=dst_list,
                        conf_level=0,
                        ipConstraint=False,
                        updateAllDaughters=False,
                        massConstraint=["D*+", "D*0", "D+", "D0", "K_S0", "pi0"],
                        path=roe_path,
                    )

                    if dst_daughter == 'pi+':
                        ma.rankByHighest(dst_list, 'chiProb', 1, path=roe_path)

                if 'D*+:veto' in dst_list:
                    dstp_lists.append(dst_list)
                else:
                    dst0_lists.append(dst_list)

            # Merge D* lists
            if dstp_lists:
                ma.copyLists('D*+:veto', dstp_lists, writeOut=False, path=roe_path)
                ma.applyCuts('D*+:veto', 'useCMSFrame(p) < 3', path=roe_path)
                ma.rankByLowest('D*+:veto', 'dmID', 1, path=roe_path)

            if dst0_lists:
                ma.copyLists('D*0:veto', dst0_lists, writeOut=False, path=roe_path)
                ma.applyCuts('D*0:veto', 'useCMSFrame(p) < 3', path=roe_path)
                ma.rankByLowest('D*0:veto', 'dmID', 1, path=roe_path)

            # Create dummy particles to transfer ExtraInfo back to signal side
            if dstp_lists:
                ma.reconstructDecay('Xsd:dstp -> D*+:veto', '', allowChargeViolation=True, path=roe_path)
                roe_path.modules()[-1].set_log_level(b2.LogLevel.ERROR)
                if writeExtraInfo:
                    ma.variableToSignalSideExtraInfo('Xsd:dstp', extra_info_veto_dstp, path=roe_path)

            if dst0_lists:
                ma.reconstructDecay('Xsd:dst0 -> D*0:veto', '', allowChargeViolation=True, path=roe_path)
                roe_path.modules()[-1].set_log_level(b2.LogLevel.ERROR)
                if writeExtraInfo:
                    ma.variableToSignalSideExtraInfo('Xsd:dst0', extra_info_veto_dst0, path=roe_path)

            # Execute ROE path
            path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

        if writeExtraInfo:
            # After all real values are set, fill any remaining missing deltaMassDiff
            # keys with NaN. This ensures every B candidate has these keys so that
            # ModeSelectorModule's hasExtraInfo check does not FATAL. NaN is converted
            # to None in ModeSelectorModule and stored as 0 in the sparse feature matrix,
            # matching the behaviour of a genuinely absent veto candidate.
            path.add_module(_SetDstarVetoDefaults(
                [particleList],
                ['Dstp_deltaMassDiff', 'Dst0_deltaMassDiff'],
            ))

    b2.B2INFO(f"DstarVeto: Added D* veto reconstruction for {particleLists}")
