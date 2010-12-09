/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomPXDLadder.h>

using namespace Belle2;
using namespace boost;

using namespace std;

Bool_t B2GeomPXDLadder::init(GearDir& content)
{
  ////B2METHOD();
  // read basic parameters from XML file
  if (!initBasicParameters(content)) {
    B2FATAL("Could not read parameters for PXD ladder from XML file");
    return false;
  }

  // get number of sensors
  GearDir sensorsContent(content);
  sensorsContent.append("/Sensors/Sensor");
  nComponents = int(sensorsContent.getNumberNodes());
  components = new B2GeomPXDSensor*[nComponents];
  for (int iComponent = 0; iComponent < nComponents; iComponent++) components[iComponent] = NULL;

  // initialize sensors
  for (iSensor = 0; iSensor < nComponents; iSensor++) {
    string relativePath = (format("Sensors/Sensor[@id=\'PXD_Layer_%1%_Ladder_Sensor_%2%\']") % iLayer % iSensor).str();
    if (!initComponent<B2GeomPXDSensor>(&components[iSensor], content, relativePath)) {
      B2FATAL("Could not initialize PXD sensor!");
      return false;
    }
  }
  return true;
}


Bool_t B2GeomPXDLadder::make()
{
  if (!makeGeneric()) {
    B2FATAL("Creating PXD Ladder failed!\n");
    return false;
  }
  for (iSensor = 0; iSensor < nComponents; ++iSensor) {
    if (!makeAndAddComponent(components[iSensor])) {
      B2FATAL("Could not build PXD sensor!")
      return false;
    }
  }
  return true;
}




