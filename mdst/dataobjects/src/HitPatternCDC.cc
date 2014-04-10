/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dataobjects/HitPatternCDC.h>

#include<framework/logging/Logger.h>

#include <string>

using namespace Belle2;

ClassImp(HitPatternCDC);

std::bitset<64> sLayerZer(static_cast<std::string>("0000000000000000000000000000000000000000000000000000000011111111"));
std::bitset<64> sLayerOne(static_cast<std::string>("0000000000000000000000000000000000000000000000000011111100000000"));
std::bitset<64> sLayerTwo(static_cast<std::string>("0000000000000000000000000000000000000000000011111100000000000000"));
std::bitset<64> sLayerThr(static_cast<std::string>("0000000000000000000000000000000000000011111100000000000000000000"));
std::bitset<64> sLayerFou(static_cast<std::string>("0000000000000000000000000000000011111100000000000000000000000000"));
std::bitset<64> sLayerFiv(static_cast<std::string>("0000000000000000000000000011111100000000000000000000000000000000"));
std::bitset<64> sLayerSix(static_cast<std::string>("0000000000000000000011111100000000000000000000000000000000000000"));
std::bitset<64> sLayerSev(static_cast<std::string>("0000000000000011111100000000000000000000000000000000000000000000"));
std::bitset<64> sLayerEig(static_cast<std::string>("0000000011111100000000000000000000000000000000000000000000000000"));

const std::bitset<64> HitPatternCDC::s_sLayerMasks[9] = {sLayerZer, sLayerOne, sLayerTwo, sLayerThr, sLayerFou,
                                                         sLayerFiv, sLayerSix, sLayerSev, sLayerEig
                                                        };

std::bitset<64> infoLayer(static_cast<std::string>("1111111100000000000000000000000000000000000000000000000000000000"));

const std::bitset<64> HitPatternCDC::s_infoLayerMask = infoLayer;


const std::vector<unsigned short> HitPatternCDC::s_indexMin = {0, 8, 14, 20, 26, 32, 38, 44, 50};
const std::vector<unsigned short> HitPatternCDC::s_indexMax = {7, 13, 19, 25, 31, 37, 43, 49, 55};

void HitPatternCDC::layerRangeCheck(const unsigned short layer) const
{
  return;
}
void HitPatternCDC::sLayerRangeCheck(const unsigned short slayer) const
{
  return;
}
