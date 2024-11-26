##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
This module contains helper functions and classes for generating multiple
channels in a single job. It is designed for run dependent MC production.
'''
from .constants import EventCode, EventCodes_llXX, EventCodes_hhISR  # noqa
from .core import add_generators  # noqa
