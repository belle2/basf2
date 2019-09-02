/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMAlignment::EKLMAlignment()
{
}

EKLMAlignment::~EKLMAlignment()
{
}

void EKLMAlignment::setSectorAlignment(uint16_t segment,
                                       KLMAlignmentData* dat)
{
  std::map<uint16_t, KLMAlignmentData>::iterator it;
  it = m_SectorAlignment.find(segment);
  if (it == m_SectorAlignment.end()) {
    m_SectorAlignment.insert(
      std::pair<uint16_t, KLMAlignmentData>(segment, *dat));
  } else
    it->second = *dat;
}

const KLMAlignmentData* EKLMAlignment::getSectorAlignment(
  uint16_t segment) const
{
  std::map<uint16_t, KLMAlignmentData>::const_iterator it;
  it = m_SectorAlignment.find(segment);
  if (it == m_SectorAlignment.end())
    return nullptr;
  return &(it->second);
}

void EKLMAlignment::setSegmentAlignment(uint16_t segment,
                                        KLMAlignmentData* dat)
{
  std::map<uint16_t, KLMAlignmentData>::iterator it;
  it = m_SegmentAlignment.find(segment);
  if (it == m_SegmentAlignment.end()) {
    m_SegmentAlignment.insert(
      std::pair<uint16_t, KLMAlignmentData>(segment, *dat));
  } else
    it->second = *dat;
}

const KLMAlignmentData* EKLMAlignment::getSegmentAlignment(
  uint16_t segment) const
{
  std::map<uint16_t, KLMAlignmentData>::const_iterator it;
  it = m_SegmentAlignment.find(segment);
  if (it == m_SegmentAlignment.end())
    return nullptr;
  return &(it->second);
}

double EKLMAlignment::getGlobalParam(unsigned short element,
                                     unsigned short param) const
{
  const KLMAlignmentData* alignmentData;
  EKLMElementID id(element);
  alignmentData = getSectorAlignment(id.getSectorNumber());
  if (alignmentData == nullptr)
    return 0;
  switch (param) {
    case 1:
      return alignmentData->getDeltaU();
    case 2:
      return alignmentData->getDeltaW();
    case 6:
      return alignmentData->getDeltaGamma();
  }
  B2FATAL("Attempt to get EKLM alignment parameter with incorrect number " <<
          param);
  return 0;
}

void EKLMAlignment::setGlobalParam(double value, unsigned short element,
                                   unsigned short param)
{
  KLMAlignmentData* alignmentData;
  EKLMElementID id(element);
  alignmentData = const_cast<KLMAlignmentData*>(
                    getSectorAlignment(id.getSectorNumber()));
  if (alignmentData == nullptr)
    return;
  switch (param) {
    case 1:
      alignmentData->setDeltaU(value);
      return;
    case 2:
      alignmentData->setDeltaW(value);
      return;
    case 6:
      alignmentData->setDeltaGamma(value);
      return;
  }
  B2FATAL("Attempt to set EKLM alignment parameter with incorrect number " <<
          param);
}

/* TODO: this function is not implemented. */
std::vector<std::pair<unsigned short, unsigned short>>
                                                    EKLMAlignment::listGlobalParams()
{
  return {};
}
