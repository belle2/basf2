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
#include <framework/logging/Logger.h>
#include <klm/dataobjects/KLMAlignableElement.h>
#include <klm/eklm/dbobjects/EKLMAlignment.h>

using namespace Belle2;

EKLMAlignment::EKLMAlignment()
{
}

EKLMAlignment::~EKLMAlignment()
{
}

void EKLMAlignment::setSectorAlignment(uint16_t sector,
                                       KLMAlignmentData* dat)
{
  std::map<uint16_t, KLMAlignmentData>::iterator it;
  it = m_SectorAlignment.find(sector);
  if (it == m_SectorAlignment.end()) {
    m_SectorAlignment.insert(
      std::pair<uint16_t, KLMAlignmentData>(sector, *dat));
  } else {
    it->second = *dat;
  }
}

const KLMAlignmentData* EKLMAlignment::getSectorAlignment(
  uint16_t sector) const
{
  std::map<uint16_t, KLMAlignmentData>::const_iterator it;
  it = m_SectorAlignment.find(sector);
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
  KLMAlignableElement id(element);
  alignmentData = getSectorAlignment(id.getModuleNumber());
  if (alignmentData == nullptr)
    return 0;
  return alignmentData->getParameter(
           static_cast<enum KLMAlignmentData::ParameterNumbers>(param));
}

void EKLMAlignment::setGlobalParam(double value, unsigned short element,
                                   unsigned short param)
{
  KLMAlignmentData* alignmentData;
  KLMAlignableElement id(element);
  alignmentData = const_cast<KLMAlignmentData*>(
                    getSectorAlignment(id.getModuleNumber()));
  if (alignmentData == nullptr)
    return;
  alignmentData->setParameter(
    static_cast<enum KLMAlignmentData::ParameterNumbers>(param), value);
}

/* TODO: this function is not implemented. */
std::vector< std::pair<unsigned short, unsigned short> >
EKLMAlignment::listGlobalParams()
{
  return {};
}
