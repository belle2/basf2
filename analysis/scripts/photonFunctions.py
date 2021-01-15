# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Functions that work on photon lists.

Contributor(s): Torben Ferber
                Sam Cunliffe
                Michael De Nuccio
"""

from basf2 import create_path
import string
import random
import modularAnalysis as ma


def getRandomId(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))


def writeClosestParticleExtraClusterInfo(
    particleList,
    particleSelection='True',
    roe_path=None,
    deadend_path=None,
    path=None
):
    """
    Add various variables to the first particle that are related to their angular separation and kinematics.

    @param particleList Particle list with particle candidates that will have the extra information in the end
    @param particleSelection Selection for the other particle
    @param roe_path a path for the rest of event to be executed
    @param deadend_path a path for skipping irrelevant RestOfEvent objects that may exist (if this was called twice, for instance)
    @param path modules are added to this path
    """

    particleType = particleList.split(":")[0]

    if not roe_path:
        roe_path = create_path()

    if not deadend_path:
        deadend_path = create_path()

    # build rest of event
    ma.buildRestOfEvent(particleList, path=path)

    # get random listnames (in case we run this function multiple times)
    pListPair = 'vpho:writeClosestParticleExtraClusterInfo' + getRandomId()
    pList0 = particleType + ':writeClosestParticleExtraClusterInfo' + getRandomId()
    pList1 = particleType + ':writeClosestParticleExtraClusterInfo' + getRandomId()

    ma.signalSideParticleFilter(particleList, '', roe_path, deadend_path)

    ma.fillSignalSideParticleList(pList0, '^' + particleList, path=roe_path)

    ma.fillParticleList(pList1, 'isInRestOfEvent == 1 and ' + particleSelection, path=roe_path)

    ma.reconstructDecay(pListPair + ' -> ' + pList0 + ' ' + pList1, '', path=roe_path)

    # only keep the one with the smallest opening angle
    ma.rankByLowest(pListPair, 'daughterClusterAngleInBetween(0, 1)', 1, path=roe_path)

    # add new variables to the signal side particle
    # NOTE: if somebody needs these variables in the CMS frame, they need to
    # modify this function since it's not possible outside the roe_path
    ma.variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterClusterAngleInBetween(0, 1))': 'openingAngle'}, path=roe_path)
    ma.variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterDiffOf(0, 1, clusterTheta))': 'deltaTheta'}, path=roe_path)
    ma.variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterDiffOfClusterPhi(0, 1))': 'deltaPhi'}, path=roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
