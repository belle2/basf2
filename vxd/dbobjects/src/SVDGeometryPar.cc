/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/SVDGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***

/*
void SVDGeometryPar::read(const GearDir& content)
{
  B2INFO("Parsing SVD.xml to create SVDGeometryPar payload");
}
*/

VXDSensorInfoBasePar* SVDGeometryPar::createSensorInfo(const GearDir& sensor)
{

  /*
  SensorInfo* info = new SensorInfo(
        VxdID(0, 0, 0),
        sensor.getLength("width"),
        sensor.getLength("length"),
        sensor.getLength("height"),
        sensor.getInt("pixelsU"),
        sensor.getInt("pixelsV[1]"),
        sensor.getLength("splitLength", 0),
        sensor.getInt("pixelsV[2]", 0)
  );
  info->setDEPFETParams(
        sensor.getDouble("BulkDoping") / (Unit::um * Unit::um * Unit::um),
        sensor.getWithUnit("BackVoltage"),
        sensor.getWithUnit("TopVoltage"),
        sensor.getLength("SourceBorderSmallPixel"),
        sensor.getLength("ClearBorderSmallPixel"),
        sensor.getLength("DrainBorderSmallPixel"),
        sensor.getLength("SourceBorderLargePixel"),
        sensor.getLength("ClearBorderLargePixel"),
        sensor.getLength("DrainBorderLargePixel"),
        sensor.getLength("GateDepth"),
        sensor.getBool("DoublePixel"),
        sensor.getDouble("ChargeThreshold"),
        sensor.getDouble("NoiseFraction")
  );
  info->setIntegrationWindow(
        sensor.getTime("IntegrationStart"),
        sensor.getTime("IntegrationEnd")
  );
  m_SensorInfo.push_back(info);
  return info;
  */

  B2INFO(" create SVD sensor info: interstripcapacitance is " << sensor.getDouble("InterstripCapacitance"));
  return 0;
}

