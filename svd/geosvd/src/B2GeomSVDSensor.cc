/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDSensor.h>
#include <svd/dataobjects/SVDVolumeUserInfo.h>

using namespace boost;
using namespace Belle2;
using namespace std;

B2GeomSVDSensor::B2GeomSVDSensor()
{
  volSilicon = NULL;
  volFoam = NULL;
  volSMDs = NULL;
  volKapton = NULL;
  volKaptonFlex = NULL;
  volKaptonFront = NULL;
}

Bool_t B2GeomSVDSensor::init(GearDir& sensorContent)
{
  //B2METHOD();
  //Read sensor data
  // initialize basic parameters from XML file
  if (!initBasicParameters(sensorContent)) {
    B2FATAL("Could not initialize SVD sensor!");
    return false;
  }

  if (!initComponent<B2GeomSVDSensorSilicon>(&volSilicon, sensorContent, "Silicon")) {
    B2FATAL("Parameter reading for SVD silicon failed!");
    return false;
  }

  // read parameters for rohacell foam pad
  initComponent<B2GeomVXDVolume>(&volFoam, sensorContent, "Foam");

  initComponent<B2GeomVXDVolume>(&volKapton, sensorContent, "Kapton");
  initComponent<B2GeomVXDVolume>(&volKaptonFlex, sensorContent, "KaptonFlex");
  initComponent<B2GeomVXDVolume>(&volKaptonFront, sensorContent, "KaptonFront");
  initComponent<B2GeomVXDVolume>(&volSMDs, sensorContent, "SMDs");

  sensorContent.setDirPath((format("//Offsets/Sensor[@id=\'SVD_Offset_Layer_%1%_Ladder_%2%_Sensor_%3%\']/") % iLayer % iLadder % iSensor).str());
  if (!initOffsets(sensorContent)) {
    B2WARNING("SVD sensor offsets not initialized!");
  }
  return true;

}

Bool_t B2GeomSVDSensor::make()
{
  //B2METHOD();
  // create container for SVD sensor components
  if (!makeGeneric()) {
    B2FATAL("Could not build SVD sensor");
    return false;
  }


  // add silicon to container
  if (!makeAndAddComponent(volSilicon)) {
    B2FATAL("Could not build the SVD silicon");
    return false;
  }

  // add the rohacell foam pad
  makeAndAddComponent(volFoam);

  // add the kapton layer
  makeAndAddComponent(volKaptonFront);


  // add the kapton layer
  makeAndAddComponent(volKapton);

  // add the kapton layer
  makeAndAddComponent(volKaptonFlex);

// add chips
  makeAndAddComponent(volSMDs);
  return true;
}

// ------------------------------------------------------------------------------------------------
// Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

B2GeomSVDSensorSilicon::B2GeomSVDSensorSilicon()
{
  volActive = NULL;
}

Bool_t B2GeomSVDSensorSilicon::init(GearDir& siliconContent)
{
  //B2METHOD();
  // initialize basic parameters from XML file
  if (!initBasicParameters(siliconContent)) {
    B2FATAL("Could not initialize silicon of SVD sensor!");
    return false;
  }

  // initialize the contained active volume
  // Read parameters for active sensor
  if (!initComponent<B2GeomSVDSensorActive>(&volActive, siliconContent, "Active")) {
    B2FATAL("Parameter reading for SVD active silicon failed!");
    return false;
  }
  return true;
}

Bool_t B2GeomSVDSensorSilicon::make()
{
  // build geomtry of SVD sensor silicon
  if (!makeGeneric()) {
    B2FATAL("Could not build silicon of SVD sensor");
    return false;
  }

  // build volume of active silicon
  if (!makeAndAddComponent(volActive)) {
    B2FATAL("Cannot build TGeoVolume for SVD active silicon!");
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// Active Silicon part of the sensor
// ------------------------------------------------------------------------------------------------

Bool_t B2GeomSVDSensorActive::init(GearDir& content)
{
  if (!initBasicParameters(content)) {
    B2FATAL("Could not read parameters for PXD sensitive volume!!!!")
    return false;
  }

  // Read user parameters
  if (content.isParamAvailable("UPitch")) {
    m_uPitch = content.getParamLength("UPitch");
  }
  if (content.isParamAvailable("UPitch2")) {
    m_uPitch2 = content.getParamLength("UPitch2");
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
  return true;
}

Bool_t B2GeomSVDSensorActive::make()
{
  Bool_t res = makeGeneric();
  if (res)
    tVolume->SetField(new SVDVolumeUserInfo(iLayer, iLadder, iSensor, m_uPitch, m_uPitch2, m_uCells, m_vPitch, m_vCells));
  return res;
}
