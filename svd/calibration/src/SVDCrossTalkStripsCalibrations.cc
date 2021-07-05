/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDCrossTalkStripsCalibrations.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>


std::string Belle2::SVDCrossTalkStripsCalibrations::name = std::string("SVDCrossTalkStripsCalibrations");


bool Belle2::SVDCrossTalkStripsCalibrations::isInCrossTalkRegion(const VxdID& sensorID, const bool& isU , const float& clsPosition)
{
  const VXD::SensorInfoBase* currentSensorInfo = dynamic_cast<const VXD::SensorInfoBase*>(&VXD::GeoCache::get(
                                                   sensorID)); /** SensorInfo class providing information about the current sensor*/

  float m_uPitch = currentSensorInfo->getUPitch(0); /** getting the pitch dimension for u-side*/

  float m_vPitch = currentSensorInfo->getVPitch(0); /** getting the pitch dimension for v-side*/

  float currentPitch;
  if (isU) currentPitch = m_uPitch;
  else currentPitch = m_vPitch;

  return isCrossTalkStrip(sensorID, isU, (int)(clsPosition / currentPitch + 0.5));
}

