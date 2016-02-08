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

unsigned short HitPatternVXD::getNdf() const
{
  return 2 * getNPXDHits() + getNSVDHits();
}

unsigned short HitPatternVXD::getNVXDLayers(const PXDMode& pxdmode) const
{
  return getNPXDLayers(pxdmode) + getNSVDLayers();
}

void HitPatternVXD::setSVDLayer(const unsigned short svdLayer, unsigned short uHits, unsigned short vHits)
{
  // take care of input which could result in wrong behaviour
  B2ASSERT("SVD layer is out of range.", svdLayer <= 3);
  resetSVDLayer(svdLayer);
  if (uHits > 3) uHits = 3;
  if (vHits > 3) vHits = 3;
  // set the bits
  std::bitset<32> uHitPattern(uHits);
  uHitPattern <<= s_svdUshift + 4 * svdLayer;
  std::bitset<32> vHitPattern(vHits);
  vHitPattern <<= s_svdVshift + 4 * svdLayer;
  m_pattern |= (uHitPattern | vHitPattern);
}

std::pair<const unsigned short, const unsigned short> HitPatternVXD::getSVDLayer(const unsigned short svdLayer) const
{
  B2ASSERT("SVD layer is out of range.", svdLayer <= 3);
  std::bitset<32> uHitsPattern(m_pattern & (s_SVDLayerMasks[svdLayer] & s_SVDuvMasks[0]));
  std::bitset<32> vHitsPattern(m_pattern & (s_SVDLayerMasks[svdLayer] & s_SVDuvMasks[1]));
  const unsigned short uHits = (uHitsPattern >>= (s_svdUshift + 4 * svdLayer)).to_ulong();
  const unsigned short vHits = (vHitsPattern >>= (s_svdVshift + 4 * svdLayer)).to_ulong();
  return std::make_pair(uHits, vHits);
}

void HitPatternVXD::resetSVDLayer(const unsigned short svdLayer)
{
  B2ASSERT("SVD layer is out of range.", svdLayer <= 3);
  m_pattern &= ~s_SVDLayerMasks[svdLayer];
  return;
}

unsigned short HitPatternVXD::getNSVDHits() const
{
  unsigned short svdHits = 0;
  for (unsigned short svdLayer = 0; svdLayer <= 3; ++svdLayer) {
    std::pair<const unsigned short, const unsigned short> svdHitPair = getSVDLayer(svdLayer);
    svdHits += svdHitPair.first + svdHitPair.second;
  }
  return svdHits;
}

unsigned short HitPatternVXD::getNSVDLayers() const
{
  unsigned short nSVD = 0;
  // there are 4 SVD layers ...
  for (unsigned short layer = 0; layer < 4; ++layer) {
    std::pair<const unsigned short, const unsigned short> hits(getSVDLayer(layer));
    if ((hits.first + hits.second) > 0) ++nSVD;
  }
  return nSVD;
}

short HitPatternVXD::getFirstSVDLayer() const
{
  // there are 4 SVD layers ...
  for (unsigned short layer = 0; layer < 4; ++layer) {
    std::pair<const unsigned short, const unsigned short> hits(getSVDLayer(layer));
    if ((hits.first + hits.second) > 0) return layer;
  }
  return -1;
}

short HitPatternVXD::getLastSVDLayer() const
{
  // there are 4 SVD layers ...
  for (short layer = 3; layer >= 0; --layer) {
    std::pair<const unsigned short, const unsigned short> hits(getSVDLayer(layer));
    if ((hits.first + hits.second) > 0) return layer;
  }
  return -1;
}

void HitPatternVXD::setPXDLayer(const unsigned short pxdLayer, unsigned short nHits, const PXDMode& mode)
{
  B2ASSERT("PXD layer is out of range.", pxdLayer <= 1);
  // take care of human error
  resetPXDLayer(pxdLayer, mode);
  if (nHits > 3) nHits = 3;
  // set hits
  std::bitset<32> hits(nHits);
  hits <<= 4 * pxdLayer + 2 * static_cast<unsigned short>(mode);
  m_pattern |= hits;
}

unsigned short HitPatternVXD::getPXDLayer(const unsigned short pxdLayer, const PXDMode& mode) const
{
  B2ASSERT("PXD layer is out of range.", pxdLayer <= 1);
  std::bitset<32> hits(m_pattern & (s_PXDLayerMasks[pxdLayer] & s_PXDModeMasks[static_cast<unsigned short>(mode)]));
  return (hits >>= (4 * pxdLayer + 2 * static_cast<unsigned short>(mode))).to_ulong();
}

void HitPatternVXD::resetPXDLayer(const unsigned short pxdLayer, const PXDMode& mode)
{
  B2ASSERT("PXD layer is out of range.", pxdLayer <= 1);
  m_pattern &= ~(s_PXDLayerMasks[pxdLayer] & s_PXDModeMasks[static_cast<unsigned short>(mode)]);
}

unsigned short HitPatternVXD::getNPXDHits() const
{
  unsigned short pxdHits = 0;
  for (unsigned short pxdLayer = 0; pxdLayer <= 1; ++pxdLayer) {
    pxdHits += getPXDLayer(pxdLayer, PXDMode::normal);
    pxdHits += getPXDLayer(pxdLayer, PXDMode::gated);
  }
  return pxdHits;
}

unsigned short HitPatternVXD::getNPXDLayers(const PXDMode& mode) const
{
  unsigned short nPXD = 0;
  // there are 2 PXD layers ...
  for (unsigned short layer = 0; layer < 2; ++layer) {
    if (getPXDLayer(layer, mode) > 0) ++nPXD;
  }
  return nPXD;
}

short HitPatternVXD::getFirstPXDLayer(const PXDMode& mode) const
{
  for (unsigned short layer = 0; layer < 2; ++layer) {
    if (getPXDLayer(layer, mode) > 0) return layer;
  }
  return -1;
}

short HitPatternVXD::getLastPXDLayer(const PXDMode& mode) const
{
  for (short layer = 1; layer >= 0; --layer) {
    if (getPXDLayer(layer, mode) > 0) return layer;
  }
  return -1;
}
