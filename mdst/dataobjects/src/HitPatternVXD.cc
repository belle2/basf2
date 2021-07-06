/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dataobjects/HitPatternVXD.h>
#include <framework/logging/Logger.h>


using namespace Belle2;

const std::list<unsigned short> HitPatternVXD::s_PXDLayerNumbers = {1, 2};
const std::list<unsigned short> HitPatternVXD::s_SVDLayerNumbers = {3, 4, 5, 6};

const std::bitset<32> HitPatternVXD::s_LayerMasks[6] = {
  std::bitset<32>(static_cast<std::string>("00000000000000000000000000001111")),  // layer1 PXD
  std::bitset<32>(static_cast<std::string>("00000000000000000000000011110000")),  // layer2 PXD
  std::bitset<32>(static_cast<std::string>("00000000000000000000111100000000")),  // layer3 SVD
  std::bitset<32>(static_cast<std::string>("00000000000000001111000000000000")),  // layer4 SVD
  std::bitset<32>(static_cast<std::string>("00000000000011110000000000000000")),  // layer5 SVD
  std::bitset<32>(static_cast<std::string>("00000000111100000000000000000000"))   // layer6 SVD
};

const std::bitset<32> HitPatternVXD::s_PXDModeMasks[2] = {
  std::bitset<32>(static_cast<std::string>("00000000000000000000000000110011")),  // normal mode
  std::bitset<32>(static_cast<std::string>("00000000000000000000000011001100"))   // gated mode
};

const std::bitset<32> HitPatternVXD::s_SVDuvMasks[2] = {
  std::bitset<32>(static_cast<std::string>("00000000001100110011001100000000")),  // u layer
  std::bitset<32>(static_cast<std::string>("00000000110011001100110000000000"))   // v layer
};

const std::bitset<32> HitPatternVXD::s_V0DaughterMask(static_cast<std::string>("00000011000000000000000000000000"));

const std::bitset<32> HitPatternVXD::s_infoLayerMask(static_cast<std::string>("11111100000000000000000000000000"));

unsigned short HitPatternVXD::getNdf() const
{
  return 2 * getNPXDHits() + getNSVDHits();
}

unsigned short HitPatternVXD::getNVXDLayers(const PXDMode& pxdmode) const
{
  return getNPXDLayers(pxdmode) + getNSVDLayers();
}

void HitPatternVXD::setSVDLayer(const unsigned short layerId, unsigned short uHits, unsigned short vHits)
{
  B2ASSERT("SVD layer is out of range.", 3 <= layerId and layerId <= 6);
  resetSVDLayer(layerId);
  if (uHits > 3) uHits = 3;
  if (vHits > 3) vHits = 3;
  const unsigned short arrayId = layerId - 3;
  std::bitset<32> uHitPattern(uHits);
  uHitPattern <<= s_svdUshift + 4 * arrayId;
  std::bitset<32> vHitPattern(vHits);
  vHitPattern <<= s_svdVshift + 4 * arrayId;
  m_pattern |= (uHitPattern | vHitPattern);
}

std::pair<const unsigned short, const unsigned short> HitPatternVXD::getSVDLayer(const unsigned short layerId) const
{
  B2ASSERT("SVD layer is out of range.", 3 <= layerId and layerId <= 6);
  std::bitset<32> uHitsPattern(m_pattern & (getLayerMask(layerId) & s_SVDuvMasks[0]));
  std::bitset<32> vHitsPattern(m_pattern & (getLayerMask(layerId) & s_SVDuvMasks[1]));
  const unsigned short arrayId = layerId - 3;
  const unsigned short uHits = (uHitsPattern >> (s_svdUshift + 4 * arrayId)).to_ulong();
  const unsigned short vHits = (vHitsPattern >> (s_svdVshift + 4 * arrayId)).to_ulong();
  return std::make_pair(uHits, vHits);
}

void HitPatternVXD::resetSVDLayer(const unsigned short layerId)
{
  B2ASSERT("SVD layer is out of range.", 3 <= layerId and layerId <= 6);
  m_pattern &= ~getLayerMask(layerId);
  return;
}

unsigned short HitPatternVXD::getNSVDHits() const
{
  unsigned short svdHits = 0;
  for (const auto layerId : s_SVDLayerNumbers) {
    std::pair<const unsigned short, const unsigned short> svdHitPair = getSVDLayer(layerId);
    svdHits += svdHitPair.first + svdHitPair.second;
  }
  return svdHits;
}

unsigned short HitPatternVXD::getNSVDLayers() const
{
  unsigned short nSVD = 0;
  for (const auto layerId : s_SVDLayerNumbers) {
    std::pair<const unsigned short, const unsigned short> hits(getSVDLayer(layerId));
    if ((hits.first + hits.second) > 0) ++nSVD;
  }
  return nSVD;
}

short HitPatternVXD::getFirstSVDLayer() const
{
  for (const auto layerId : s_SVDLayerNumbers) {
    std::pair<const unsigned short, const unsigned short> hits(getSVDLayer(layerId));
    if ((hits.first + hits.second) > 0) return layerId;
  }
  return -1;
}

short HitPatternVXD::getLastSVDLayer() const
{
  for (auto rit = s_SVDLayerNumbers.rbegin(); rit != s_SVDLayerNumbers.rend(); ++rit) {
    const auto layerId = *rit;
    std::pair<const unsigned short, const unsigned short> hits(getSVDLayer(layerId));
    if ((hits.first + hits.second) > 0) return layerId;
  }
  return -1;
}

void HitPatternVXD::setPXDLayer(const unsigned short layerId, unsigned short nHits, const PXDMode& mode)
{
  B2ASSERT("PXD layer is out of range.", layerId == 1 or layerId == 2);
  resetPXDLayer(layerId, mode);
  if (nHits > 3) nHits = 3;  // A Maximum of 3 can be stored in 2 bits.
  std::bitset<32> hits(nHits);
  const unsigned short arrayId = layerId - 1;
  hits <<= 4 * arrayId + 2 * static_cast<unsigned short>(mode);
  m_pattern |= hits;
}

unsigned short HitPatternVXD::getPXDLayer(const unsigned short layerId, const PXDMode& mode) const
{
  B2ASSERT("PXD layer is out of range.", layerId == 1 or layerId == 2);
  const unsigned short arrayId = layerId - 1;
  std::bitset<32> hits(m_pattern & (getLayerMask(layerId) & s_PXDModeMasks[static_cast<unsigned short>(mode)]));
  return (hits >> (4 * arrayId + 2 * static_cast<unsigned short>(mode))).to_ulong();
}

void HitPatternVXD::resetPXDLayer(const unsigned short layerId, const PXDMode& mode)
{
  B2ASSERT("PXD layer is out of range.", layerId == 1 or layerId == 2);
  m_pattern &= ~(getLayerMask(layerId) & s_PXDModeMasks[static_cast<unsigned short>(mode)]);
}

unsigned short HitPatternVXD::getNPXDHits() const
{
  unsigned short pxdHits = 0;
  for (const auto layerId : s_PXDLayerNumbers) {
    pxdHits += getPXDLayer(layerId, PXDMode::normal);
    pxdHits += getPXDLayer(layerId, PXDMode::gated);
  }
  return pxdHits;
}

unsigned short HitPatternVXD::getNPXDLayers(const PXDMode& mode) const
{
  unsigned short nPXD = 0;
  for (const auto layerId : s_PXDLayerNumbers) {
    if (getPXDLayer(layerId, mode) > 0) ++nPXD;
  }
  return nPXD;
}

short HitPatternVXD::getFirstPXDLayer(const PXDMode& mode) const
{
  for (const auto layerId : s_PXDLayerNumbers) {
    if (getPXDLayer(layerId, mode) > 0) return layerId;
  }
  return -1;
}

short HitPatternVXD::getLastPXDLayer(const PXDMode& mode) const
{
  for (auto rit = s_PXDLayerNumbers.rbegin(); rit != s_PXDLayerNumbers.rend(); ++rit) {
    const auto layerId = *rit;
    if (getPXDLayer(layerId, mode) > 0) return layerId;
  }
  return -1;
}

void HitPatternVXD::setInnermostHitShareStatus(const unsigned short innermostHitShareStatus)
{
  B2ASSERT("Information is out of range. Information: " << innermostHitShareStatus << ", range: " << (s_V0DaughterMask >>
           (4 * 6)).to_ulong(),
           innermostHitShareStatus <= (s_V0DaughterMask >> (4 * 6)).to_ulong());
  resetInnermostHitShareStatus();
  std::bitset<32> hits(innermostHitShareStatus);
  hits <<= 4 * 6;
  m_pattern |= hits;
}

unsigned short HitPatternVXD::getInnermostHitShareStatus() const
{
  std::bitset<32> hits(m_pattern & s_V0DaughterMask);
  return (hits >> (4 * 6)).to_ulong();
}

void HitPatternVXD::resetInnermostHitShareStatus()
{
  m_pattern &= ~(s_V0DaughterMask);
}

std::string HitPatternVXD::__str__() const
{
  return m_pattern.to_string();
}
