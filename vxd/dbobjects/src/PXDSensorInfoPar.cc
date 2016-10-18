/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/PXDSensorInfoPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void PXDSensorInfoPar::read(const GearDir& sensor)
{
  m_width = sensor.getLength("width");
  m_length = sensor.getLength("length");
  m_height = sensor.getLength("height");
  m_pixelsU = sensor.getInt("pixelsU");
  m_pixelsV1 = sensor.getInt("pixelsV[1]");
  m_pixelsV2 = sensor.getInt("pixelsV[2]", 0);
  m_splitLength = sensor.getLength("splitLength", 0);

  m_bulkDoping = sensor.getDouble("BulkDoping");
  m_backVoltage = sensor.getWithUnit("BackVoltage");
  m_topVoltage = sensor.getWithUnit("TopVoltage");
  m_sourceBorderSmallPixel = sensor.getLength("SourceBorderSmallPixel");
  m_clearBorderSmallPixel = sensor.getLength("ClearBorderSmallPixel");
  m_drainBorderSmallPixel = sensor.getLength("DrainBorderSmallPixel");
  m_sourceBorderLargePixel = sensor.getLength("SourceBorderLargePixel");
  m_clearBorderLargePixel = sensor.getLength("ClearBorderLargePixel");
  m_drainBorderLargePixel =  sensor.getLength("DrainBorderLargePixel");
  m_gateDepth = sensor.getLength("GateDepth");
  m_doublePixel = sensor.getBool("DoublePixel");
  m_chargeThreshold = sensor.getDouble("ChargeThreshold");
  m_noiseFraction = sensor.getDouble("NoiseFraction");
  m_integrationStartTime = sensor.getTime("IntegrationStart");
  m_integrationEndTime = sensor.getTime("IntegrationEnd");
}
