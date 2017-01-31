/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDGeometryPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <vxd/dataobjects/VxdID.h>

using namespace Belle2;
using namespace std;


std::vector<VXDGeoPlacementPar> VXDGeometryPar::getSubComponents(GearDir path)
{
  vector<VXDGeoPlacementPar> result;
  for (const GearDir& component : path.getNodes("Component")) {
    string type;
    if (!component.exists("@type")) {
      type = component.getString("@name");
    } else {
      type = component.getString("@type");
    }
    int nPos = max(component.getNumberNodes("u"), component.getNumberNodes("v"));
    nPos = max(nPos, component.getNumberNodes("w"));
    nPos = max(nPos, component.getNumberNodes("woffset"));
    for (int iPos = 1; iPos <= nPos; ++iPos) {
      string index = (boost::format("[%1%]") % iPos).str();
      result.push_back(VXDGeoPlacementPar(
                         type,
                         component.getLength("u" + index, 0),
                         component.getLength("v" + index, 0),
                         component.getString("w" + index, "bottom"),
                         component.getLength("woffset" + index, 0)
                       ));
    }
  }
  return result;
}

const VXDGeoComponentPar& VXDGeometryPar::getComponent(std::string name) const
{
  //Check if component already exists
  std::map<string, VXDGeoComponentPar>::const_iterator cached = m_componentCache.find(name);
  if (cached == m_componentCache.end()) {
    B2FATAL("Could not find component " << name);
  }
  return cached->second;
}


int  VXDGeometryPar::getSensitiveChipID(std::string name) const
{
  //Check if sensorType already exists
  std::map<string, int>::const_iterator cached = m_sensitiveIDCache.find(name);
  if (cached == m_sensitiveIDCache.end()) {
    return -1;
  }
  return cached->second;
}


const VXDGeoSensorPar& VXDGeometryPar::getSensor(string sensorTypeID) const
{
  //Check if sensorType already exists
  std::map<string, VXDGeoSensorPar>::const_iterator cached = m_sensorMap.find(sensorTypeID);
  if (cached == m_sensorMap.end()) {
    B2FATAL("Invalid SensorTypeID " << sensorTypeID);
  }
  return cached->second;
}

const VXDGeoLadderPar& VXDGeometryPar::getLadder(int layer) const
{
  //Check if component already exists
  map<int, VXDGeoLadderPar>::const_iterator cached = m_ladders.find(layer);
  if (cached == m_ladders.end()) {
    B2FATAL("Could not find ladder for layer " << (boost::format("%1%") % layer).str());
  }
  return cached->second;
}

void VXDGeometryPar::cacheLadder(int layer, GearDir components)
{
  string path = (boost::format("Ladder[@layer=%1%]/") % layer).str();
  GearDir paramsLadder(components, path);
  if (!paramsLadder) {
    B2FATAL("Could not find Ladder definition for layer " << layer);
  }

  m_ladders[layer] = VXDGeoLadderPar(
                       layer,
                       paramsLadder.getLength("shift"),
                       paramsLadder.getLength("radius"),
                       paramsLadder.getAngle("slantedAngle", 0),
                       paramsLadder.getLength("slantedRadius", 0),
                       paramsLadder.getLength("Glue/oversize", 0),
                       paramsLadder.getString("Glue/Material", "")
                     );

  for (const GearDir& sensorInfo : paramsLadder.getNodes("Sensor")) {

    m_ladders[layer].addSensor(VXDGeoSensorPlacementPar(
                                 sensorInfo.getInt("@id"),
                                 sensorInfo.getString("@type"),
                                 sensorInfo.getLength("."),
                                 sensorInfo.getBool("@flipU", false),
                                 sensorInfo.getBool("@flipV", false),
                                 sensorInfo.getBool("@flipW", false)
                               ));
  }
}


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDGeometryPar::read(const string& prefix, const GearDir& content)
{
  //Read prefix ('SVD' or 'PXD')
  m_prefix = prefix;

  //Read some global parameters
  m_globals = VXDGlobalPar(content);

  //Read envelope parameters
  m_envelope = VXDEnvelopePar(GearDir(content, "Envelope/"));

  // Read alignment for detector m_prefix ('PXD' or 'SVD')
  string path = (boost::format("Align[@component='%1%']/") % m_prefix).str();
  GearDir params(GearDir(content, "Alignment/"), path);
  if (!params) {
    B2WARNING("Could not find alignment parameters for component " << m_prefix);
    return;
  }
  m_alignment[m_prefix] = VXDAlignmentPar(params.getLength("du"),
                                          params.getLength("dv"),
                                          params.getLength("dw"),
                                          params.getAngle("alpha"),
                                          params.getAngle("beta"),
                                          params.getAngle("gamma")
                                         );


  //Read the definition of all sensor types
  GearDir components(content, "Components/");
  for (const GearDir& paramsSensor : components.getNodes("Sensor")) {
    string sensorTypeID = paramsSensor.getString("@type");


    VXDGeoSensorPar sensor(
      paramsSensor.getString("Material"),
      paramsSensor.getString("Color", ""),
      paramsSensor.getLength("width"),
      paramsSensor.getLength("width2", 0),
      paramsSensor.getLength("length"),
      paramsSensor.getLength("height"),
      paramsSensor.getBool("@slanted", false)
    );
    sensor.setActive(VXDGeoComponentPar(
                       paramsSensor.getString("Material"),
                       paramsSensor.getString("Active/Color", "#f00"),
                       paramsSensor.getLength("Active/width"),
                       paramsSensor.getLength("Active/width2", 0),
                       paramsSensor.getLength("Active/length"),
                       paramsSensor.getLength("Active/height")
                     ), VXDGeoPlacementPar(
                       "Active",
                       paramsSensor.getLength("Active/u"),
                       paramsSensor.getLength("Active/v"),
                       paramsSensor.getString("Active/w", "center"),
                       paramsSensor.getLength("Active/woffset", 0)
                     ));
    sensor.setSensorInfo(createSensorInfo(GearDir(paramsSensor, "Active")));
    sensor.setComponents(getSubComponents(paramsSensor));
    m_sensorMap[sensorTypeID] = sensor;
  }

  //Build all ladders including Sensors
  GearDir support(content, "Support/");
  createHalfShellSupport(support);

  for (const GearDir& shell : content.getNodes("HalfShell")) {

    string shellName = m_prefix + "." + shell.getString("@name");

    path = (boost::format("Align[@component='%1%']/") % shellName).str();
    GearDir params(GearDir(content, "Alignment/"), path);
    if (!params) {
      B2WARNING("Could not find alignment parameters for component " << shellName);
      return;
    }
    m_alignment[shellName] = VXDAlignmentPar(params.getLength("du"),
                                             params.getLength("dv"),
                                             params.getLength("dw"),
                                             params.getAngle("alpha"),
                                             params.getAngle("beta"),
                                             params.getAngle("gamma")
                                            );



    VXDHalfShellPar halfShell(shell.getString("@name") , shell.getAngle("shellAngle", 0));

    for (const GearDir& layer : shell.getNodes("Layer")) {
      int layerID = layer.getInt("@id");

      cacheLadder(layerID, components);
      createLayerSupport(layerID, support);
      createLadderSupport(layerID, support);

      //Loop over defined ladders
      for (const GearDir& ladder : layer.getNodes("Ladder")) {
        int ladderID = ladder.getInt("@id");
        double phi = ladder.getAngle("phi", 0);
        readLadderInfo(layerID, ladderID, content);
        halfShell.addLadder(layerID, ladderID,  phi);
      }
    }
    m_halfShells.push_back(halfShell);
  }

  //Create diamond radiation sensors if defined and in background mode
  GearDir radiationDir(content, "RadiationSensors");
  if (m_globals.getActiveChips() &&  radiationDir) {
    m_radiationsensors = VXDGeoRadiationSensorsPar(
                           m_prefix,
                           radiationDir.getBool("insideEnvelope"),
                           radiationDir.getLength("width"),
                           radiationDir.getLength("length"),
                           radiationDir.getLength("height"),
                           radiationDir.getString("material")
                         );

    //Add radiation sensor positions
    for (GearDir& position : radiationDir.getNodes("position")) {
      VXDGeoRadiationSensorsPositionPar diamonds(position.getLength("z"),
                                                 position.getLength("radius"),
                                                 position.getAngle("theta")
                                                );

      //Loop over all phi positions
      for (GearDir& sensor : position.getNodes("phi")) {
        //Add sensor with angle and id
        diamonds.addSensor(sensor.getInt("@id"), sensor.getAngle());
      }
      m_radiationsensors.addPosition(diamonds);
    }
  }


  //Free some space
  //m_componentCache.clear();
  //FIXME: delete SensorInfo instances
  //m_sensorMap.clear();

}


void VXDGeometryPar::readLadderInfo(int layerID, int ladderID,  GearDir content)
{
  VxdID ladder(layerID, ladderID, 0);

  // Read alignment for ladder
  m_alignment[ladder] = VXDAlignmentPar(ladder, GearDir(content, "Alignment/"));

  for (const VXDGeoSensorPlacementPar& p : m_ladders[layerID].getSensors()) {
    VxdID sensorID(ladder);
    sensorID.setSensorNumber(p.getSensorID());

    std::map<string, VXDGeoSensorPar>::iterator it = m_sensorMap.find(p.getSensorTypeID());
    if (it == m_sensorMap.end()) {
      B2FATAL("Invalid SensorTypeID " << p.getSensorTypeID() << ", please check the definition of " << sensorID);
    }
    VXDGeoSensorPar& s = it->second;
    string name = m_prefix + "." + (string)sensorID;

    //Now create all the other components and place the Sensor
    if (!m_globals.getOnlyActiveMaterial()) cacheSubComponents(s.getComponents() , GearDir(content, "Components/"));
    m_alignment[sensorID] = VXDAlignmentPar(sensorID, GearDir(content, "Alignment/"));
  }
  return;
}


VXDAlignmentPar VXDGeometryPar::getAlignment(std::string name) const
{
  //Check if component already exists
  map<string, VXDAlignmentPar>::const_iterator cached = m_alignment.find(name);
  if (cached == m_alignment.end()) {
    B2FATAL("Could not find alignment parameters for component " << name);
  }
  return cached->second;
}

void VXDGeometryPar::cacheComponent(const std::string& name, GearDir componentsDir)
{
  //Check if component already exists
  map<string, VXDGeoComponentPar>::iterator cached = m_componentCache.find(name);
  if (cached != m_componentCache.end()) {
    return; // nothing to do
  }

  //Not cached, so lets create a new one
  string path = (boost::format("descendant::Component[@name='%1%']/") % name).str();
  GearDir params(componentsDir, path);
  if (!params) B2FATAL("Could not find definition for component " << name);

  VXDGeoComponentPar c(
    params.getString("Material",  m_globals.getDefaultMaterial()),
    params.getString("Color", ""),
    params.getLength("width", 0),
    params.getLength("width2", 0),
    params.getLength("length", 0),
    params.getLength("height", 0),
    params.getAngle("angle", 0)
  );

  if (c.getWidth() <= 0 || c.getLength() <= 0 || c.getHeight() <= 0) {
    B2DEBUG(100, "One dimension empty, using auto resize for component");
  }

  c.setSubComponents(getSubComponents(params));
  cacheSubComponents(c.getSubComponents(), componentsDir);

  if (m_globals.getActiveChips() && params.exists("activeChipID")) {
    int chipID = params.getInt("activeChipID");
    m_sensitiveIDCache[name] = chipID;
  }
  m_componentCache[name] = c;
}


void VXDGeometryPar::cacheSubComponents(const std::vector<VXDGeoPlacementPar>& placements , GearDir componentsDir)
{
  for (const VXDGeoPlacementPar& p : placements) {
    cacheComponent(p.getName(), componentsDir);
  }
  return;
}




