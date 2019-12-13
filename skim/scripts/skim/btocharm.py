#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
List of functions to skim events containing :math:`B\\to X_c + h` decays,
where :math:`X_c` stays for :math:`D^0`, :math:`D^{\\pm}`, :math:`D^{*0}` and :math:`D^{*\\pm}`,
and :math:`h` stays for :math:`\\pi^{\\pm}`, :math:`K^{\\pm}`, :math:`\\rho^{\\pm}` and :math:`a_1^{\\pm}`.
"""

__authors__ = [
    "Fernando Abudinen",
    "Hulya Atmacan",
    "Chiara La Licata",
    "Minakshi Nayak",
    "Niharika Rout"
]

import modularAnalysis as ma


def BsigToD0hTohhList(path):
    """
    Skim list definitions for all charged  B to charm 2 body decays.

    **Skim Author**: H. Atmacan

    **Skim Name**: BtoD0h_hh

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140100

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^- \\pi^+) \\pi^+`,

    3.  :math:`B^{+}\\to \\overline{D}^{0} (\\to \\pi^+ \\pi^-) \\pi^+`,

    4.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ K^-) \\pi^+`,

    5.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) K^+`,

    6.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^- \\pi^+) K^+`,

    7.  :math:`B^{+}\\to \\overline{D}^{0} (\\to \\pi^+ \\pi^-) K^+`,

    8.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ K^-) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.5 < M_D0 < 2.2
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadD0_hh_loose()  from the ``charm.py`` script where D0 channels are defined.


    """

    __author__ = "H. Atmacan"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['anti-D0:hh pi+:all',
                    'anti-D0:hh K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0h_hh' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0h_hh' + str(chID))

    Lists = BsigList
    return Lists


def BsigToD0hToKshhList(path):
    """
    **Skim Author**: M. Nayak

    **Skim Name**: BtoD0h_Kshh

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140200

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ \\pi^-) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^- \\pi^+) \\pi^+`,

    3.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^-) \\pi^+`,

    4.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ K^-) \\pi^+`,

    5.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ \\pi^-) K^+`,

    6.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^- \\pi^+) K^+`,

    7.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^-) K^+`,

    8.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 K^+ K^-) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.5 < M_D0 < 2.2
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadD0_Kshh_loose()  from the ``charm.py`` script where D0 channels are defined.

    """
    __author__ = "M. Nayak"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['anti-D0:Kshh pi+:all',
                    'anti-D0:Kshh K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0h_Kshh' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0h_Kshh' + str(chID))

    Lists = BsigList
    return Lists


def BsigToD0hToKspi0List(path):
    """
    **Skim Author**: M. Nayak

    **Skim Name**: BtoD0h_Kspi0

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140300

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^0) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^0) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.5 < M_D0 < 2.2
    2. 5.2 < Mbc
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadD0_Kspi0_loose()  from the ``charm.py`` script where D0 channels are defined.

    """
    __author__ = "M. Nayak"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['anti-D0:Kspi0 pi+:all',
                    'anti-D0:Kspi0 K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0h_Kspi0' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0h_Kspi0' + str(chID))

    Lists = BsigList
    return Lists


def BsigToD0hToKspipipi0List(path):
    """
    **Skim Author**: N. Rout

    **Skim Name**: BtoD0h_Kspipipi0

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140400

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^- \\pi^0) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K_{\\rm S}^0 \\pi^+ \\pi^- \\pi^0) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.8 < M_D0 < 1.9
    2. 5.25 < Mbc
    3. abs(deltaE) < 0.2

    Note:
    This skim uses loadD0_Kspipipi0() from the ``charm.py`` script where D0 channels are defined.

    """
    __author__ = "N. Rout"

    Bcuts = 'Mbc > 5.25 and abs(deltaE) < 0.2'

    BsigChannels = ['anti-D0:Kspipipi0 pi+:all',
                    'anti-D0:Kspipipi0 K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0h_Kspipipi0' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0h_Kspipipi0' + str(chID))

    Lists = BsigList
    return Lists


def BsigToD0hToKpiList(path):
    """
    Skim list definitions for all charged  B to charm 3 body decays.

    **Skim Author**: Niharika Rout

    **Skim Name**: BtoD0h_Kpi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140101

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.7 < M_D0 < 2.0
    2. Mbc > 5.2
    3. abs(deltaE) < 0.5

    Note:
    This skim uses loadStdD0_Kpi() from the ``charm.py`` script
    where D0 channels are defined.


    """

    __author__ = "Niharika Rout"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BsigChannels = ['anti-D0:Kpi pi+:all',
                    'anti-D0:Kpi K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0h_Kpi' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0h_Kpi' + str(chID))

    Lists = BsigList
    return Lists


def BsigToD0hToKpipipiList(path):
    """
    **Skim Author**: N. Rout

    **Skim Name**: BtoD0h_Kpipipi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14140102

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:


    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^- pi^- pi^+) \\pi^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ pi^- pi^- pi^+) K^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    1. 1.7 < M_D0 < 2.0
    2. Mbc > 5.2
    3. abs(deltaE) < 0.5

    Note:
    This skim uses loadStdD0_Kpipipi() from the ``charm.py`` script
    where D0 channels are defined.

    """
    __author__ = "Niharika Rout"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'

    BsigChannels = ['anti-D0:Kpipipi pi+:all',
                    'anti-D0:Kpipipi K+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0h_Kpipipi' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0h_Kpipipi' + str(chID))

    Lists = BsigList
    return Lists


def loadB0toDpi_Kpipi(path):
    """
    **Skim Author**: C. La Licata

    **Skim Name**: B0toDpi_Kpipi

    **Skim Category**: physics, hadronic neutral B to charm

    **Skim Code**: 14120600

    **Working Group**: BtoCharm (WG4)

    **Decay Mode**:


    1.  :math:`B^{0}\\to \\D^{-} (\\to K^+ pi^- pi^-) \\pi^+`

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    2. Mbc > 5.2
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadStdDplus_Kpipi from the ``charm.py`` script
    where D- channel is defined.

    """

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:Dpi_Kpipi -> D-:Kpipi pi+:all', Bcuts, 0, path=path)

    return ['B0:Dpi_Kpipi']


def loadB0toDstarh_Kpi(path):
    """
    **Skim Author**: C. La Licata

    **Skim Name**: B0toDStarPi_D0pi_Kpi

    **Skim Category**: physics, hadronic neutral B to charm

    **Skim Code**: 14120700

    **Working Group**: BtoCharm (WG4)

    **Decay Mode**:


    1.  :math:`B^{0}\\to \\D^{*-} (\\to K^+ pi^-) \\pi^+`

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    2. Mbc > 5.2
    3. abs(deltaE) < 0.3

    Note:
    This skim uses loadStdDstarPlus_D0pi_Kpi from the ``charm.py`` script
    where D*- channel is defined.

    """

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:Dstarpi_Kpi -> D*-:D0_Kpi pi+:all', Bcuts, 0, path=path)

    return ['B0:Dstarpi_Kpi']


def loadB0toDstarh_Kpipipi(path):
    """
   **Skim Author**: C. La Licata

   **Skim Name**: B0toDStarPi_D0pi_Kpipipi

   **Skim Category**: physics, hadronic neutral B to charm

   **Skim Code**: 14120700

   **Working Group**: BtoCharm (WG4)

   **Decay Mode**:


   1.  :math:`B^{0}\\to \\D^{*-} (\\to K^+ pi^- pi^- pi^+) \\pi^+`

   **Particle Lists**: Standard lists for all particles.

   **Additional Cuts**:
   2. Mbc > 5.2
   3. abs(deltaE) < 0.3

   Note:
   This skim uses  from the ``charm.py`` script
   where D*- channel is defined.

   """

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:Dstarpi_Kpipipi -> D*-:D0_Kpipipi pi+:all', Bcuts, 0, path=path)

    return ['B0:Dstarpi_Kpipipi']


def DoubleCharmList(path):
    """
    **Skim Author**: Phillip Urquijo

    **Skim Name**: DoubleCharm

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14130200

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{0}\\to \\overline{D}^{0} D^{0}`,

    2.  :math:`B^{0}\\to \\overline{D*}^{0} D^{0}`,

    3.  :math:`B^{0}\\to \\overline{D*}^{0} D^{*0}`,

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:
    2. 5.2 < Mbc < 5.29
    3. abs(deltaE) < 0.5

    """
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_Channels = ['D0:all D0:all',
                   'D*0:all D0:all',
                   'D*0:all D*0:all']

    B0List = []
    for chID, channel in enumerate(B0_Channels):
        ma.reconstructDecay('B0:DC' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        B0List.append('B0:DC' + str(chID))

    Lists = B0List
    return Lists
