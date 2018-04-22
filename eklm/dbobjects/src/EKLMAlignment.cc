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
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMAlignment::EKLMAlignment()
{
}

EKLMAlignment::~EKLMAlignment()
{
}

void EKLMAlignment::setSectorAlignment(uint16_t segment,
                                       EKLMAlignmentData* dat)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_SectorAlignment.find(segment);
  if (it == m_SectorAlignment.end()) {
    m_SectorAlignment.insert(
      std::pair<uint16_t, EKLMAlignmentData>(segment, *dat));
  } else
    it->second = *dat;
}

EKLMAlignmentData* EKLMAlignment::getSectorAlignment(uint16_t segment)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_SectorAlignment.find(segment);
  if (it == m_SectorAlignment.end())
    return NULL;
  return &(it->second);
}

void EKLMAlignment::setSegmentAlignment(uint16_t segment,
                                        EKLMAlignmentData* dat)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_SegmentAlignment.find(segment);
  if (it == m_SegmentAlignment.end()) {
    m_SegmentAlignment.insert(
      std::pair<uint16_t, EKLMAlignmentData>(segment, *dat));
  } else
    it->second = *dat;
}

EKLMAlignmentData* EKLMAlignment::getSegmentAlignment(uint16_t segment)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_SegmentAlignment.find(segment);
  if (it == m_SegmentAlignment.end())
    return NULL;
  return &(it->second);
}

void EKLMAlignment::set(EKLMElementID element, int parameter, double value)
{
  int sector, segment;
  EKLMAlignmentData* sectorAlignment, *segmentAlignment;
  if (element.getType() == EKLMElementID::c_Sector) {
    sector = element.getSectorNumber();
    sectorAlignment = getSectorAlignment(sector);
    if (sectorAlignment == NULL) {
      B2FATAL("EKLM sector alignment data not found, "
              "probable error in sector number.");
    }
    switch (parameter) {
      case 1:
        sectorAlignment->setDx(value);
        break;
      case 2:
        sectorAlignment->setDy(value);
        break;
      case 3:
        sectorAlignment->setDalpha(value);
        break;
      default:
        B2FATAL("Incorrect EKLM alignment parameter " << parameter);
    }
  } else {
    segment = element.getSegmentNumber();
    segmentAlignment = getSegmentAlignment(segment);
    if (segmentAlignment == NULL) {
      B2FATAL("EKLM segment alignment data not found, "
              "probable error in segment number.");
    }
    switch (parameter) {
      case 1:
        segmentAlignment->setDy(value);
        break;
      case 2:
        segmentAlignment->setDalpha(value);
        break;
      default:
        B2FATAL("Incorrect EKLM alignment parameter " << parameter);
    }
  }
}

double EKLMAlignment::getGlobalParam(unsigned short element,
                                     unsigned short param)
{
  EKLMAlignmentData* alignmentData;
  EKLMElementID id(element);
  alignmentData = getSectorAlignment(id.getSectorNumber());
  if (alignmentData == NULL)
    return 0;
  switch (param) {
    case 1:
      return alignmentData->getDx();
    case 2:
      return alignmentData->getDy();
    case 6:
      return alignmentData->getDalpha();
  }
  B2FATAL("Attempt to get EKLM alignment parameter with incorrect number " <<
          param);
  return 0;
}

void EKLMAlignment::setGlobalParam(double value, unsigned short element,
                                   unsigned short param)
{
  EKLMAlignmentData* alignmentData;
  EKLMElementID id(element);
  alignmentData = getSectorAlignment(id.getSectorNumber());
  if (alignmentData == NULL)
    return;
  switch (param) {
    case 1:
      alignmentData->setDx(value);
      return;
    case 2:
      alignmentData->setDy(value);
      return;
    case 6:
      alignmentData->setDalpha(value);
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
