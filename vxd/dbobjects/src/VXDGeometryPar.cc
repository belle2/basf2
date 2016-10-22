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


// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDGeometryPar::read(const GearDir& content)
{

  //Read some global parameters
  m_globals = VXDGlobalPar(content);

  //Read alignment parameters
  m_alignment = VXDAlignmentPar(GearDir(content, "Alignment/"));

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
    //G4Transform3D shellAlignment = getAlignment(m_prefix + "." + shellName);

    for (const GearDir& layer : shell.getNodes("Layer")) {
      int layerID = layer.getInt("@id");
      //setCurrentLayer(layerID);
      //Place Layer support
      createLayerSupport(layerID, support);
      createLadderSupport(layerID, support);
    }
  }

  //Create diamond radiation sensors if defined and in background mode
  //GearDir radiationDir(content, "RadiationSensors");
  //if (m_activeChips && radiationDir) {
  //m_radiationsensors.create(radiationDir, topVolume, *envelope);
  //}

  //Free some space
  //m_componentCache.clear();
  //FIXME: delete SensorInfo instances
  //m_sensorMap.clear();


  //Read the definition of all sensor types
  for (const GearDir& shell : content.getNodes("HalfShell")) {
    m_halfShells.push_back(VXDHalfShellPar(shell));
  }

}
