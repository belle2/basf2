/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/dbobjects/SVDGeometryPar.h>
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

  for (const GearDir& params : support.getNodes("HalfShell/RotationSolid")) {

    VXDRotationSolidPar rotationSolidPar(params.getString("Name", ""),
                                         params.getString("Material", "Air"),
                                         params.getString("Color", ""),
                                         params.getAngle("minPhi", 0),
                                         params.getAngle("maxPhi", 2 * M_PI),
                                         (params.getNodes("InnerPoints/point").size() > 0)
                                        );

    for (const GearDir point : params.getNodes("InnerPoints/point")) {
      pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
      rotationSolidPar.getInnerPoints().push_back(ZXPoint);
    }
    for (const GearDir point : params.getNodes("OuterPoints/point")) {
      pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
      rotationSolidPar.getOuterPoints().push_back(ZXPoint);
    }
    m_halfShell.push_back(rotationSolidPar);
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
    m_supportRibs[layer] = SVDSupportRibsPar(support.getLength("SupportRibs/spacing"),
                                             support.getLength("SupportRibs/height"),
                                             support.getLength("SupportRibs/inner/width"),
                                             support.getLength("SupportRibs/outer/width"),
                                             support.getLength("SupportRibs/inner/tabLength"),
                                             support.getString("SupportRibs/outer/Material"),
                                             support.getString("SupportRibs/inner/Material"),
                                             support.getString("SupportRibs/outer/Color"),
                                             support.getString("SupportRibs/inner/Color"),
                                             support.getString("SupportRibs/endmount/Material")
                                            );

    // Get values for the layer if available
    if (params.exists("spacing")) m_supportRibs[layer].setSpacing(params.getLength("spacing"));
    if (params.exists("height")) m_supportRibs[layer].setHeight(params.getLength("height"));

    for (const GearDir& box : params.getNodes("box")) {
      SVDSupportBoxPar boxPar(box.getAngle("theta"),
                              box.getLength("z"),
                              box.getLength("r"),
                              box.getLength("length")
                             );
      m_supportRibs[layer].getBoxes().push_back(boxPar);
    }

    for (const GearDir& tab : params.getNodes("tab")) {
      SVDSupportTabPar tabPar(tab.getAngle("theta"),
                              tab.getLength("z"),
                              tab.getLength("r")
                             );
      m_supportRibs[layer].getTabs().push_back(tabPar);
    }

    for (const GearDir& endmount : params.getNodes("Endmount")) {
      SVDEndmountPar mountPar(endmount.getString("@name"),
                              endmount.getLength("height"),
                              endmount.getLength("width"),
                              endmount.getLength("length"),
                              endmount.getLength("z"),
                              endmount.getLength("r")
                             );
      m_supportRibs[layer].getEndmounts().push_back(mountPar);
    }

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

