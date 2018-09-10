#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 26 Jun 2018
#######################################################

from basf2 import *
from modularAnalysis import *


def CharmlessHad2BodyB0List():
    """
    Skim list definitions for all neutral B to charmless 2 body modes.

    **Skim Author**: Kim Smith (khsmith@student.unimelb.edu.au)

    **Skim Name**: CharmlessHad2Body

    **Working Group**: BtoCharmless (WG9)

    **Decay Modes**:
    1. B0 -> pi+ pi-
    2. B0 -> pi0 pi0
    3. B0 -> K+ pi-
    4. B0 -> K+ K-
    5. B0 -> eta pi0
    6. B0 -> eta eta
    7. B0 -> K_S0 pi0
    8. B0 -> rho0 pi0
    9. B0 -> rho- pi+
    10. B0 -> K+ rho-
    11. B0 -> eta rho0
    12. B0 -> omega eta
    13. B0 -> phi pi0
    14. B0 -> phi eta
    15. B0 -> omega pi0
    16. B0 -> eta K*0
    17. B0 -> K*+ pi-
    18. B0 -> K*0 pi0
    19. B0 -> K*+ K-
    20. B0 -> rho0 rho0
    21. B0 -> rho+ rho-
    22. B0 -> omega rho0
    23. B0 -> omega omega
    24. B0 -> phi rho0
    25. B0 -> phi omega
    26. B0 -> phi phi
    27. B0 -> omega K*0
    28. B0 -> K*0 rho0
    29. B0 -> K*+ rho-
    30. B0 -> K*0 phi
    31. B0 -> K*0 anti-K*0
    32. B0 -> K*0 K*0
    33. B0 -> K*+ K*-
    34. B0 -> eta f_0
    35. B0 -> omega f_0
    36. B0 -> phi f_0
    37. B0 -> rho0 f_0
    38. B0 -> K*0 f_0
    39. B0 -> f_0 f_0

    **Particle Lists**: Standard loose lists for all light mesons.
    Modified K_S0:all with tighter invariant mass, 0.48 < M < 0.51.
    Modified pi0:skim with tighter invariant mass and minimum energy, 0.12 < M < 0.14 and E > 0.25.

    **Additional Cuts**:
    1. R2EventLevel < 0.5 (event level)
    2. nTracks >= 2 (event level)
    3. 5.24 < Mbc < 5.29
    4. abs(deltaE) < 0.5

    Note:
      Does not include decay modes involving eta prime at the current time.
    """

    applyEventCuts('R2EventLevel < 0.5 and nTracks >= 2')

    cutAndCopyList('pi0:hbd', 'pi0:skim', '0.12 < M < 0.14 and E > 0.25')
    cutAndCopyList('K_S0:hbd', 'K_S0:all', '0.48 < M < 0.51')

    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_PPChannels = ['pi+:loose pi-:loose',  # 1
                     'pi0:hbd pi0:hbd',  # 2
                     'K+:loose pi-:loose',  # 3
                     'K+:loose K-:loose',  # 4
                     'eta:loose pi0:hbd',  # 5
                     'eta:loose eta:loose',  # 6
                     'K_S0:hbd pi0:hbd']  # 7

    B0_PVChannels = ['rho0:loose pi0:hbd',  # 1
                     'rho-:loose pi+:loose',  # 2
                     'K+:loose rho-:loose',  # 3
                     'eta:loose rho0:loose',  # 4
                     'omega:loose eta:loose',  # 5
                     'phi:loose pi0:hbd',  # 6
                     'phi:loose eta:loose',  # 7
                     'omega:loose pi0:hbd',  # 8
                     'eta:loose K*0:loose',  # 9
                     'K*+:loose pi-:loose',  # 10
                     'K*0:loose pi0:hbd',  # 11
                     'K*+:loose K-:loose']  # 12

    B0_VVChannels = ['rho0:loose rho0:loose',  # 1
                     'rho+:loose rho-:loose',  # 2
                     'omega:loose rho0:loose',  # 3
                     'omega:loose omega:loose',  # 4
                     'phi:loose rho0:loose',  # 5
                     'phi:loose omega:loose',  # 6
                     'phi:loose phi:loose',  # 7
                     'omega:loose K*0:loose',  # 8
                     'K*0:loose rho0:loose',  # 9
                     'K*+:loose rho-:loose',  # 10
                     'K*0:loose phi:loose',  # 11
                     'K*0:loose anti-K*0:loose',  # 12
                     'K*0:loose K*0:loose',  # 13
                     'K*+:loose K*-:loose']  # 14

    B0_PSChannels = ['eta:loose f_0:loose']  # 1

    B0_VSChannels = ['omega:loose f_0:loose',  # 1
                     'phi:loose f_0:loose',  # 2
                     'rho0:loose f_0:loose',  # 3
                     'K*0:loose f_0:loose']  # 4

    B0_SSChannels = ['f_0:loose f_0:loose']  # 1

    B0PPList = []
    for chID, channel in enumerate(B0_PPChannels):
        reconstructDecay('B0:HAD_b2PP' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PPList.append('B0:HAD_b2PP' + str(chID))

    B0PVList = []
    for chID, channel in enumerate(B0_PVChannels):
        reconstructDecay('B0:HAD_b2PV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PVList.append('B0:HAD_b2PV' + str(chID))

    B0VVList = []
    for chID, channel in enumerate(B0_VVChannels):
        reconstructDecay('B0:HAD_b2VV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0VVList.append('B0:HAD_b2VV' + str(chID))

    B0PSList = []
    for chID, channel in enumerate(B0_PSChannels):
        reconstructDecay('B0:HAD_b2PS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PSList.append('B0:HAD_b2PS' + str(chID))

    B0VSList = []
    for chID, channel in enumerate(B0_VSChannels):
        reconstructDecay('B0:HAD_b2VS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0VSList.append('B0:HAD_b2VS' + str(chID))

    B0SSList = []
    for chID, channel in enumerate(B0_SSChannels):
        reconstructDecay('B0:HAD_b2SS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0SSList.append('B0:HAD_b2SS' + str(chID))

    copyLists('B0:2BodyB0', B0PPList + B0PVList + B0VVList + B0PSList + B0VSList + B0SSList)

    List = ['B0:2BodyB0']
    return List


def CharmlessHad2BodyBmList():
    """
    Skim list definitions for all charged B to charmless 2 body modes.

    **Skim Author**: Kim Smith (khsmith@student.unimelb.edu.au)

    **Skim Name**: CharmlessHad2Body

    **Working Group**: BtoCharmless (WG9)

    **Decay Modes**:
    1. B- -> pi- pi0
    2. B- -> K- pi0
    3. B- -> eta pi-
    4. B- -> eta K-
    5. B- -> rho0 pi-
    6. B- -> rho- pi0
    7. B- -> K- rho0
    8. B- -> omega pi-
    9. B- -> eta rho-
    10. B- -> phi pi-
    11. B- -> eta K*-
    12. B- -> omega K-
    13. B- -> K*0 pi-
    14. B- -> K*- pi0
    15. B- -> K- phi
    16. B- -> rho- rhro0
    17. B- -> omega rho-
    18. B- -> phi rho-
    19. B- -> omega K*-
    20. B- -> K*- rho0
    21. B- -> K*0 rho-
    22. B- -> K*- K*0
    23. B- -> K*- phi
    24. B- -> pi- f_0
    25. B- -> f_0 K-
    26. B- -> rho- f_0
    27. B- -> K*- f_0

    **Particle Lists**: Standard loose lists for all light mesons.
    Modified K_S0:all with tighter invariant mass, 0.48 < M < 0.51.
    Modified pi0:skim with tighter invariant mass and minimum energy, 0.12 < M < 0.14 and E > 0.25.

    **Additional Cuts**:
    1. R2EventLevel < 0.5 (event level)
    2. nTracks >= 2 (event level)
    3. 5.24 < Mbc < 5.29
    4. abs(deltaE) < 0.5

    Note:
      Does not include decay modes involving eta prime at the current time.
    """

    applyEventCuts('R2EventLevel < 0.5 and nTracks >= 2')

    cutAndCopyList('pi0:hbd', 'pi0:skim', '0.12 < M < 0.14 and E > 0.25')
    cutAndCopyList('K_S0:hbd', 'K_S0:all', '0.48 < M < 0.51')

    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    Bm_PPChannels = ['pi-:loose pi0:hbd',  # 1
                     'K-:loose pi0:hbd',  # 2
                     'eta:loose pi-:loose',  # 3
                     'eta:loose K-:loose']  # 4

    Bm_PVChannels = ['rho0:loose pi-:loose',  # 1
                     'rho-:loose pi0:hbd',  # 2
                     'K-:loose rho0:loose',  # 3
                     'omega:loose pi-:loose',  # 4
                     'eta:loose rho-:loose',  # 5
                     'phi:loose pi-:loose',  # 6
                     'eta:loose K*-:loose',  # 7
                     'omega:loose K-:loose',  # 8
                     'K*0:loose pi-:loose',  # 9
                     'K*-:loose pi0:hbd',  # 10
                     'K-:loose phi:loose']  # 11

    Bm_VVChannels = ['rho-:loose rho0:loose',  # 1
                     'omega:loose rho-:loose',  # 2
                     'phi:loose rho-:loose',  # 3
                     'omega:loose K*-:loose',  # 4
                     'K*-:loose rho0:loose',  # 5
                     'K*0:loose rho-:loose',  # 6
                     'K*-:loose K*0:loose',  # 7
                     'K*-:loose phi:loose']  # 8

    Bm_PSChannels = ['pi-:loose f_0:loose',  # 1
                     'f_0:loose K-:loose']  # 2

    Bm_VSChannels = ['rho-:loose f_0:loose',  # 1
                     'K*-:loose f_0:loose']  # 2

    BmPPList = []
    for chID, channel in enumerate(Bm_PPChannels):
        reconstructDecay('B-:HAD_b2PP' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPPList.append('B-:HAD_b2PP' + str(chID))

    BmPVList = []
    for chID, channel in enumerate(Bm_PVChannels):
        reconstructDecay('B-:HAD_b2PV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPVList.append('B-:HAD_b2PV' + str(chID))

    BmVVList = []
    for chID, channel in enumerate(Bm_VVChannels):
        reconstructDecay('B-:HAD_b2VV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmVVList.append('B-:HAD_b2VV' + str(chID))

    BmPSList = []
    for chID, channel in enumerate(Bm_PSChannels):
        reconstructDecay('B-:HAD_b2PS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPSList.append('B-:HAD_b2PS' + str(chID))

    BmVSList = []
    for chID, channel in enumerate(Bm_VSChannels):
        reconstructDecay('B-:HAD_b2VS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmVSList.append('B-:HAD_b2VS' + str(chID))

    copyLists('B-:2BodyBm', BmPPList + BmPVList + BmVVList + BmPSList + BmVSList)

    List = ['B-:2BodyBm']
    return List
