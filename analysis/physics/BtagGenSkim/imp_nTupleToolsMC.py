#!/usr/bin/env python
# -*- coding: utf-8 -*-


def appendBTools(toolsList, decayString):
    toolsList += ['DeltaEMbc', decayString]
    toolsList += ['MCTruth', decayString]
    # B info
    toolsList += \
        ['CustomFloats[getExtraInfo(decayModeID):isSignal:abs(daughter(0,genMotherPDG))]'
         , decayString]


def appendBDTools(toolsList, decayString):
    toolsList += ['DeltaEMbc', decayString]
    toolsList += ['MCTruth', decayString]
    # D momentum in CMS
    toolsList += ['CustomFloats[daughter(0,p_CMS)]', decayString]
    # total momentum of the daughters
    toolsList += \
        ['CustomFloats[daughter(0,p):daughter(1,p):daughter(2,p):daughter(3,p):daughter(4,p):daughter(5,p)]'
         , decayString]
    # MC truth for the D
    toolsList += \
        ['CustomFloats[daughter(0,mcPDG):daughter(0,mcStatus):daughter(0,M):daughter(0,ErrM):daughter(0,SigM)]'
         , decayString]
    # invariant masses of daughters
    toolsList += \
        ['CustomFloats[daughterInvariantMass(0,1):daughterInvariantMass(0,2):daughterInvariantMass(0,3):daughterInvariantMass(0,4)]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughterInvariantMass(1,2):daughterInvariantMass(1,3):daughterInvariantMass(2,3):daughterInvariantMass(1,2,3)]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughterInvariantMass(3,4):daughterInvariantMass(1,4):daughterInvariantMass(2,5):daughterInvariantMass(4,5)]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughter(0,daughterInvariantMass(0,1)):daughter(0,daughterInvariantMass(0,2)):daughter(0,daughterInvariantMass(0,3))]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughter(0,daughterInvariantMass(1,2)):daughter(0,daughterInvariantMass(1,3)):daughter(0,daughterInvariantMass(2,3))]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughter(0,daughterInvariantMass(0,1,2)):daughter(0,daughterInvariantMass(0,1,3)):daughter(0,daughterInvariantMass(0,2,3)):daughter(0,daughterInvariantMass(1,2,3))]'
         , decayString]
    # B info
    toolsList += ['CustomFloats[getExtraInfo(decayModeID):isSignal]',
                  decayString]
    # D info
    toolsList += \
        ['CustomFloats[daughter(0,getExtraInfo(decayModeID)):daughter(0,isSignal):abs(daughter(0,genMotherPDG))]'
         , decayString]


def appendBDSTTools(toolsList, decayString):
    toolsList += ['DeltaEMbc', decayString]
    toolsList += ['MCTruth', decayString]
    # D momentum in CMS
    toolsList += ['CustomFloats[daughter(0,p_CMS)]', decayString]
    # total momentum of the daughters
    toolsList += \
        ['CustomFloats[daughter(0,p):daughter(1,p):daughter(2,p):daughter(3,p):daughter(4,p):daughter(5,p)]'
         , decayString]
    # MC truth for the D
    toolsList += \
        ['CustomFloats[daughter(0,mcPDG):daughter(0,mcStatus):daughter(0,M):daughter(0,ErrM):daughter(0,SigM)]'
         , decayString]
    # invariant masses of daughters
    toolsList += \
        ['CustomFloats[daughterInvariantMass(0,1):daughterInvariantMass(0,2):daughterInvariantMass(0,3):daughterInvariantMass(0,4)]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughterInvariantMass(1,2):daughterInvariantMass(1,3):daughterInvariantMass(2,3):daughterInvariantMass(1,2,3)]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughterInvariantMass(3,4):daughterInvariantMass(1,4):daughterInvariantMass(2,5):daughterInvariantMass(4,5)]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughter(0,daughterInvariantMass(0,1)):daughter(0,daughterInvariantMass(0,2)):daughter(0,daughterInvariantMass(0,3))]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughter(0,daughterInvariantMass(1,2)):daughter(0,daughterInvariantMass(1,3)):daughter(0,daughterInvariantMass(2,3))]'
         , decayString]
    toolsList += \
        ['CustomFloats[daughter(0,daughterInvariantMass(0,1,2)):daughter(0,daughterInvariantMass(0,1,3)):daughter(0,daughterInvariantMass(0,2,3)):daughter(0,daughterInvariantMass(1,2,3))]'
         , decayString]
    # B info
    toolsList += ['CustomFloats[getExtraInfo(decayModeID):isSignal]',
                  decayString]
    # DST info
    toolsList += \
        ['CustomFloats[daughter(0,getExtraInfo(decayModeID)):daughter(0,isSignal)]'
         , decayString]
    # D info
    toolsList += \
        ['CustomFloats[daughter(0,daughter(0,getExtraInfo(decayModeID))):daughter(0,daughter(0,isSignal)):abs(daughter(0,genMotherPDG))]'
         , decayString]


def appendDTools(toolsList, decayString):
    toolsList += ['MCKinematics', decayString]
    toolsList += ['MCTruth', decayString]
    # D info
    toolsList += ['CustomFloats[M:getExtraInfo(decayModeID):isSignal]',
                  decayString]


def appendDSTTools(toolsList, decayString):
    toolsList += ['MCKinematics', decayString]
    toolsList += ['MCTruth', decayString]
    # D* info
    toolsList += \
        ['CustomFloats[M:massDifference(0):getExtraInfo(decayModeID):isSignal]'
         , decayString]
    # D info
    toolsList += \
        ['CustomFloats[daughter(0,getExtraInfo(decayModeID)):daughter(0,isSignal)]'
         , decayString]


def appendGenTools(toolsList, decayString):
    toolsList += ['MCKinematics', decayString]
    toolsList += ['MCTruth', decayString]


