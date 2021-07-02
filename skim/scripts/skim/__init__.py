#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Skim Package
------------

Core classes of skim package are provided as convenient imports directly from ``skim``.

To use a specific skim, import the skim from the appropriate working group submodule in
``skim.WGs``.

    from skim.WGs.systematics import RadEEList
"""

# Provide core classes of skim package
from skim.core import BaseSkim, CombinedSkim  # noqa
from skim.utils.misc import fancy_skim_header  # noqa
