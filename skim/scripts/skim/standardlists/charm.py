#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# /***************************************************************************
#  *   BASF2 (Belle Analysis Framework 2)                                    *
#  *                                                                         *
#  *   This script contains the definitions of the standard particle lists   *
#  *   used in the reconstruction of control channels and analyses for       *
#  *   data validation.                                                      *
#  *                                                                         *
#  *   Contributors: Fernando Abudinen, Chiara La Licata, Niharika Rout.     *
#  *                                                                         *
#  ***************************************************************************/

import basf2 as b2
import modularAnalysis as ma
import vertex as vx


def loadStdD0(path):
    """
    Creates a 'D0:all' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K-:all pi+:all',
      2. 'K-:all pi+:all pi+:all pi-:all',
      3. 'K-:all pi+:all pi0:skim',
      4. 'K_S0:merged pi+:all pi-:all'.

    @param path     modules are added to this path
    """
    D0Cuts = '1.7 < M < 2.0'
    D0_Channels = ['K-:all pi+:all',
                   'K-:all pi+:all pi+:all pi-:all',
                   'K-:all pi+:all pi0:skim',
                   'K_S0:merged pi+:all pi-:all',
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

      1. 'K-:all pi+:all',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpi -> K-:all pi+:all', cut='1.7 < M < 2.0', dmID=1, path=path)
    return ['D0:Kpi']


def loadStdD0_Kpipipi(path=None):
    """
    Creates a 'D0:Kpipipi' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      2. 'K-:all pi+:all pi+:all pi-:all',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpipipi -> K-:all pi+:all pi+:all pi-:all',
                        cut='1.7 < M < 2.0', dmID=2, path=path)
    return ['D0:Kpipipi']


def loadStdD0_Kpipi0(path=None):
    """
    Creates a 'D0:Kpipi0' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      3. 'K-:all pi+:all pi0:skim',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kpipi0 -> K-:all pi+:all pi0:skim', cut='1.7 < M < 2.0', dmID=3, path=path)
    return ['D0:Kpipi0']


def loadStdD0_Kspipi(path=None):
    """
    Creates a 'D0:Kspipi' list, with an invariant mass in the range :math:`1.7 < M < 2.0~{\\rm GeV}/c^2`,
    from the following particles lists:

      4. 'K_S0:merged pi+:all pi-:all'.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D0:Kspipi -> K_S0:merged pi+:all pi-:all', cut='1.7 < M < 2.0', dmID=4, path=path)
    return ['D0:Kspipi']


def loadD0_hh_loose(path):
    """
    Creates a 'D0:hh' list, with an invariant mass in the range :math:`1.5 < M < 2.2~{\\rm GeV}/c^2`,
    from the following particles lists:

      1. 'K+:all pi-:all',
      2. 'K-:all pi+:all',
      3. 'pi+:all pi-:all',
      4. 'K+:all K-:all'.

    @param path     modules are added to this path
    """

    Dcuts = '1.5 < M < 2.2'
    D0_hh_Channels = [
        'K+:all pi-:all',
        'K-:all pi+:all',
        'pi+:all pi-:all',
        'K+:all K-:all'
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

      1. 'K_S0:merged pi+:all pi-:all',
      2. 'K_S0:merged K+:all K-:all'.

    @param path     modules are added to this path
    """

    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:merged pi+:all pi-:all',
        'K_S0:merged K+:all K-:all'
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
    from the 'K_S0:merged' and the 'pi0:skim' lists.

    @param path     modules are added to this path
    """

    Dcuts = '1.5 < M < 2.2'

    D0_Channels = [
        'K_S0:merged pi0:skim'
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
    from the 'K_S0:merged', 'pi+:all', 'pi-:all' and 'pi0:skim' lists.

    @param path     modules are added to this path
    """

    Dcuts = '1.8 < M < 1.9'

    D0_Channels = [
        'K_S0:merged pi+:all pi-:all pi0:skim'
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

      1. 'K-:all pi+:all pi+:all',
      2. 'K_S0:merged pi+:all',
      3. 'K_S0:merged pi+:all pi0:skim',
      4. 'K-:all pi+:all pi+:all pi0:skim'.

    @param path     modules are added to this path
    """
    DplusCuts = '1.8 < M < 1.9'
    Dplus_Channels = ['K-:all pi+:all pi+:all',
                      'K_S0:merged pi+:all',
                      'K_S0:merged pi+:all pi0:skim',
                      'K-:all pi+:all pi+:all pi0:skim',
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

      1. 'K-:all pi+:all pi+:all',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kpipi -> K-:all pi+:all pi+:all', cut='1.8 < M < 1.9', dmID=1, path=path)
    return ['D+:Kpipi']


def loadStdDplus_Kspi(path=None):
    """
    Creates a 'D+:Kspi' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      2. 'K_S0:merged pi+:all',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kspi -> K_S0:merged pi+:all', cut='1.8 < M < 1.9', dmID=2, path=path)
    return ['D+:Kspi']


def loadStdDplus_Kspipi0(path=None):
    """
    Creates a 'D+:Kspipi0' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      3. 'K_S0:merged pi+:all pi0:skim',

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kspipi0 -> K_S0:merged pi+:all pi0:skim', cut='1.8 < M < 1.9',
                        dmID=3, path=path)
    return ['D+:Kspipi0']


def loadStdDplus_Kpipipi0(path=None):
    """
    Creates a 'D+:Kpipipi0' list, with an invariant mass in the range :math:`1.8 < M < 1.9~{\\rm GeV}/c^2`,
    from the following particles lists:

      4. 'K-:all pi+:all pi+:all pi0:skim'.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D+:Kpipipi0 -> K-:all pi+:all pi+:all pi0:skim', cut='1.8 < M < 1.9',
                        dmID=3, path=path)
    return ['D+:Kpipipi0']


def loadStdDstar0(path):
    """
    Create a list of 'D*0:all' list, with an invariant mass cut :math:`1.95 < M < 2.05 GeV`, from the following particles lists:

      - 'D0:all pi0:loose'
      - 'D0:all gamma:loose'

    @param path     modules are added to this path
    """
    Dstar0Cuts = 'massDifference(0) < 0.16'
    D_Channels = ['D0:all pi0:loose',
                  'D0:all gamma:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay(decayString='D*0:std' + str(chID) + ' -> ' + channel, cut=Dstar0Cuts, dmID=chID, path=path)
        DList.append('D*0:std' + str(chID))
    ma.copyLists(outputListName='D*0:all', inputListNames=DList, path=path)
    return DList


def loadStdDstar0_D0pi0_all(path=None):
    """
    Creates a 'D*0:all' list combining the 'D0:all' and 'pi0:skim' lists by requiring the mass difference between D*0 and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    Dstar0Cuts = 'massDifference(0) < 0.16'
    Dstar0_Channels = ['D0:all pi0:skim',
                       ]

    Dstar0List = []
    for chID, channel in enumerate(Dstar0_Channels):
        ma.reconstructDecay(decayString='D*0:all' + str(chID) + ' -> ' + channel, cut=Dstar0Cuts, dmID=chID, path=path)
        Dstar0List.append('D*0:all' + str(chID))
    ma.copyLists(outputListName='D*0:all', inputListNames=Dstar0List, path=path)
    return Dstar0List


def loadStdDstar0_D0pi0_Kpi(path=None):
    """
    Creates a 'D*0:D0_Kpi' list combining the 'D0:Kpi' and 'pi0:skim' lists by requiring the mass difference between D*0 and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kpi -> D0:Kpi pi0:skim', cut='massDifference(0) < 0.16', dmID=1, path=path)
    return ['D*0:D0_Kpi']


def loadStdDstar0_D0pi0_Kpipipi(path=None):
    """
    Creates a 'D*0:D0_Kpipipi' list combining the 'D0:Kpipipi' and 'pi0:skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kpipipi -> D0:Kpipipi pi0:skim', cut='massDifference(0) < 0.16', dmID=2, path=path)
    return ['D*0:D0_Kpipipi']


def loadStdDstar0_D0pi0_Kpipi0(path=None):
    """
    Creates a 'D*0:D0_Kpipi0' list combining the 'D0:Kpipi0' and 'pi0:skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kpipi0 -> D0:Kpipi0 pi0:skim', cut='massDifference(0) < 0.16', dmID=3, path=path)
    return ['D*0:D0_Kpipi0']


def loadStdDstar0_D0pi0_Kspipi(path=None):
    """
    Creates a 'D*0:D0_Kspipi' list combining the 'D0:Kspipi' and 'pi0:skim' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*0:D0_Kspipi -> D0:Kspipi pi0:skim', cut='massDifference(0) < 0.16', dmID=4, path=path)
    return ['D*0:D0_Kspipi']


def loadStdDstarPlus(path):
    """
    Create a list of 'D*++:all' list, with an invariant mass cut :math:`1.95 < M < 2.05 GeV`, from the following particles lists:
      - 'D0:all pi+:all'
      - 'D+:all pi0:loose'

    @param path     modules are added to this path
    """
    DstarPlusCuts = 'massDifference(0) < 0.16'
    D_Channels = ['D0:all pi+:all',
                  'D+:all pi0:loose']

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay(decayString='D*+:std' + str(chID) + ' -> ' + channel, cut=DstarPlusCuts, dmID=chID, path=path)
        DList.append('D*+:std' + str(chID))
    ma.copyLists(outputListName='D*+:all', inputListNames=DList, path=path)
    return DList


def loadStdDstarPlus_D0pi_all(path=None):
    """
    Creates a 'D*+:all' list combining the 'D0:all' and 'pi+:all' list by requiring the mass difference between D*+ and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    DstarPlusCuts = 'massDifference(0) < 0.16'
    DstarPlus_Channels = ['D0:all pi+:all',
                          ]

    DstarPlusList = []
    for chID, channel in enumerate(DstarPlus_Channels):
        ma.reconstructDecay(decayString='D*+:std' + str(chID) + ' -> ' + channel, cut=DstarPlusCuts, dmID=chID, path=path)
        DstarPlusList.append('D*+:std' + str(chID))
    ma.copyLists(outputListName='D*+:all', inputListNames=DstarPlusList, path=path)
    return DstarPlusList


def loadStdDstarPlus_D0pi_Kpi(path=None):
    """
    Creates a 'D*+:D0_Kpi' list combining the 'D0:Kpi' and 'pi+:all' lists by requiring the mass difference between D*0 and D0
    to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpi -> D0:Kpi pi+:all', cut='massDifference(0) < 0.16', dmID=1, path=path)
    return ['D*+:D0_Kpi']


def loadStdDstarPlus_D0pi_Kpipipi(path=None):
    """
    Creates a 'D*+:D0_Kpipipi' list combining the 'D0:Kpipipi' and 'pi+:all' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpipipi -> D0:Kpipipi pi+:all', cut='massDifference(0) < 0.16', dmID=2, path=path)
    return ['D*+:D0_Kpipipi']


def loadStdDstarPlus_D0pi_Kpipi0(path=None):
    """
    Creates a 'D*+:D0_Kpipi0' list combining the 'D0:Kpipi0' and 'pi+:all' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kpipi0 -> D0:Kpipi0 pi+:all', cut='massDifference(0) < 0.16', dmID=3, path=path)
    return ['D*+:D0_Kpipi0']


def loadStdDstarPlus_D0pi_Kspipi(path=None):
    """
    Creates a 'D*+:D0_Kspipi' list combining the 'D0:Kspipi' and 'pi+:all' lists by requiring the mass difference
    between D*0 and D0 to be in the range :math:`\\Delta M < 0.16~{\\rm GeV}/c^2`.

    @param path     modules are added to this path
    """
    ma.reconstructDecay(decayString='D*+:D0_Kspipi -> D0:Kspipi pi+:all', cut='massDifference(0) < 0.16', dmID=4, path=path)
    return ['D*+:D0_Kspipi']
