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

std::bitset<32> pxdLayerZer(static_cast<std::string>("00000000000000000000000000001111"));
std::bitset<32> pxdLayerOne(static_cast<std::string>("00000000000000000000000011110000"));

const std::bitset<32> HitPatternVXD::s_PXDLayerMasks[2] = {
  pxdLayerZer, pxdLayerOne
};

std::bitset<32> pxdNormMode(static_cast<std::string>("00000000000000000000000000110011"));
std::bitset<32> pxdGateMode(static_cast<std::string>("00000000000000000000000011001100"));

const std::bitset<32> HitPatternVXD::s_PXDModeMasks[2] = {
  pxdNormMode, pxdGateMode
};

std::bitset<32> svdLayerZer(static_cast<std::string>("00000000000000000000111100000000"));
std::bitset<32> svdLayerOne(static_cast<std::string>("00000000000000001111000000000000"));
std::bitset<32> svdLayerTwo(static_cast<std::string>("00000000000011110000000000000000"));
std::bitset<32> svdLayerThr(static_cast<std::string>("00000000111100000000000000000000"));

const std::bitset<32> HitPatternVXD::s_SVDLayerMasks[4] = {
  svdLayerZer, svdLayerOne, svdLayerTwo, svdLayerThr
};

std::bitset<32> SVDuLayer(static_cast<std::string>("00000000001100110011001100000000"));
std::bitset<32> SVDvLayer(static_cast<std::string>("00000000110011001100110000000000"));

const std::bitset<32> HitPatternVXD::s_SVDuvMasks[2] = {
  SVDuLayer, SVDvLayer
};

std::bitset<32> infoLayerVXD(static_cast<std::string>("11111111000000000000000000000000"));

const std::bitset<32> HitPatternVXD::s_infoLayerMask = infoLayerVXD;



