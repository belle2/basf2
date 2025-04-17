#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


def writeKinkFinderParametersToDB():
    """
    run this script to create db file storing the payload information of the KinkFinderParameters
    see `tracking/dbobjects/include/KinkFinderParameters.h` for definition of the parameters
    """

    parameters = Belle2.KinkFinderParameters()

    # kinkFitter working mode (unsigned char) from 0 to 15  in binary:
    # 1st bit: reassign hits (1 is On, 0 is Off)
    # 2nd bit: flip tracks with close end points (1 is On, 0 is Off)
    # 3rd bit: fit both tracks as one (1 is On, 0 is Off)
    # 4th bit: find candidate tracks to be split and do it (1 is On, 0 is Off)
    kinkFitterMode = 0b1111
    # Cut on Chi2 for the Kink vertex (double)
    vertexChi2Cut = 10000.
    # Cut on distance between tracks at the Kink vertex [cm] (double)
    vertexDistanceCut = 2.
    # Preselection cut on transverse shift from the outer CDC wall for the track ending points [cm] (double)
    precutRho = 10.
    # Preselection cut on z shift from the outer CDC wall for the track ending points [cm] (double)
    precutZ = 0.
    # Preselection cut on distance between ending points of two tracks [cm] (double)
    precutDistance = 10.
    # Preselection cut on 2D distance between ending points of two tracks (for bad z cases) [cm] (double)
    precutDistance2D = 10.
    # Preselection cut on maximal number of fitted CDC hits for a track candidate to be split (int)
    precutSplitNCDCHit = 49
    # Preselection cut on maximal p-value for a track candidate to be split
    precutSplitPValue = 0.01

    parameters.setParameters(kinkFitterMode, vertexChi2Cut, vertexDistanceCut,
                             precutRho, precutZ, precutDistance, precutDistance2D,
                             precutSplitNCDCHit, precutSplitPValue)
    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    # write db object to 'localdb/'
    Belle2.Database.Instance().storeData('KinkFinderParameters', parameters, iov)


if __name__ == '__main__':
    writeKinkFinderParametersToDB()
