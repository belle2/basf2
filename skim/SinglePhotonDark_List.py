#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Single photon skim list for the dark photon analysis in WG8: Dark/low multi.
Skim LFN code:   18020100
Physics channel: ee → A'γ; A' → invisible
"""

__author__ = "Sam Cunliffe"


from modularAnalysis import cutAndCopyList, applyEventCuts


def SinglePhotonDarkList():
    """Single photon skim list"""

    # no clean tracks in the event
    applyEventCuts('nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) < 1')

    # no other photon above 150 MeV
    cutAndCopyList('gamma:150', 'gamma:all', 'E > 0.1')
    applyEventCuts('0 < nParticlesInList(gamma:150) < 2')

    # all remaining single photon events with region dependent energy cuts
    region_dependent = ' [clusterReg == 2 and E > 1.0] or '  # barrel
    region_dependent += '[clusterReg == 1 and E > 2.0] or '  # fwd
    region_dependent += '[clusterReg == 3 and E > 2.0]'      # bwd
    cutAndCopyList('gamma:singlePhoton', 'gamma:150', region_dependent)
    return ['gamma:singlePhoton']
