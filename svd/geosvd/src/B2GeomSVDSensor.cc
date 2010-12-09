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
  if (!initComponent<B2GeomVXDVolume>(&volActive, siliconContent, "Active")) {
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
















