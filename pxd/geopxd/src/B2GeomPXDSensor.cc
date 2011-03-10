/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomPXDSensor.h>
#include <pxd/dataobjects/PXDVolumeUserInfo.h>

using namespace boost;
using namespace Belle2;

using namespace std;

B2GeomPXDSensor::B2GeomPXDSensor()
{
  volSilicon = NULL;
  volSwitchers1 = NULL;
  volSwitchers2 = NULL;
}

Bool_t B2GeomPXDSensor::init(GearDir& sensorContent)
{
  ////B2METHOD();
  if (!initBasicParameters(sensorContent)) {
    B2FATAL("Could not initialize basic parameters of PXD sensor");
    return false;
  }

  // Read parameters for Silicon part
  if (!initComponent<B2GeomPXDSensorSilicon>(&volSilicon, sensorContent, "Silicon")) {
    B2FATAL("Parameter reading for PXD silicon failed!\n");
    return false;
  }

  // Read parameters for Switchers
  if (!initComponent<B2GeomVXDVolume>(&volSwitchers1, sensorContent, "Switchers1")) B2WARNING("No switchters created!");
  if (!initComponent<B2GeomVXDVolume>(&volSwitchers2, sensorContent, "Switchers2"))  B2WARNING("No switchters created!");

  // Read offsets from XML file
  sensorContent.setDirPath((format("//Offsets/Sensor[@id=\'PXD_Offset_Layer_%1%_Ladder_%2%_Sensor_%3%\']/") % iLayer % iLadder % iSensor).str());
  if (!initOffsets(sensorContent)) {
    B2WARNING("PXD sensor offsets not initialized!");
  }
  return true;
}


Bool_t B2GeomPXDSensor::make()
{
  ////B2METHOD();

  // Build the container for the sensor components
  if (!makeGeneric()) {
    B2FATAL("Creating PXD Sensor failed!");
    return false;
  }

  // Build the silicon of the PXD sensor
  if (!makeAndAddComponent(volSilicon)) {
    B2FATAL("Cannot create silicon of PXD sensor!\n");
    return false;
  }

  makeAndAddComponent(volSwitchers1);

  makeAndAddComponent(volSwitchers2);

  return true;
}

// ------------------------------------------------------------------------------------------------
// Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

B2GeomPXDSensorSilicon::B2GeomPXDSensorSilicon()
{
  volActive = NULL;
  volThinned = NULL;
}

Bool_t B2GeomPXDSensorSilicon::init(GearDir& content)
{
  // initialize parameters from XML
  if (!initBasicParameters(content)) return false;


  initComponent<B2GeomVXDVolume>(&volThinned, content, "Thinned");

  if (!initComponent<B2GeomPXDSensorActive>(&volActive, content, "Active")) {
    B2FATAL("Parameter reading for SVD silicon active failed!\n");
    return false;
  }
  return true;
}

Bool_t B2GeomPXDSensorSilicon::make()
{
  if (!makeGeneric()) {
    B2FATAL("Cannot build silicon of PXD sensor");
    return false;
  }

  // add thinned out volume with air
  makeAndAddComponent(volThinned);

  // add active volume to the silicon
  if (!makeAndAddComponent(volActive)) {
    B2FATAL("Cannot build active volume of PXD sensor!");
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// Active Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

Bool_t B2GeomPXDSensorActive::init(GearDir& content)
{
  if (!initBasicParameters(content)) {
    B2FATAL("Could not read parameters for PXD sensitive volume!!!!")
    return false;
  }

  // check if the sensor or the ladder is mirrored!
  // then the active sensor has also to be mirrored in order to keep the right coordinates
  while (true) {
    if (!goToParentNode(content)) break;
    if (content.isParamAvailable("ReflectX")) isReflectX = !isReflectX;
    if (content.isParamAvailable("ReflectY")) isReflectY = !isReflectY;
    if (content.isParamAvailable("ReflectZ")) isReflectZ = !isReflectZ;
  }
  return true;
}

Bool_t B2GeomPXDSensorActive::make()
{
  Bool_t res = makeGeneric();
  if (res)
    tVolume->SetField(new PXDVolumeUserInfo(iLayer, iLadder, iSensor));
  return res;
}








