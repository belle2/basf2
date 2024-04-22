#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Skim list for flagged-skims. They are empty skims since
they are not actually used in the code but they are needed to pass the
test_skim_framework.py.
"""
from skim import BaseSkim, fancy_skim_header

__authors__ = [
    "Valerio Bertacchi"
]


@fancy_skim_header
class f_SLME_p1(BaseSkim):
    """
    Skim for flagged SLME, part 1
    """


@fancy_skim_header
class f_SLME_p2(BaseSkim):
    """
    Skim for flagged SLME, part 2
    """


@fancy_skim_header
class f_EWP_p1(BaseSkim):
    """
    Skim for flagged EWP, part 1
    """


@fancy_skim_header
class f_EWP_p2(BaseSkim):
    """
    Skim for flagged EWP, part 2
    """


@fancy_skim_header
class f_TDCPV_p1(BaseSkim):
    """
    Skim for flagged TDCPV, part 1
    """


@fancy_skim_header
class f_TDCPV_p2(BaseSkim):
    """
    Skim for flagged TDCPV, part 2
    """


@fancy_skim_header
class f_bhadronic_p1(BaseSkim):
    """
    Skim for flagged bhadronic, part 1
    """


@fancy_skim_header
class f_bhadronic_p2(BaseSkim):
    """
    Skim for flagged bhadronic, part 2
    """


@fancy_skim_header
class f_quarkonium_p1(BaseSkim):
    """
    Skim for flagged quarkonium, part 1
    """


@fancy_skim_header
class f_quarkonium_p2(BaseSkim):
    """
    Skim for flagged quarkonium, part 2
    """


@fancy_skim_header
class f_charm_p1(BaseSkim):
    """
    Skim for flagged charm, part 1
    """


@fancy_skim_header
class f_charm_p2(BaseSkim):
    """
    Skim for flagged charm, part 2
    """


@fancy_skim_header
class f_dark_p1(BaseSkim):
    """
    Skim for flagged dark, part 1
    """


@fancy_skim_header
class f_dark_p2(BaseSkim):
    """
    Skim for flagged dark, part 2
    """


@fancy_skim_header
class f_tau_p1(BaseSkim):
    """
    Skim for flagged dark, part 1
    """


@fancy_skim_header
class f_tau_p2(BaseSkim):
    """
    Skim for flagged dark, part 2
    """
