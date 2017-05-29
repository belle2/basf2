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

void EKLMAlignment::add(EKLMElementID element, int parameter,
                        double correction, bool invertSign)
{
  double corr;
  int sector, segment;
  EKLMAlignmentData* sectorAlignment, *segmentAlignment;
  corr = correction;
  if (invertSign)
    corr = -corr;
  if (element.getType() == EKLMElementID::c_Sector) {
    sector = element.getSectorNumber();
    sectorAlignment = getSectorAlignment(sector);
    if (sectorAlignment == NULL) {
      B2FATAL("EKLM sector alignment data not found, "
              "probable error in sector number.");
    }
    switch (parameter) {
      case 1:
        sectorAlignment->setDx(sectorAlignment->getDx() + corr);
        break;
      case 2:
        sectorAlignment->setDy(sectorAlignment->getDy() + corr);
        break;
      case 3:
        sectorAlignment->setDalpha(sectorAlignment->getDalpha() + corr);
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
        segmentAlignment->setDy(segmentAlignment->getDy() + corr);
        break;
      case 2:
        segmentAlignment->setDalpha(segmentAlignment->getDalpha() + corr);
        break;
      default:
        B2FATAL("Incorrect EKLM alignment parameter " << parameter);
    }
  }
}

