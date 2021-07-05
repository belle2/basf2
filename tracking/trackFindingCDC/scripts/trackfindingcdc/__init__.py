##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Loading some common libraries
from ROOT import std  # noqa
from ROOT import Belle2  # make Belle2 namespace available # noqa
import basf2  # noqa
from ROOT import gSystem
gSystem.Load('libcdc')
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')
gSystem.Load('libgenfit2')
