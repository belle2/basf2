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

    # no good tracks in the event
    cleaned = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15'  # cm, cm, GeV/c
    applyEventCuts('nCleanedTracks(' + cleaned + ') < 1')

    # no other photon above 100 MeV
    angle = '0.296706 < theta < 2.61799'  # rad, (17 -- 150 deg)
    minimum = 'E > 0.1'  # GeV
    cutAndCopyList('gamma:100', 'gamma:all', minimum + ' and ' + angle)
    applyEventCuts('0 < nParticlesInList(gamma:100) < 2')

    # all remaining single photon events (== candidates) with region
    # dependent minimum energy in GeV
    region_dependent = ' [clusterReg ==  2 and E > 1.0] or '  # barrel
    region_dependent += '[clusterReg ==  1 and E > 2.0] or '  # fwd
    region_dependent += '[clusterReg ==  3 and E > 2.0] or '  # bwd
    region_dependent += '[clusterReg == 11 and E > 2.0] or '  # between fwd and barrel
    region_dependent += '[clusterReg == 13 and E > 2.0] '     # between bwd and barrel
    cutAndCopyList('gamma:singlePhoton', 'gamma:100', region_dependent)
    return ['gamma:singlePhoton']
