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
by the FEI, which is important for the ModeSelector neural network.
"""

import basf2 as b2
import modularAnalysis as ma
import vertex
from variables import variables as vm


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
    particleList: str,
    pi0List: str = 'pi0:eff50_May2020Fit',
    path: b2.Path = None,
    deltaMassDiffCut: tuple = (-0.02, 0.02),
    dMassCut: tuple = (-0.03, 0.03),
    writeExtraInfo: bool = True
):
    """
    Add D* veto reconstruction to the path for a B meson particle list.

    This function reconstructs D* candidates by combining D mesons (first daughter
    of B candidates) with soft pions or pi0s from the Rest of Event. The goal is
    to identify cases where the FEI reconstructed B -> D X but the true decay
    was B -> D* X.

    For B candidates with D0 as first daughter:
        - D*+ -> D0 pi+ (from ROE)
        - D*0 -> D0 pi0 (from ROE)

    For B candidates with D+ as first daughter:
        - D*+ -> D+ pi0 (from ROE)

    Parameters
    ----------
    particleList : str
        Name of the B meson particle list (e.g., 'B+:feiHadronic')
    pi0List : str
        Name of the pi0 list to use for D*0 reconstruction
    path : basf2.Path
        The basf2 path to add modules to
    deltaMassDiffCut : tuple
        Cut on deltaMassDiff (D* mass diff - true mass diff) in GeV
    dMassCut : tuple
        Cut on D and D* mass deviation (dM) in GeV
    writeExtraInfo : bool
        Whether to write ExtraInfo to particles

    The following ExtraInfo fields are added to B candidates:

    For D0 daughter (D*+ and D*0 veto):
        - Dstp_deltaMassDiff: Delta mass difference for D*+ -> D0 pi+
        - Dstp_chiProb: Vertex fit chi2 probability for D*+
        - Dst0_deltaMassDiff: Delta mass difference for D*0 -> D0 pi0
        - Dst0_chiProb: Vertex fit chi2 probability for D*0

    For D+ daughter (D*+ veto only):
        - Dstp_deltaMassDiff: Delta mass difference for D*+ -> D+ pi0
        - Dstp_chiProb: Vertex fit chi2 probability for D*+
    """
    if path is None:
        b2.B2FATAL("Path is required for addDstarVeto")

    add_dstar_veto_aliases()

    # Extract list name parts
    particle_type = particleList.split(':')[0]  # e.g., 'B+' or 'B0'
    list_label = particleList.split(':')[1] if ':' in particleList else ''

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
        'daughter(0,chiProb)': 'Dstp_chiProb',
        'daughter(0,deltaMassDiffInvM)': 'Dstp_deltaMassDiff',
    }
    extra_info_veto_dst0 = {
        'daughter(0,chiProb)': 'Dst0_chiProb',
        'daughter(0,deltaMassDiffInvM)': 'Dst0_deltaMassDiff',
    }

    # Create pi+ list for D*+ -> D0 pi+ reconstruction
    from_ip = "[[dr < 2] and [abs(dz) < 4]]"
    p_cut = " and [p > 0.05] and [useCMSFrame(p) < 0.5]"
    ma.fillParticleList('pi+:forDstVeto', from_ip + p_cut, path=path)

    # Copy pi0 list
    ma.copyList('pi0:forDstVeto', pi0List, path=path)

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
        ma.cutAndCopyList('pi0:roe', 'pi0:forDstVeto', roe_condition, path=roe_path)

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
                # Will rank by vertex fit quality after fit
                pass

            # Vertex fit with mass constraints
            vertex.treeFit(
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
            if writeExtraInfo:
                ma.variableToSignalSideExtraInfo('Xsd:dstp', extra_info_veto_dstp, path=roe_path)

        if dst0_lists:
            ma.reconstructDecay('Xsd:dst0 -> D*0:veto', '', allowChargeViolation=True, path=roe_path)
            if writeExtraInfo:
                ma.variableToSignalSideExtraInfo('Xsd:dst0', extra_info_veto_dst0, path=roe_path)

        # Execute ROE path
        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    b2.B2INFO(f"DstarVeto: Added D* veto reconstruction for {particleList}")
