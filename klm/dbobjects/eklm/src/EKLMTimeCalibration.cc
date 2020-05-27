/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/eklm/EKLMTimeCalibration.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMTimeCalibration::EKLMTimeCalibration()
{
}

EKLMTimeCalibration::~EKLMTimeCalibration()
{
}

void EKLMTimeCalibration::
setTimeCalibrationData(uint16_t strip, EKLMTimeCalibrationData* dat)
{
  std::map<uint16_t, EKLMTimeCalibrationData>::iterator it;
  it = m_data.find(strip);
  if (it == m_data.end())
    m_data.insert(std::pair<uint16_t, EKLMTimeCalibrationData>(strip, *dat));
  else
    B2WARNING("TimeCalibration data for the strip already exists.");
}

const EKLMTimeCalibrationData*
EKLMTimeCalibration::getTimeCalibrationData(uint16_t strip) const
{
  std::map<uint16_t, EKLMTimeCalibrationData>::const_iterator it;
  it = m_data.find(strip);
  if (it == m_data.end())
    return nullptr;
  return &(it->second);
}
