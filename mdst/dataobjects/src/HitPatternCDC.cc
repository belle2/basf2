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

std::bitset<64> infoLayerCDC(static_cast<std::string>("1111111100000000000000000000000000000000000000000000000000000000"));

const std::bitset<64> HitPatternCDC::s_infoLayerMask = infoLayerCDC;

const std::map<unsigned short, std::pair<unsigned short, unsigned short>> HitPatternCDC::s_superLayerIndices = {
  {0, std::make_pair(0, 7)},
  {1, std::make_pair(8, 13)},
  {2, std::make_pair(14, 19)},
  {3, std::make_pair(20, 25)},
  {4, std::make_pair(26, 31)},
  {5, std::make_pair(32, 37)},
  {6, std::make_pair(38, 43)},
  {7, std::make_pair(44, 49)},
  {8, std::make_pair(50, 55)}
};
