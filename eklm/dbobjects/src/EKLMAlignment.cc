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

void EKLMAlignment::setAlignmentData(uint16_t segment, EKLMAlignmentData* dat)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_Data.find(segment);
  if (it == m_Data.end())
    m_Data.insert(std::pair<uint16_t, EKLMAlignmentData>(segment, *dat));
  else
    it->second = *dat;
}

EKLMAlignmentData* EKLMAlignment::getAlignmentData(uint16_t segment)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_Data.find(segment);
  if (it == m_Data.end())
    return NULL;
  return &(it->second);
}

void EKLMAlignment::cleanAlignmentData()
{
  m_Data.clear();
}

