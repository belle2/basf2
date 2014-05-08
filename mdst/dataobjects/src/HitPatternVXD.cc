/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dataobjects/HitPatternVXD.h>

using namespace Belle2;

ClassImp(HitPatternVXD);

HitPatternVXD::HitPatternVXD(): m_pattern(0)
{

}

std::bitset<32> pxdLayerZerModeA(static_cast<std::string>("00000000000000000000000000000011"));
std::bitset<32> pxdLayerOneModeA(static_cast<std::string>("00000000000000000000000000001100"));
std::bitset<32> pxdLayerTwoModeB(static_cast<std::string>("00000000000000000000000000110000"));
std::bitset<32> pxdLayerThrModeB(static_cast<std::string>("00000000000000000000000011000000"));
std::bitset<32> vxdLayerZer(static_cast<std::string>("00000000000000000000001100000000"));
std::bitset<32> vxdLayerOne(static_cast<std::string>("00000000000000000000110000000000"));
std::bitset<32> vxdLayerTwo(static_cast<std::string>("00000000000000000011000000000000"));
std::bitset<32> vxdLayerThr(static_cast<std::string>("00000000000000001100000000000000"));



const std::bitset<32> HitPatternVXD::s_LayerMasks[8] = {pxdLayerZerModeA, pxdLayerOneModeA, pxdLayerTwoModeB, pxdLayerThrModeB,
                                                        vxdLayerZer, vxdLayerOne, vxdLayerTwo, vxdLayerThr
                                                       };

std::bitset<32> infoLayerVXD(static_cast<std::string>("11111111111111110000000000000000"));

const std::bitset<32> HitPatternVXD::s_infoLayerMask = infoLayerVXD;

const std::vector<unsigned short> HitPatternVXD::s_layerBitOne = {0, 2, 4, 6, 8, 10, 12, 14};
const std::vector<unsigned short> HitPatternVXD::s_layerBitTwo = {1, 3, 5, 7, 9, 11, 13, 15};

