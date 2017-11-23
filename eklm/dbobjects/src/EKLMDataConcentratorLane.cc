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
#include <eklm/dbobjects/EKLMDataConcentratorLane.h>

using namespace Belle2;

EKLMDataConcentratorLane::EKLMDataConcentratorLane()
{
  m_Copper = 0;
  m_DataConcentrator = 0;
  m_Lane = 0;
}

EKLMDataConcentratorLane::~EKLMDataConcentratorLane()
{
}

int EKLMDataConcentratorLane::getCopper() const
{
  return m_Copper;
}

void EKLMDataConcentratorLane::setCopper(int copper)
{
  m_Copper = copper;
}

int EKLMDataConcentratorLane::getDataConcentrator() const
{
  return m_DataConcentrator;
}

void EKLMDataConcentratorLane::setDataConcentrator(int dataConcentrator)
{
  m_DataConcentrator = dataConcentrator;
}

int EKLMDataConcentratorLane::getLane() const
{
  return m_Lane;
}

void EKLMDataConcentratorLane::setLane(int lane)
{
  m_Lane = lane;
}

bool EKLMDataConcentratorLane::operator<(
  const EKLMDataConcentratorLane& lane) const
{
  if (this->getCopper() < lane.getCopper())
    return true;
  else if (this->getCopper() > lane.getCopper())
    return false;
  if (this->getDataConcentrator() < lane.getDataConcentrator())
    return true;
  else if (this->getDataConcentrator() > lane.getDataConcentrator())
    return false;
  return this->getLane() < lane.getLane();
}

