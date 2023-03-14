#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


parameters = Belle2.TrackFlippingCuts()
parameters.setCuts(0.01, 0.8)

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

# write db object to 'localdb/'
Belle2.Database.Instance().storeData('TRKTrackFlipAndRefit_MVA_cuts', parameters, iov)
