##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2


def matchTriggerClusters(minClusterEnergy=0.1, maxAngle=0.1, minFracEnergy=0.0, maxFracEnergy=2.0, path=None):
    """
    Matching of ECLClusters and ECLTriggerClusters based on energy and angular separation
    @param minClusterEnergy minimum cluster energy
    @param maxAngle maximal angular separation
    @param minFracEnergy minimum energy fraction trg/cluster
    @param maxFracEnergy maximal energy fraction trg/cluster

    """

    matchtrgclust = b2.register_module('ECLTriggerClusterMatcher')
    matchtrgclust.param('minClusterEnergy', minClusterEnergy)
    matchtrgclust.param('maxAngle', maxAngle)
    matchtrgclust.param('minFracEnergy', minFracEnergy)
    matchtrgclust.param('maxFracEnergy', maxFracEnergy)
    matchtrgclust.param('minClusterEnergy', minClusterEnergy)

    path.add_module(matchtrgclust)


def matchTrackECLCalDigits(extRadius=130.0, angleFWDGap=31.80, angleBWDGap=129.7, trackHypothesis=11, path=None):
    """
    Matching of tracks to ECLCalDigits that are close to gaps
    @param extRadius Radius to which track is extrapolated
    @param angleFWDGap center of FWD gap (deg)
    @param angleBWDGap center of BWD gap (deg)
    @param trackHypothesis track hypothesis
    """

    matchtrkdig = b2.register_module('ECLTrackCalDigitMatch')
    matchtrkdig.param('extRadius', extRadius)
    matchtrkdig.param('angleFWDGap', angleFWDGap)
    matchtrkdig.param('angleBWDGap', angleBWDGap)
    matchtrkdig.param('trackHypothesis', trackHypothesis)

    path.add_module(matchtrkdig)
