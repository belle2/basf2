# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Functions that work on photon lists.

Contributor(s): Torben Ferber
                Sam Cunliffe
"""

from basf2 import *
import string
import random
from analysisPath import *
from variables import variables
from modularAnalysis import *


def getRandomId(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))


def writeClosestPhotonExtraInfo(
    photonList,
    photonSelection='',
    path=analysis_main
):
    """
    Add various variables to the first photon that are related to their angular separation and kinematics.

    @param photonList Photon list with photon candidates that will have the extra information in the end
    @param photonSelection Selection for the other photon
    @param path modules are added to this path
    """

    # build rest of event
    buildRestOfEvent(photonList, path=analysis_main)

    # create new path for ROE
    roe_path = create_path()

    # get random listnames (in case we run this function multiple times)
    pListPair = 'vpho:writeClosestPhotonExtraInfo' + getRandomId()
    pList0 = 'gamma:writeClosestPhotonExtraInfo' + getRandomId()
    pList1 = 'gamma:writeClosestPhotonExtraInfo' + getRandomId()

    fillSignalSideParticleList(pList0, '^' + photonList, path=roe_path)
    fillParticleList(pList1, 'isInRestOfEvent == 1 and ' + photonSelection, path=roe_path)

    reconstructDecay(pListPair + ' -> ' + pList0 + ' ' + pList1, '', path=roe_path)

    # only keep the one with the smallest opening angle
    rankByLowest(pListPair, 'daughterAngleInBetween(0, 1)', 1, path=roe_path)

    # add new variables to the signal side particle
    variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterAngleInBetween(0, 1))': 'openingAngle'}, path=roe_path)
    variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterDiffOf(0, 1, Theta))': 'deltaTheta'}, path=roe_path)
    variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterDiffOf(0, 1, phi))': 'deltaPhi'}, path=roe_path)

    analysis_main.for_each('RestOfEvent', 'RestOfEvents', roe_path)
