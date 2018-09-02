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

unsigned short HitPatternCDC::getNHits() const
{
  // Shift the 8 MSBs to the right and return their value as integer.
  return static_cast<unsigned short int>((m_pattern >> 56).to_ulong());
}

void HitPatternCDC::setNHits(unsigned short nHits)
{
  if (nHits > 256) {
    // Maximum with 8 available bits
    nHits = 255;
  }
  // Reset the 8 MSBs to zero.
  m_pattern = m_pattern & ~s_infoLayerMask;
  // Set the total number of hits as the 8 MSBs
  std::bitset<64> numberOfHits(nHits);
  numberOfHits <<= 56;
  // Set the 8 MSBs to the total number of hits.
  // The 8 MSBs have to be zero, otherwise this breaks.
  m_pattern = numberOfHits | m_pattern;
}

ULong64_t HitPatternCDC::getInteger() const
{
  // cppcheck-suppress knownConditionTrueFalse
  if (sizeof(unsigned long) >= 8) {
    return m_pattern.to_ulong();
  } else {
    return m_pattern.to_ullong();
  }
}

void HitPatternCDC::setLayer(const unsigned short layer)
{
  B2ASSERT("Layer is out of range.", layer <= 55);
  m_pattern.set(layer);
}

void HitPatternCDC::resetLayer(const unsigned short layer)
{
  B2ASSERT("Layer is out of range.", layer <= 55);
  m_pattern.reset(layer);
}

bool HitPatternCDC::hasLayer(const unsigned short layer) const
{
  B2ASSERT("Layer is out of range.", layer <= 55);
  return m_pattern[layer];
}

short HitPatternCDC::getFirstLayer() const
{
  for (unsigned int i = 0; i < m_pattern.size(); ++i) {
    if ((m_pattern & ~s_infoLayerMask).test(i)) return i;
  }
  return -1;
}

short HitPatternCDC::getLastLayer() const
{
  // m_pattern.size()-8 because the first 8 bits are not pattern
  for (unsigned int i = m_pattern.size() - 8; i > 0; --i) {
    // -1 because of the index couting...
    if ((m_pattern & ~s_infoLayerMask).test(i - 1)) return i - 1;
  }
  return -1;
}

bool HitPatternCDC::hasSLayer(const unsigned short sLayer) const
{
  B2ASSERT("Super layer outof range.", sLayer <= 8);
  return ((m_pattern & s_sLayerMasks[sLayer]).any());
}

void HitPatternCDC::resetSLayer(const unsigned short sLayer)
{
  B2ASSERT("Super layer outof range.", sLayer <= 8);
  for (unsigned short int ii = 0; ii < m_pattern.size(); ++ii) {
    if ((s_sLayerMasks[sLayer])[ii]) {resetLayer(ii);}
  }
}

std::bitset<64> HitPatternCDC::getSLayerPattern(const unsigned short sLayer)
{
  return m_pattern & s_sLayerMasks[sLayer];
}

unsigned short HitPatternCDC::getSLayerNHits(const unsigned short sLayer) const
{
  B2ASSERT("Super layer outof range.", sLayer <= 8);
  return static_cast<unsigned short>((m_pattern & s_sLayerMasks[sLayer]).count());
}

unsigned short HitPatternCDC::getLongestContRunInSL(const unsigned short sLayer) const
{
  B2ASSERT("Super layer outof range.", sLayer <= 8);
  unsigned short max = 0;
  unsigned short counter = 0;
  std::pair<unsigned short, unsigned short> indices = s_superLayerIndices.at(sLayer);
  for (unsigned short i = indices.first; i <= indices.second; ++i) {
    counter += m_pattern[i];
    if (m_pattern[i] == 0) {
      if (counter > max) {
        max = counter;
      }
      counter = 0;
    }
  }
  return std::max(max, counter);
}

bool HitPatternCDC::hasAxialLayer() const
{
  return ((s_sLayerMasks[0] | s_sLayerMasks[2] | s_sLayerMasks[4] | s_sLayerMasks[6] | s_sLayerMasks[8])
          & m_pattern).any();
}

bool HitPatternCDC::hasStereoLayer() const
{
  return ((s_sLayerMasks[1] | s_sLayerMasks[3] | s_sLayerMasks[5] | s_sLayerMasks[7])
          & m_pattern).any();
}
