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

    1. :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^+`,

    2. :math:`B^{+}\\to \\overline{D}^{0} (\\to K^- \\pi^+) \\pi^+`,

    3. :math:`B^{+}\\to \\overline{D}^{0} (\\to \\pi^+ \\pi^-) \\pi^+`,

    4. :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ K^-) \\pi^+`,

    5. :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) K^+`,

    6. :math:`B^{+}\\to \\overline{D}^{0} (\\to K^- \\pi^+) K^+`,

    7. :math:`B^{+}\\to \\overline{D}^{0} (\\to \\pi^+ \\pi^-) K^+`,

    8. :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ K^-) K^+`,



    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.5 < M_D0 < 2.2``

    2. ``5.2 < Mbc``

    3. ``abs(deltaE) < 0.3``

    Note:
      This skim uses ``loadD0_hh_loose()``  from the ``charm.py`` script where :math:`D^0` channels are defined.
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

        1. ``1.5 < M_D0 < 2.2``

        2. ``5.2 < Mbc``

        3. ``abs(deltaE) < 0.3``

    Note:
      This skim uses ``loadD0_Kshh_loose()``  from the ``charm.py`` script where D0 channels are defined.

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

    1. ``1.5 < M_D0 < 2.2``

    2. ``5.2 < Mbc``

    3. ``abs(deltaE) < 0.3``


    Note:
      This skim uses ``loadD0_Kspi0_loose()``  from the ``charm.py`` script where :math:`D^0` channels are defined.

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

    1. ``1.8 < M_D0 < 1.9``

    2. ``5.25 < Mbc``

    3. ``abs(deltaE) < 0.2``


    Note:
      This skim uses ``loadD0_Kspipipi0()`` from the ``charm.py`` script where :math:`D^0` channels are defined.

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

    1. ``1.7 < M_D0 < 2.0``

    2. ``Mbc > 5.2``

    3. ``abs(deltaE) < 0.5``


    Note:
      This skim uses ``loadStdD0_Kpi()`` from the ``charm.py`` script where :math:`D^0` channels are defined.

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

    1. ``1.7 < M_D0 < 2.0``

    2. ``Mbc > 5.2``

    3. ``abs(deltaE) < 0.5``


    Note:
      This skim uses ``loadStdD0_Kpipipi()`` from the ``charm.py`` script where :math:`D^0` channels are defined.

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


    1.  :math:`B^{0}\\to D^{-} (D^{0} \\to K^+ \\pi^- \\pi^-) \\pi^+`

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``Mbc > 5.2``

    2. ``abs(deltaE) < 0.3``

    Note:
      This skim uses ``loadStdDplus_Kpipi()`` from the ``charm.py`` script where :math:`D^-` channel is defined.

    """

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:Dpi_Kpipi -> D-:Kpipi pi+:all', Bcuts, 0, path=path)

    return ['B0:Dpi_Kpipi']


def loadB0toDstarPi_Kpi(path):
    """
    **Skim Author**: C. La Licata

    **Skim Name**: B0toDstarPi_D0pi_Kpi

    **Skim Category**: physics, hadronic neutral B to charm

    **Skim Code**: 14120700

    **Working Group**: BtoCharm (WG4)

    **Decay Mode**:

    1.  :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^-) \\pi^+`

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``Mbc > 5.2``

    2. ``abs(deltaE) < 0.3``


    Note:
        This skim uses ``loadStdDstarPlus_D0pi_Kpi()`` from the ``charm.py`` script where the :math:`D^{*-}` channel is defined.

    """

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:Dstarpi_Kpi -> D*-:D0_Kpi pi+:all', Bcuts, 0, path=path)

    return ['B0:Dstarpi_Kpi']


def loadB0toDstarPi_Kpipipi(path):
    """
    **Skim Author**: C. La Licata

    **Skim Name**: B0toDstarPi_D0pi_Kpipipi

    **Skim Category**: physics, hadronic neutral B to charm

    **Skim Code**: 14120700

    **Working Group**: BtoCharm (WG4)

    **Decay Mode**:


    1.  :math:`B^{0}\\to (D^{*-} \\to K^+ \\pi^- \\pi^- \\pi^+) \\pi^+`

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``Mbc > 5.2``

    2. ``abs(deltaE) < 0.3``

    """

    Bcuts = '5.2 < Mbc and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:Dstarpi_Kpipipi -> D*-:D0_Kpipipi pi+:all', Bcuts, 0, path=path)

    return ['B0:Dstarpi_Kpipipi']


def BtoD0rho_KpiList(path):
    """

    **Skim Author**: Fernando Abudinen

    **Skim Name**: BtoD0rho_Kpi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14141000

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\rho^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{*0} (\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^0) \\rho^+`,

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.7 < M_D0 < 2.0``

    2. ``0.47 < M_rho < 1.07``

    3. ``DM_Dstar_D < 0.16``

    4. ``Mbc > 5.2``

    5. ``abs(deltaE) < 0.3``

    Note:
    This skim uses loadStdD0_Kpi() and loadStdDstar0_D0pi0_Kpi() from the ``charm.py`` script,
    where D0 channels are defined, and loadStdAllRhoPlus() from the ``lightmesons.py `` script.

    """

    __author__ = "Fernando Abudinen"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = ['anti-D0:Kpi rho+:all',
                    'anti-D*0:D0_Kpi rho+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0rho_Kpi' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0rho_Kpi' + str(chID))

    Lists = BsigList
    return Lists


def BtoD0rho_Kpipipi_Kpipi0List(path):
    """

    **Skim Author**: Fernando Abudinen

    **Skim Name**: BtoD0rho_Kpipipi_Kpipi0

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14141001

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{+}\\to \\overline{D}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0) \\rho^+`,

    2.  :math:`B^{+}\\to \\overline{D}^{*0} (\\to \\overline{D}^{0} (\\to K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)
                         \\pi^0) \\rho^+`,


    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.7 < M_D0 < 2.0``

    2. ``DM_Dstar_D < 0.16``

    3. ``0.6 < M_rho < 0.9``

    4. ``cosHel_rho < 0.90``

    5. ``Mbc > 5.2``

    6. ``abs(deltaE) < 0.3``

    Note:
    This skim uses loadStdD0 and loadStdDstar0_D0pi0 functions from the ``charm.py`` script,
    where D0 channels are defined, and loadStdAllRhoPlus() from the ``lightmesons.py `` script.

   .. Warning::
       This skim saves only three randomly-chosen :math:`B^{+}` candidates in a 'B+:BtoD0rho_merged' list,
       since the candidate multiplicity of this skim is very high.

    """

    __author__ = "Fernando Abudinen"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3 and cosHelicityAngle(1,0) < 0.9 and 0.6 <= daughter(1,M) <= 0.9'

    BsigChannels = [
                    'anti-D0:Kpipipi rho+:all',
                    'anti-D0:Kpipi0 rho+:all',
                    'anti-D*0:D0_Kpipipi rho+:all',
                    'anti-D*0:D0_Kpipi0 rho+:all'
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0rho' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0rho' + str(chID))

    ma.copyLists(outputListName='B+:BtoD0rho_merged', inputListNames=BsigList, path=path)

    # Select only three random candidates to save them as these channels have high multiplicity.
    ma.rankByHighest(particleList='B+:BtoD0rho_merged', variable='cos(mdstIndex)', numBest=3,
                     outputVariable='cosMdstIndex_rank', path=path)

    return ['B+:BtoD0rho_merged']


def B0toDrho_KpipiList(path):
    """

    **Skim Author**: Fernando Abudinen

    **Skim Name**: B0toDrho_Kpipi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14121100

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{0}\\to D^{-} (\\to K^+ \\pi^- \\pi^-) \\rho^+`,

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.8 < M_D < 1.9``

    2. ``0.47 < M_rho < 1.07``

    4. ``Mbc > 5.2``

    5. ``abs(deltaE) < 0.3``

    Note:
    This skim uses loadStdDplus_Kpipi() from the ``charm.py`` script,
    where D0 channels are defined, and loadStdAllRhoPlus() from the ``lightmesons.py `` script.

    """

    __author__ = "Fernando Abudinen"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:B0toDrho_Kpipi -> D-:Kpipi rho+:all', Bcuts, 1, path=path)

    return ['B0:B0toDrho_Kpipi']


def B0toDrho_KspiList(path):
    """

    **Skim Author**: Fernando Abudinen

    **Skim Name**: B0toDrho_Kspi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14121101

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{0}\\to D^{-} (\\to K_{\\rm S}^0 \\pi^-) \\rho^+`,

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.8 < M_D < 1.9``

    2. ``0.47 < M_rho < 1.07``

    4. ``Mbc > 5.2``

    5. ``abs(deltaE) < 0.3``

    Note:
    This skim uses stdKshorts() from ```stdV0s.py`` and loadStdDplus_Kspi() from the ``charm.py`` script,
    where D0 channels are defined, and loadStdAllRhoPlus() from the ``lightmesons.py`` script.

    """

    __author__ = "Fernando Abudinen"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:B0toDrho_Kspi -> D-:Kspi rho+:all', Bcuts, 1, path=path)

    return ['B0:B0toDrho_Kspi']


def B0toDstarRho_KpiList(path):
    """

    **Skim Author**: Fernando Abudinen

    **Skim Name**: B0toDstarRho_D0pi_Kpi

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14121200

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{0}\\to D^{*-} (\\to \\overline{D}^{0} (\\to K^+ \\pi^-) \\pi^-) \\rho^+`,

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.7 < M_D < 2.0``

    2. ``0.47 < M_rho < 1.07``

    3. ``DM_Dstar_D < 0.16``

    4. ``Mbc > 5.2``

    5. ``abs(deltaE) < 0.3``

    Note:
    This skim uses loadStdD0_Kpi() and loadStdDstarPlus_D0pi_Kpi() from the ``charm.py`` script,
    where D0 channels are defined, and loadStdAllRhoPlus() from the ``lightmesons.py `` script.

    """

    __author__ = "Fernando Abudinen"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    ma.reconstructDecay('B0:BtoDstarRho_D0pi_Kpi -> D*-:D0_Kpi rho+:all', Bcuts, 1, path=path)

    return ['B0:BtoDstarRho_D0pi_Kpi']


def B0toDstarRho_Kpipipi_Kpipi0List(path):
    """

    **Skim Author**: Fernando Abudinen

    **Skim Name**: B0toDstarRho_Kpipipi_Kpipi0

    **Skim Category**: physics, hadronic B to charm

    **Skim Code**: 14121201

    **Working Group**: BtoCharm (WG4)

    **Decay Modes**:

    1.  :math:`B^{0}\\to D^{*-} (\\to \\overline{D}^{0} (\\to
                         K^+2\\pi^-\\pi^+, K^+\\pi^-\\pi^0)
                         \\pi^-) \\rho^+`,

    **Particle Lists**: Standard lists for all particles.

    **Additional Cuts**:

    1. ``1.7 < M_D < 2.0``

    2. ``0.47 < M_rho < 1.07``

    3. ``DM_Dstar_D < 0.16``

    4. ``Mbc > 5.2``

    5. ``abs(deltaE) < 0.3``

    Note:
    This skim uses loadStdD0 and loadStdDstarPlus_D0pi functions from the ``charm.py`` script,
    where D0 channels are defined, and loadStdAllRhoPlus() from the ``lightmesons.py `` script.

    """

    __author__ = "Fernando Abudinen"

    Bcuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

    BsigChannels = [
                    'D*-:D0_Kpipipi rho+:all',
                    'D*-:D0_Kpipi0 rho+:all',
                    ]
    BsigList = []
    for chID, channel in enumerate(BsigChannels):
        ma.reconstructDecay('B+:BtoD0rho' + str(chID) + ' -> ' + channel, Bcuts, chID, path=path)
        BsigList.append('B+:BtoD0rho' + str(chID))

    Lists = BsigList
    return Lists


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

    1. ``5.2 < Mbc < 5.29``

    2. ``abs(deltaE) < 0.5``

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
