/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/eklm/EKLMSegmentAlignment.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMSegmentAlignment::EKLMSegmentAlignment()
{
}

EKLMSegmentAlignment::~EKLMSegmentAlignment()
{
}

void EKLMSegmentAlignment::setSegmentAlignment(uint16_t segment,
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

const KLMAlignmentData* EKLMSegmentAlignment::getSegmentAlignment(
  uint16_t segment) const
{
  std::map<uint16_t, KLMAlignmentData>::const_iterator it;
  it = m_SegmentAlignment.find(segment);
  if (it == m_SegmentAlignment.end())
    return nullptr;
  return &(it->second);
}

double EKLMSegmentAlignment::getGlobalParam(unsigned short element,
                                            unsigned short param) const
{
  const KLMAlignmentData* alignmentData = getSegmentAlignment(element);
  if (alignmentData == nullptr)
    return 0;
  return alignmentData->getParameter(
           static_cast<enum KLMAlignmentData::ParameterNumbers>(param));
}

void EKLMSegmentAlignment::setGlobalParam(double value, unsigned short element,
                                          unsigned short param)
{
  KLMAlignmentData* alignmentData =
    const_cast<KLMAlignmentData*>(getSegmentAlignment(element));
  /*
   * Create alignment data if it does not exist.
   * This is necessary for errors and corrections.
   */
  if (alignmentData == nullptr) {
    KLMAlignmentData newAlignmentData(0, 0, 0, 0, 0, 0);
    newAlignmentData.setParameter(
      static_cast<enum KLMAlignmentData::ParameterNumbers>(param), value);
    m_SegmentAlignment.insert(
      std::pair<uint16_t, KLMAlignmentData>(element, newAlignmentData));
  } else {
    alignmentData->setParameter(
      static_cast<enum KLMAlignmentData::ParameterNumbers>(param), value);
  }
}

/* TODO: this function is not implemented. */
std::vector< std::pair<unsigned short, unsigned short> >
EKLMSegmentAlignment::listGlobalParams()
{
  return {};
}
