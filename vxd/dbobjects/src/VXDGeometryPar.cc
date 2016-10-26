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

void VXDGeometryPar::setCurrentLayer(int layer, GearDir components)
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

  //Read the definition of all sensor types
  GearDir components(content, "Components/");
  for (const GearDir& paramsSensor : components.getNodes("Sensor")) {
    string sensorTypeID = paramsSensor.getString("@type");
    B2INFO("Reading sensors: SensorTypeID: " << sensorTypeID);

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
    B2INFO("Building " << m_prefix << " half-shell " << shellName);

    //Full name of alignemnt component
    string component = m_prefix + "." + shellName;
    //Add component to alignment map
    m_alignment[component] = VXDAlignmentPar(component, GearDir(content, "Alignment/"));

    for (const GearDir& layer : shell.getNodes("Layer")) {
      int layerID = layer.getInt("@id");

      setCurrentLayer(layerID, components);
      createLayerSupport(layerID, support);
      createLadderSupport(layerID, support);
    }
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

  //Read the definition of all sensor types
  for (const GearDir& shell : content.getNodes("HalfShell")) {
    m_halfShells.push_back(VXDHalfShellPar(shell));
  }

  //Free some space
  //m_componentCache.clear();
  //FIXME: delete SensorInfo instances
  //m_sensorMap.clear();

}
