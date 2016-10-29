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

      VXDGeoPlacementPar aPlacement;
      aPlacement.setName(type);
      aPlacement.setU(component.getLength("u" + index, 0));
      aPlacement.setV(component.getLength("v" + index, 0));
      aPlacement.setW(component.getString("w" + index, "bottom"));
      aPlacement.setWOffset(component.getLength("woffset" + index, 0));

      result.push_back(aPlacement);
    }
  }
  return result;
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
                       paramsLadder.getLength("shift") / Unit::mm,
                       paramsLadder.getLength("radius") / Unit::mm,
                       paramsLadder.getAngle("slantedAngle", 0),
                       paramsLadder.getLength("slantedRadius", 0) / Unit::mm,
                       paramsLadder.getLength("Glue/oversize", 0) / Unit::mm,
                       paramsLadder.getString("Glue/Material", "")
                     );

  for (const GearDir& sensorInfo : paramsLadder.getNodes("Sensor")) {

    m_ladders[layer].addSensor(VXDGeoSensorPlacementPar(
                                 sensorInfo.getInt("@id"),
                                 sensorInfo.getString("@type"),
                                 sensorInfo.getLength(".") / Unit::mm,
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
  m_alignment[m_prefix] = VXDAlignmentPar(m_prefix, GearDir(content, "Alignment/"));

  //Read the definition of all sensor types
  GearDir components(content, "Components/");
  for (const GearDir& paramsSensor : components.getNodes("Sensor")) {
    string sensorTypeID = paramsSensor.getString("@type");
    //B2INFO("Reading sensors: SensorTypeID: " << sensorTypeID);

    // Create a new sensor from GearBox
    VXDGeoSensorPar sensor;
    sensor.setSlanted(paramsSensor.getBool("@slanted", false));
    sensor.setMaterial(paramsSensor.getString("Material"));
    sensor.setColor(paramsSensor.getString("Color", ""));
    sensor.setWidth(paramsSensor.getLength("width") / Unit::mm);
    sensor.setWidth2(paramsSensor.getLength("width2", 0) / Unit::mm);
    sensor.setLength(paramsSensor.getLength("length") / Unit::mm);
    sensor.setHeight(paramsSensor.getLength("height") / Unit::mm);

    // Create a new component for active volume (from where charge is collected)
    VXDGeoComponentPar activeArea;
    activeArea.setMaterial(paramsSensor.getString("Material"));
    activeArea.setColor(paramsSensor.getString("Active/Color", "#f00"));
    activeArea.setWidth(paramsSensor.getLength("Active/width") / Unit::mm);
    activeArea.setWidth2(paramsSensor.getLength("Active/width2", 0) / Unit::mm);
    activeArea.setLength(paramsSensor.getLength("Active/length") / Unit::mm);
    activeArea.setHeight(paramsSensor.getLength("Active/height") / Unit::mm);

    // Define where to place active volume
    VXDGeoPlacementPar activePlacement;
    activePlacement.setName("Active");
    activePlacement.setU(paramsSensor.getLength("Active/u") / Unit::mm);
    activePlacement.setV(paramsSensor.getLength("Active/v") / Unit::mm);
    activePlacement.setW(paramsSensor.getString("Active/w", "center"));
    activePlacement.setWOffset(paramsSensor.getLength("Active/woffset", 0) / Unit::mm);

    sensor.setActive(activeArea, activePlacement);
    sensor.setSensorInfo(createSensorInfo(GearDir(paramsSensor, "Active")));
    sensor.setComponents(getSubComponents(paramsSensor));
    m_sensorMap[sensorTypeID] = sensor;
  }

  //Build all ladders including Sensors
  GearDir support(content, "Support/");
  createHalfShellSupport(support);



  for (const GearDir& shell : content.getNodes("HalfShell")) {

    string shellName =  shell.getString("@name");
    //B2INFO("Building " << m_prefix << " half-shell " << shellName);

    //Full name of alignemnt component
    string component = m_prefix + "." + shellName;
    m_alignment[component] = VXDAlignmentPar(component, GearDir(content, "Alignment/"));

    for (const GearDir& layer : shell.getNodes("Layer")) {
      int layerID = layer.getInt("@id");

      cacheLadder(layerID, components);
      createLayerSupport(layerID, support);
      createLadderSupport(layerID, support);

      //Loop over defined ladders
      for (const GearDir& ladder : layer.getNodes("Ladder")) {
        int ladderID = ladder.getInt("@id");
        //double phi = ladder.getAngle("phi", 0);

        readLadderInfo(layerID, ladderID, content);
      }
    }
  }


  //Read the definition of all sensor types
  for (const GearDir& shell : content.getNodes("HalfShell")) {
    m_halfShells.push_back(VXDHalfShellPar(shell));
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
        //We need angle and Id
        diamonds.addPhi(sensor.getAngle());
        diamonds.addId(sensor.getInt("@id"));
      }
      m_radiationsensors.addSensor(diamonds);
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



void VXDGeometryPar::cacheComponent(const std::string& name, GearDir componentsDir)
{

  //B2INFO("Caching Component " << name);

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
  //B2INFO("Creating Sub Components for Component " << m_prefix + "." + name);
  vector<VXDGeoPlacementPar> subComponents = getSubComponents(params);
  cacheSubComponents(subComponents, componentsDir);

  if (m_globals.getActiveChips() && params.exists("activeChipID")) {
    int chipID = params.getInt("activeChipID");
    //B2INFO("For component " << name << " reading active chipID " <<  chipID);
    m_sensitiveIDCache[name] = chipID;
  }
  m_componentCache[name] = c;
}


void VXDGeometryPar::cacheSubComponents(std::vector<VXDGeoPlacementPar> placements , GearDir componentsDir)
{
  for (VXDGeoPlacementPar& p : placements) {
    cacheComponent(p.getName(), componentsDir);
  }
  return;
}




