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
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***

/*
void SVDGeometryPar::read(const GearDir& content)
{
}
*/

SVDGeometryPar::~SVDGeometryPar()
{
  // FIXME: delete sensorInfo causes run time crashes, outcomment it for the moment
  //for (SVDSensorInfoPar* sensorInfo : m_SensorInfo) delete sensorInfo;
  //m_SensorInfo.clear();
}

void SVDGeometryPar::createHalfShellSupport(GearDir support)
{

  for (const GearDir& rotationsolid : support.getNodes("RotationSolid")) {
    m_halfShell.push_back(VXDRotationSolidPar(rotationsolid));
  }
  return;
}




void SVDGeometryPar::createLayerSupport(int layer, GearDir support)
{

}


void SVDGeometryPar::createLadderSupport(int layer, GearDir support)
{

}



VXDSensorInfoBasePar* SVDGeometryPar::createSensorInfo(const GearDir& sensor)
{

  SVDSensorInfoPar* info = new SVDSensorInfoPar(
    VxdID(0, 0, 0),
    sensor.getLength("width"),
    sensor.getLength("length"),
    sensor.getLength("height"),
    sensor.getInt("stripsU"),
    sensor.getInt("stripsV"),
    sensor.getLength("width2", 0)
  );

  const double unit_pF = 1000 * Unit::fC / Unit::V; // picofarad
  info->setSensorParams(
    sensor.getWithUnit("DepletionVoltage"),
    sensor.getWithUnit("BiasVoltage"),
    sensor.getDouble("BackplaneCapacitance") * unit_pF,
    sensor.getDouble("InterstripCapacitance") * unit_pF,
    sensor.getDouble("CouplingCapacitance") * unit_pF,
    sensor.getWithUnit("ElectronicNoiseU"),
    sensor.getWithUnit("ElectronicNoiseV")
  );

  m_SensorInfo.push_back(info);
  return info;
}

