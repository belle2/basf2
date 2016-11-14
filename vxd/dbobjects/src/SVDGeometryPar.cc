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


bool SVDGeometryPar::getSupportRibsExist(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDSupportRibsPar>::const_iterator cached = m_supportRibs.find(layer);
  if (cached == m_supportRibs.end()) {
    return false;
  }
  return true;
}

bool SVDGeometryPar::getEndringsExist(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDEndringsPar>::const_iterator cached = m_endrings.find(layer);
  if (cached == m_endrings.end()) {
    return false;
  }
  return true;
}

bool SVDGeometryPar::getCoolingPipesExist(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDCoolingPipesPar>::const_iterator cached = m_coolingPipes.find(layer);
  if (cached == m_coolingPipes.end()) {
    return false;
  }
  return true;
}


const SVDSupportRibsPar& SVDGeometryPar::getSupportRibs(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDSupportRibsPar>::const_iterator cached = m_supportRibs.find(layer);
  if (cached == m_supportRibs.end()) {
    B2FATAL("No SupportRibs found for layer " << std::to_string(layer));
  }
  return cached->second;
}

const SVDEndringsPar& SVDGeometryPar::getEndrings(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDEndringsPar>::const_iterator cached = m_endrings.find(layer);
  if (cached == m_endrings.end()) {
    B2FATAL("No Endrings found for layer " << std::to_string(layer));
  }
  return cached->second;
}

const SVDCoolingPipesPar& SVDGeometryPar::getCoolingPipes(int layer) const
{
  //Check if sensorType already exists
  std::map<int, SVDCoolingPipesPar>::const_iterator cached = m_coolingPipes.find(layer);
  if (cached == m_coolingPipes.end()) {
    B2FATAL("No CoolingPipes found for layer " << std::to_string(layer));
  }
  return cached->second;
}

void SVDGeometryPar::createHalfShellSupport(GearDir support)
{
  if (!support) return;

  for (const GearDir& rotationsolid : support.getNodes("HalfShell/RotationSolid")) {
    m_halfShell.push_back(VXDRotationSolidPar(rotationsolid));
  }
  return;
}




void SVDGeometryPar::createLayerSupport(int layer, GearDir support)
{

  if (!support) return;

  //Check if there are any endrings defined for this layer. If not we don't create any
  GearDir endrings(support, (boost::format("Endrings/Layer[@id='%1%']") % layer).str());
  if (endrings) {
    m_endrings[layer] = SVDEndringsPar(layer, support);
  }

  // Now let's add the cooling pipes to the Support
  GearDir pipes(support, (boost::format("CoolingPipes/Layer[@id='%1%']") % layer).str());
  if (pipes) {
    m_coolingPipes[layer] = SVDCoolingPipesPar(layer, support);
  }

  return;
}


void SVDGeometryPar::createLadderSupport(int layer, GearDir support)
{

  if (!support) return;

  // Check if there are any support ribs defined for this layer. If not return empty assembly
  GearDir params(support, (boost::format("SupportRibs/Layer[@id='%1%']") % layer).str());
  if (params) {
    m_supportRibs[layer] = SVDSupportRibsPar(layer, support);
  }

  return;
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

  info->setSensorParams(
    sensor.getWithUnit("DepletionVoltage"),
    sensor.getWithUnit("BiasVoltage"),
    sensor.getDouble("BackplaneCapacitance") ,
    sensor.getDouble("InterstripCapacitance") ,
    sensor.getDouble("CouplingCapacitance") ,
    sensor.getWithUnit("ElectronicNoiseU"),
    sensor.getWithUnit("ElectronicNoiseV")
  );

  m_SensorInfo.push_back(info);
  return info;
}

