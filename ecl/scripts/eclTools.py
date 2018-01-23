from basf2 import *
import os
import sys
from analysisPath import *


def matchTriggerClusters(minClusterEnergy=0.1, maxAngle=0.1, minFracEnergy=0.0, maxFracEnergy=2.0, path=analysis_main):
    """
    Matching of ECLClusters and ECLTriggerClusters based on energy and angular separation
    @param minClusterEnergy minimum cluster energy
    @param maxAngle maximal angular separation
    @param minFracEnergy minimum energy fraction trg/cluster
    @param maxFracEnergy maximal energy fraction trg/cluster

    """

    matchtrgclust = register_module('ECLTriggerClusterMatcher')
    matchtrgclust.param('minClusterEnergy', minClusterEnergy)
    matchtrgclust.param('maxAngle', maxAngle)
    matchtrgclust.param('minFracEnergy', minFracEnergy)
    matchtrgclust.param('maxFracEnergy', maxFracEnergy)
    matchtrgclust.param('minClusterEnergy', minClusterEnergy)

    path.add_module(matchtrgclust)
