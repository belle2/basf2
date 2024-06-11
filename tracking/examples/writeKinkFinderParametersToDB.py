#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


parameters = Belle2.KinkFinderParameters()
# int kinkFitterMode, double vertexChi2Cut, double vertexDistanceCut
# double precutRho, double precutZ, double precutDistance, double precutDistance2D
# kinkFitterMode in binary (1 is ON, 0 is OFF), but written in decimal:
# 1 bit (from right) reassign hits, 2 bit flip tracks with close end points,
# 3 bit fit both tracks as one, and 4 bit split track, candidate to be a combined track
parameters.setParameters(0b1111, 10000., 2., 10., 0., 10., 10., 49, 0.01)
iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

# write db object to 'localdb/'
Belle2.Database.Instance().storeData('KinkFinderParameters', parameters, iov)
