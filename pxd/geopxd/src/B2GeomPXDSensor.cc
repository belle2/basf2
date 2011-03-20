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
  if (!initComponent<B2GeomPXDSensorSwitchers1>(&volSwitchers1, sensorContent, "Switchers1"))  B2WARNING("No switchters created!");
  if (!initComponent<B2GeomPXDSensorSwitchers2>(&volSwitchers2, sensorContent, "Switchers2"))  B2WARNING("No switchters created!");

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

//--------------------------------------------------------------------------------
//Switchers1 part - DCDs and DHPs
//--------------------------------------------------------------------------------

B2GeomPXDSensorSwitchers1::B2GeomPXDSensorSwitchers1()
{
  volDCD1 = NULL;
  volDCD2 = NULL;
  volDCD3 = NULL;
  volDCD4 = NULL;
  volDCD5 = NULL;
  volDCD6 = NULL;

  volDHP1 = NULL;
  volDHP2 = NULL;
  volDHP3 = NULL;
  volDHP4 = NULL;
  volDHP5 = NULL;
  volDHP6 = NULL;
}

Bool_t B2GeomPXDSensorSwitchers1::init(GearDir& sensorContent)
{
  ////B2METHOD();
  if (!initBasicParameters(sensorContent)) {
    B2FATAL("Could not initialize basic parameters of PXD sensor");
    return false;
  }

  // Read parameters for DCDs
  if (!initComponent<B2GeomVXDVolume>(&volDCD1, sensorContent, "DCD1"))  B2WARNING("No DCD1 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDCD2, sensorContent, "DCD2"))  B2WARNING("No DCD2 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDCD3, sensorContent, "DCD3"))  B2WARNING("No DCD3 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDCD4, sensorContent, "DCD4"))  B2WARNING("No DCD4 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDCD5, sensorContent, "DCD5"))  B2WARNING("No DCD5 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDCD6, sensorContent, "DCD6"))  B2WARNING("No DCD6 created!");

  // Read parameters for DHPs
  if (!initComponent<B2GeomVXDVolume>(&volDHP1, sensorContent, "DHP1"))  B2WARNING("No DHP1 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDHP2, sensorContent, "DHP2"))  B2WARNING("No DHP2 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDHP3, sensorContent, "DHP3"))  B2WARNING("No DHP3 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDHP4, sensorContent, "DHP4"))  B2WARNING("No DHP4 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDHP5, sensorContent, "DHP5"))  B2WARNING("No DHP5 created!");
  if (!initComponent<B2GeomVXDVolume>(&volDHP6, sensorContent, "DHP6"))  B2WARNING("No DHP6 created!");


  return true;
}


Bool_t B2GeomPXDSensorSwitchers1::make()
{
  ////B2METHOD();

  // Build the container for the sensor components
  if (!makeGeneric()) {
    B2FATAL("Creating PXD Sensor failed!");
    return false;
  }

  //for(int iComponent = 0; iComponent != 6; ++iComponent) makeAndAddComponent(volDCD[iComponent]);
  makeAndAddComponent(volDCD1);
  makeAndAddComponent(volDCD2);
  makeAndAddComponent(volDCD3);
  makeAndAddComponent(volDCD4);
  makeAndAddComponent(volDCD5);
  makeAndAddComponent(volDCD6);

  makeAndAddComponent(volDHP1);
  makeAndAddComponent(volDHP2);
  makeAndAddComponent(volDHP3);
  makeAndAddComponent(volDHP4);
  makeAndAddComponent(volDHP5);
  makeAndAddComponent(volDHP6);

  return true;
}

//--------------------------------------------------------------------------------
//Switchers2 part - Switchers
//--------------------------------------------------------------------------------

B2GeomPXDSensorSwitchers2::B2GeomPXDSensorSwitchers2()
{
  volSwitcher1 = NULL;
  volSwitcher2 = NULL;
  volSwitcher3 = NULL;
  volSwitcher4 = NULL;
  volSwitcher5 = NULL;
  volSwitcher6 = NULL;
}

Bool_t B2GeomPXDSensorSwitchers2::init(GearDir& sensorContent)
{
  ////B2METHOD();
  if (!initBasicParameters(sensorContent)) {
    B2FATAL("Could not initialize basic parameters of PXD sensor");
    return false;
  }

  // read parameters for switchers
  if (!initComponent<B2GeomVXDVolume>(&volSwitcher1, sensorContent, "Switcher1"))  B2WARNING("No Switcher1 created!");
  if (!initComponent<B2GeomVXDVolume>(&volSwitcher2, sensorContent, "Switcher2"))  B2WARNING("No Switcher2 created!");
  if (!initComponent<B2GeomVXDVolume>(&volSwitcher3, sensorContent, "Switcher3"))  B2WARNING("No Switcher3 created!");
  if (!initComponent<B2GeomVXDVolume>(&volSwitcher4, sensorContent, "Switcher4"))  B2WARNING("No Switcher4 created!");
  if (!initComponent<B2GeomVXDVolume>(&volSwitcher5, sensorContent, "Switcher5"))  B2WARNING("No Switcher5 created!");
  if (!initComponent<B2GeomVXDVolume>(&volSwitcher6, sensorContent, "Switcher6"))  B2WARNING("No Switcher6 created!");

  return true;
}

Bool_t B2GeomPXDSensorSwitchers2::make()
{
  ////B2METHOD();

  // Build the container for the sensor components
  if (!makeGeneric()) {
    B2FATAL("Creating PXD Sensor failed!");
    return false;
  }

  makeAndAddComponent(volSwitcher1);
  makeAndAddComponent(volSwitcher2);
  makeAndAddComponent(volSwitcher3);
  makeAndAddComponent(volSwitcher4);
  makeAndAddComponent(volSwitcher5);
  makeAndAddComponent(volSwitcher6);

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

  // Read user parameters
  if (content.isParamAvailable("UPitch")) {
    m_uPitch = content.getParamLength("UPitch");
  }
  if (content.isParamAvailable("UCells")) {
    m_uCells = atoi(content.getParamString("UCells").c_str());
  }
  if (content.isParamAvailable("VPitch")) {
    m_vPitch = content.getParamLength("VPitch");
  }
  if (content.isParamAvailable("VCells")) {
    m_vCells = atoi(content.getParamString("VCells").c_str());
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
    tVolume->SetField(new PXDVolumeUserInfo(iLayer, iLadder, iSensor, m_uPitch, m_uCells, m_vPitch, m_vCells));
  return res;
}
