#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma
from stdPi0s import stdPi0s


def loadPiForBtoHadrons(path):
    """
    Creates a ``pi+:GoodTrack`` list, with cuts :math:`|dr|<2~{\\rm cm}` and :math:`|dz|<5~{\\rm cm}`.

    Parameters
        path (basf2.Path): Skim path to be processed.
    """
    ma.fillParticleList("pi+:GoodTrack", "abs(dr) < 2 and abs(dz) < 5", path=path)


def loadPiSkimHighEff(path):
    """
    Creates a ``pi+:SkimHighEff`` list, with cuts :math:`|dr|<2~{\\rm cm}` and :math:`|dz|<5~{\\rm cm} and pionID >= 0.01`.

    Parameters
        path (basf2.Path): Skim path to be processed.
    """
    ma.fillParticleList("pi+:SkimHighEff", "abs(dr) < 2 and abs(dz) < 5 and pionID >= 0.01", path=path)


def loadSlowPi(path):
    """
    Creates a ``pi+:slowPi`` list, with cuts :math:`|dr|<2~{\\rm cm}` and :math:`|dz|<5~{\\rm cm} and useCMSFrame(p) < 0.4`.

    Parameters
        path (basf2.Path): Skim path to be processed.
    """
    ma.fillParticleList("pi+:slowPi", "abs(dr) < 2 and abs(dz) < 5 and useCMSFrame(p) < 0.4", path=path)


def loadKForBtoHadrons(path):
    """
    Creates a ``K+:GoodTrack`` list, with cuts :math:`|dr|<2~{\\rm cm}` and :math:`|dz|<5~{\\rm cm}`.

    Parameters
        path (basf2.Path): Skim path to be processed.
    """
    ma.fillParticleList("K+:GoodTrack", "abs(dr) < 2 and abs(dz) < 5", path=path)


def loadKSkimHighEff(path):
    """
    Creates a ``K+:SkimHighEff`` list, with cuts :math:`|dr|<2~{\\rm cm}` and :math:`|dz|<5~{\\rm cm} and kaonID >= 0.01`.

    Parameters
        path (basf2.Path): Skim path to be processed.
    """
    ma.fillParticleList("K+:SkimHighEff", "abs(dr) < 2 and abs(dz) < 5 and kaonID >= 0.01", path=path)


def loadStdD0(path):
    """
    Creates a 'D0:all' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K-:GoodTrack pi+:GoodTrack',
      2. 'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi-:GoodTrack',
      3. 'K-:GoodTrack pi+:GoodTrack pi0:bth_skim',
      4. 'K_S0:merged pi+:GoodTrack pi-:GoodTrack'.

    @param path     modules are added to this path
    """
    D0Cuts = '1.7 < M < 2.0'
    D0_Channels = ['K-:GoodTrack pi+:GoodTrack',
                   'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi-:GoodTrack',
                   'K-:GoodTrack pi+:GoodTrack pi0:bth_skim',
                   'K_S0:merged pi+:GoodTrack pi-:GoodTrack',
                   ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay(decayString='D0:all' + str(chID) + ' -> ' + channel, cut=D0Cuts, dmID=chID, path=path)
        D0List.append('D0:all' + str(chID))
    ma.copyLists(outputListName='D0:all', inputListNames=D0List, path=path)
    return D0List


def loadStdD0_Kpi(path=None):
    """
    Creates a 'D0:Kpi' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K-:GoodTrack pi+:GoodTrack',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpi -> K-:GoodTrack pi+:GoodTrack', cut='1.7 < M < 2.0', dmID=1, path=path)
    return ['D0:Kpi']


def loadSkimHighEffD0_Kpi(path=None):
    """
    Creates a 'D0:Kpi_skimhigheff' list, with an invariant mass in the range :math:`1.8 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K-:SkimHighEff pi+:SkimHighEff',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpi_skimhigheff -> K-:SkimHighEff pi+:SkimHighEff', cut='1.8 < M < 2.0', dmID=1, path=path)
    return ['D0:Kpi_skimhigheff']


def loadStdD0_Kpipipi(path=None):
    """
    Creates a 'D0:Kpipipi' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      2. 'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi-:GoodTrack',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpipipi -> K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi-:GoodTrack',
                        cut='1.7 < M < 2.0', dmID=2, path=path)
    return ['D0:Kpipipi']


def loadSkimHighEffD0_Kpipipi(path=None):
    """
    Creates a 'D0:Kpipipi_skimhigheff' list, with an invariant mass in the range :math:`1.8 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      2. 'K-:SkimHighEff pi+:SkimHighEff pi+:SkimHighEff pi-:SkimHighEff',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpipipi_skimhigheff -> K-:SkimHighEff pi+:SkimHighEff pi+:SkimHighEff pi-:SkimHighEff',
                        cut='1.8 < M < 2.0', dmID=2, path=path)
    return ['D0:Kpipipi_skimhigheff']


def loadStdD0_Kpipi0(path=None):
    """
    Creates a 'D0:Kpipi0' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      3. 'K-:GoodTrack pi+:GoodTrack pi0:bth_skim',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpipi0 -> K-:GoodTrack pi+:GoodTrack pi0:bth_skim', cut='1.7 < M < 2.0', dmID=3, path=path)
    return ['D0:Kpipi0']


def loadStdD0_eff20_Kpipi0(path=None):
    """
    Creates a 'D0:Kpipi0_eff20' list, with an invariant mass in the range :math:`1.8 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      3. 'K-:SkimHighEff pi+:SkimHighEff pi0:eff20_May2020',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(
        decayString='D0:Kpipi0_eff20 -> K-:SkimHighEff pi+:SkimHighEff pi0:eff20_May2020',
        cut='1.8 < M < 2.0',
        dmID=3,
        path=path)
    return ['D0:Kpipi0_eff20']


def loadStdD0_Kspipi(path=None):
    """
    Creates a 'D0:Kspipi' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      4. 'K_S0:merged pi+:GoodTrack pi-:GoodTrack'.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kspipi -> K_S0:merged pi+:GoodTrack pi-:GoodTrack', cut='1.7 < M < 2.0', dmID=4, path=path)
    return ['D0:Kspipi']


def loadD0_hh_loose(path):
    """
    Creates a 'D0:hh' list, with an invariant mass in the range :math:`1.5 < M < 2.2~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K+:GoodTrack pi-:GoodTrack',
      2. 'K-:GoodTrack pi+:GoodTrack',
      3. 'pi+:GoodTrack pi-:GoodTrack',
      4. 'K+:GoodTrack K-:GoodTrack'.

    @param path     modules are added to this path
    """

    Dcuts = '1.5 < M < 2.2'
    D0_hh_Channels = [
        'K+:GoodTrack pi-:GoodTrack',
        'K-:GoodTrack pi+:GoodTrack',
        'pi+:GoodTrack pi-:GoodTrack',
        'K+:GoodTrack K-:GoodTrack'
    ]
    D0_hh_List = []
    for chID, channel in enumerate(D0_hh_Channels):
        ma.reconstructDecay(decayString='D0:D0Bar' + str(chID) + ' -> ' + channel, cut=Dcuts, dmID=chID, path=path)
        D0_hh_List.append('D0:D0Bar' + str(chID))
        ma.copyLists(outputListName='D0:hh', inputListNames=D0_hh_List, path=path)
    return D0_hh_List


def loadD0_Kshh_loose(path):
    """
    Creates a 'D0:Kshh' list, with an invariant mass in the range :math:`1.5 < M < 2.2~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K_S0:merged pi+:GoodTrack pi-:GoodTrack',
      2. 'K_S0:merged K+:GoodTrack K-:GoodTrack'.

    @param path     modules are added to this path
    """

    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:merged pi+:GoodTrack pi-:GoodTrack',
        'K_S0:merged K+:GoodTrack K-:GoodTrack'
    ]
    D0List = []
    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay(decayString='D0:Kshh' + str(chID) + ' -> ' + channel, cut=Dcuts, dmID=chID, path=path)
        D0List.append('D0:Kshh' + str(chID))
        ma.copyLists(outputListName='D0:Kshh', inputListNames=D0List, path=path)
    return D0List


def loadD0_Kspi0_loose(path):
    """
    Creates a 'D0:Kspi0' list, with an invariant mass in the range :math:`1.5 < M < 2.2~{\\rm GeV}/c^2`,
    from the 'K_S0:merged' and the 'pi0:bth_skim' lists.

    @param path     modules are added to this path
    """

    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:merged pi0:bth_skim'
    ]

    D0List = []
    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay(decayString='D0:Kspi0' + str(chID) + ' -> ' + channel, cut=Dcuts, dmID=chID, path=path)
        D0List.append('D0:Kspi0' + str(chID))
        ma.copyLists(outputListName='D0:Kspi0', inputListNames=D0List, path=path)
    return D0List


def loadD0_Kspipipi0(path):
    """
    Creates a 'D0:Kspipipi0' list, with an invariant mass in the range :math:`1.5 < M < 2.2~{\\rm GeV}/c^2`,
    from the 'K_S0:merged', 'pi+:GoodTrack', 'pi-:GoodTrack' and 'pi0:bth_skim' lists.

    @param path     modules are added to this path
    """

    Dcuts = '1.8 < M < 1.9'

    D0_Channels = [
        'K_S0:merged pi+:GoodTrack pi-:GoodTrack pi0:bth_skim'
    ]
    D0List = []
    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay(decayString='D0:Kspipipi0' + str(chID) + ' -> ' + channel, cut=Dcuts, dmID=chID, path=path)
        D0List.append('D0:Kspipipi0' + str(chID))
        ma.copyLists(outputListName='D0:Kspipipi0', inputListNames=D0List, path=path)
    return D0List


def loadStdDplus(path):
    """
    Creates a 'D+:all' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack',
      2. 'K_S0:merged pi+:GoodTrack',
      3. 'K_S0:merged pi+:GoodTrack pi0:bth_skim',
      4. 'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi0:bth_skim'.

    @param path     modules are added to this path
    """
    DplusCuts = '1.8 < M < 1.9'
    Dplus_Channels = ['K-:GoodTrack pi+:GoodTrack pi+:GoodTrack',
                      'K_S0:merged pi+:GoodTrack',
                      'K_S0:merged pi+:GoodTrack pi0:bth_skim',
                      'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi0:bth_skim',
                      ]

    DplusList = []
    for chID, channel in enumerate(Dplus_Channels):
        ma.reconstructDecay(decayString='D+:all' + str(chID) + ' -> ' + channel, cut=DplusCuts, dmID=chID, path=path)
        DplusList.append('D+:all' + str(chID))
    ma.copyLists(outputListName='D+:all', inputListNames=DplusList, path=path)
    return DplusList


def loadStdDplus_Kpipi(path=None):
    """
    Creates a 'D+:Kpipi' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kpipi -> K-:GoodTrack pi+:GoodTrack pi+:GoodTrack', cut='1.8 < M < 1.9', dmID=1, path=path)
    return ['D+:Kpipi']


def loadStdDplus_Kspi(path=None):
    """
    Creates a 'D+:Kspi' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      2. 'K_S0:merged pi+:GoodTrack',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kspi -> K_S0:merged pi+:GoodTrack', cut='1.8 < M < 1.9', dmID=2, path=path)
    return ['D+:Kspi']


def loadStdDplus_Kspipi0(path=None):
    """
    Creates a 'D+:Kspipi0' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      3. 'K_S0:merged pi+:GoodTrack pi0:bth_skim',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kspipi0 -> K_S0:merged pi+:GoodTrack pi0:bth_skim', cut='1.8 < M < 1.9',
                        dmID=3, path=path)
    return ['D+:Kspipi0']


def loadStdDplus_Kpipipi0(path=None):
    """
    Creates a 'D+:Kpipipi0' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      4. 'K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi0:bth_skim'.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kpipipi0 -> K-:GoodTrack pi+:GoodTrack pi+:GoodTrack pi0:bth_skim', cut='1.8 < M < 1.9',
                        dmID=3, path=path)
    return ['D+:Kpipipi0']


def loadStdDstar0(path):
    """
    Create a list of 'D*0:all' list, with an invariant mass cut :math:`1.95 < M < 2.05 GeV`, from the following particles lists:

      - 'D0:all pi0:eff40_May2020'
      - 'D0:all gamma:loose'

    @param path     modules are added to this path
    """
    Dstar0Cuts = 'massDifference(0) < 0.16'
    D_Channels = ['D0:all pi0:eff40_May2020',
                  'D0:all gamma:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay(decayString='D*0:std' + str(chID) + ' -> ' + channel, cut=Dstar0Cuts, dmID=chID, path=path)
        DList.append('D*0:std' + str(chID))
    ma.copyLists(outputListName='D*0:all', inputListNames=DList, path=path)
    return DList


def loadStdDstar0_D0pi0_all(path=None):
    """
    Creates a 'D*0:all' list combining the 'D0:all' and 'pi0:bth_skim' lists by requiring the mass difference between D*0 and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    Dstar0Cuts = 'massDifference(0) < 0.16'
    Dstar0_Channels = ['D0:all pi0:bth_skim',
                       ]

    Dstar0List = []
    for chID, channel in enumerate(Dstar0_Channels):
        ma.reconstructDecay(decayString='D*0:all' + str(chID) + ' -> ' + channel, cut=Dstar0Cuts, dmID=chID, path=path)
        Dstar0List.append('D*0:all' + str(chID))
    ma.copyLists(outputListName='D*0:all', inputListNames=Dstar0List, path=path)
    return Dstar0List


def loadStdDstar0_D0pi0_Kpi(path=None):
    """
    Creates a 'D*0:D0_Kpi' list combining the 'D0:Kpi' and 'pi0:bth_skim' lists by requiring the mass difference between D*0 and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kpi -> D0:Kpi pi0:bth_skim', cut='massDifference(0) < 0.16', dmID=1, path=path)
    return ['D*0:D0_Kpi']


def loadStdDstar0_D0pi0_Kpipipi(path=None):
    """
    Creates a 'D*0:D0_Kpipipi' list combining the 'D0:Kpipipi' and 'pi0:bth_skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kpipipi -> D0:Kpipipi pi0:bth_skim', cut='massDifference(0) < 0.16', dmID=2, path=path)
    return ['D*0:D0_Kpipipi']


def loadStdDstar0_D0pi0_Kpipi0(path=None):
    """
    Creates a 'D*0:D0_Kpipi0' list combining the 'D0:Kpipi0' and 'pi0:bth_skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kpipi0 -> D0:Kpipi0 pi0:bth_skim', cut='massDifference(0) < 0.16', dmID=3, path=path)
    return ['D*0:D0_Kpipi0']


def loadStdDstar0_D0pi0_Kspipi(path=None):
    """
    Creates a 'D*0:D0_Kspipi' list combining the 'D0:Kspipi' and 'pi0:bth_skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kspipi -> D0:Kspipi pi0:bth_skim', cut='massDifference(0) < 0.16', dmID=4, path=path)
    return ['D*0:D0_Kspipi']


def loadStdDstarPlus(path):
    """
    Create a list of 'D*++:all' list, with an invariant mass cut :math:`1.95 < M < 2.05 GeV`, from the following particles lists:
      - 'D0:all pi+:GoodTrack'
      - 'D+:all pi0:eff40_May2020'

    @param path     modules are added to this path
    """
    DstarPlusCuts = 'massDifference(0) < 0.16'
    D_Channels = ['D0:all pi+:GoodTrack',
                  'D+:all pi0:eff40_May2020']

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay(decayString='D*+:std' + str(chID) + ' -> ' + channel, cut=DstarPlusCuts, dmID=chID, path=path)
        DList.append('D*+:std' + str(chID))
    ma.copyLists(outputListName='D*+:all', inputListNames=DList, path=path)
    return DList


def loadStdDstarPlus_D0pi_all(path=None):
    """
    Creates a 'D*+:all' list combining the 'D0:all' and 'pi+:GoodTrack' list by requiring the mass difference between D*+ and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    DstarPlusCuts = 'massDifference(0) < 0.16'
    DstarPlus_Channels = ['D0:all pi+:GoodTrack',
                          ]

    DstarPlusList = []
    for chID, channel in enumerate(DstarPlus_Channels):
        ma.reconstructDecay(decayString='D*+:std' + str(chID) + ' -> ' + channel, cut=DstarPlusCuts, dmID=chID, path=path)
        DstarPlusList.append('D*+:std' + str(chID))
    ma.copyLists(outputListName='D*+:all', inputListNames=DstarPlusList, path=path)
    return DstarPlusList


def loadStdDstarPlus_D0pi_Kpi(path=None):
    """
    Creates a 'D*+:D0_Kpi' list combining the 'D0:Kpi' and 'pi+:GoodTrack' lists by requiring the mass difference between D*0 and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpi -> D0:Kpi pi+:GoodTrack', cut='massDifference(0) < 0.16', dmID=1, path=path)
    return ['D*+:D0_Kpi']


def loadSkimHighEffDstarPlus_D0pi_Kpi(path=None):
    """
    Creates a 'D*+:D0_Kpi_skimhigheff' list combining the 'D0:Kpi_skimhigheff' and 'pi+:slowPi' lists by requiring the
    mass difference between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpi_skimhigheff -> D0:Kpi_skimhigheff pi+:slowPi',
                        cut='massDifference(0) < 0.16', dmID=1, path=path)
    return ['D*+:D0_Kpi_skimhigheff']


def loadStdDstarPlus_D0pi_Kpipipi(path=None):
    """
    Creates a 'D*+:D0_Kpipipi' list combining the 'D0:Kpipipi' and 'pi+:GoodTrack' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpipipi -> D0:Kpipipi pi+:GoodTrack', cut='massDifference(0) < 0.16', dmID=2, path=path)
    return ['D*+:D0_Kpipipi']


def loadSkimHighEffDstarPlus_D0pi_Kpipipi(path=None):
    """
    Creates a 'D*+:D0_Kpipipi_skimhigheff' list combining the 'D0:Kpipipi_skimhigheff' and 'pi+:slowPi' lists
    by requiring the mass difference between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpipipi_skimhigheff -> D0:Kpipipi_skimhigheff pi+:slowPi',
                        cut='massDifference(0) < 0.16', dmID=2, path=path)
    return ['D*+:D0_Kpipipi_skimhigheff']


def loadStdDstarPlus_D0pi_Kpipi0(path=None):
    """
    Creates a 'D*+:D0_Kpipi0' list combining the 'D0:Kpipi0' and 'pi+:GoodTrack' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpipi0 -> D0:Kpipi0 pi+:GoodTrack', cut='massDifference(0) < 0.16', dmID=3, path=path)
    return ['D*+:D0_Kpipi0']


def loadStdDstarPlus_D0pi_Kpipi0_eff20(path=None):
    """
    Creates a 'D*+:D0_Kpipi0_eff20' list combining the 'D0:Kpipi0_eff20' and 'pi+:slowPi' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpipi0_eff20 -> D0:Kpipi0_eff20 pi+:slowPi',
                        cut='massDifference(0) < 0.16', dmID=3, path=path)
    return ['D*+:D0_Kpipi0_eff20']


def loadStdDstarPlus_D0pi_Kspipi(path=None):
    """
    Creates a 'D*+:D0_Kspipi' list combining the 'D0:Kspipi' and 'pi+:GoodTrack' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kspipi -> D0:Kspipi pi+:GoodTrack', cut='massDifference(0) < 0.16', dmID=4, path=path)
    return ['D*+:D0_Kspipi']


def loadStdDstarPlus_Dpi0_Kpipi(path=None):
    """
    Creates a 'D*+:Dpi0_Kpipi' list combining the 'D+:Kpipi' and 'pi0:bth_skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:Dpi0_Kpipi -> D+:Kpipi pi0:bth_skim', cut='massDifference(0) < 0.16', dmID=4, path=path)
    return ['D*+:Dpi0_Kpipi']
