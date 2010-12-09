/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDLadder.h>

using namespace boost;
using namespace Belle2;

using namespace std;

B2GeomSVDLadder::B2GeomSVDLadder()
{
  volBarrelRibs = NULL;
  volSlantedRibs = NULL;
  volCoolingpipe = NULL;
}

Bool_t B2GeomSVDLadder::init(GearDir& ladderContent)
{
  //B2METHOD();
  // get parameters from XML file
  if (!initBasicParameters(ladderContent)) {
    B2FATAL("Could not initialize SVD ladder!");
    return false;
  }

  // Read parameters for ribs
  if (ladderContent.isParamAvailable("Barrel")) {
    GearDir barrelContent(ladderContent);
    barrelContent.append("Barrel/");
    initComponent<B2GeomSVDLadderRibs>(&volBarrelRibs, barrelContent, "Ribs");
  }

  if (ladderContent.isParamAvailable("Slanted")) {
    GearDir slantedContent(ladderContent);
    slantedContent.append("Slanted/");
    initComponent<B2GeomSVDLadderRibs>(&volSlantedRibs, slantedContent, "Ribs");
  }

  initComponent<B2GeomSVDLadderCoolingpipe>(&volCoolingpipe, ladderContent, "CoolingPipe");

// go to correct node of GearDir
  GearDir sensorContent(ladderContent);
  sensorContent.append("/Sensors/Sensor");
  nComponents = int(sensorContent.getNumberNodes());
  components = new B2GeomSVDSensor*[nComponents];
  for (int iComponent = 0; iComponent < nComponents; iComponent++) components[iComponent] = NULL;

  // initialise sensors
  for (iSensor = 0; iSensor < nComponents; iSensor++) {
    string relativePath((format("Sensors/Sensor[@id=\'SVD_Layer_%1%_Ladder_Sensor_%2%\']") % iLayer % iSensor).str());
    if (!initComponent<B2GeomSVDSensor>(&components[iSensor], ladderContent, relativePath)) {
      B2FATAL("Parameter reading for SVD sensor failed");
      return false;
    }
  }
  return true;
}

Bool_t B2GeomSVDLadder::make()
{
  //B2METHOD();
  // build SVD ladder from the parameters read in before
  if (!makeGeneric()) return false;

  // put SVD sensors
  for (iSensor = 0; iSensor < nComponents; ++iSensor) {
    if (!makeAndAddComponent(components[iSensor])) {
      B2FATAL("Cannot build SVD geometry without sensor!!!");
      return false;
    }
  }

  // put barrel ribs
  makeAndAddComponent(volBarrelRibs);

  // put slanted ribs
  makeAndAddComponent(volSlantedRibs);

  // put cooling pipe
  makeAndAddComponent(volCoolingpipe);
  return true;
}

// ------------------------------------------------------------------------------------------------
// 2 x 1Rib of SVD ladder
// ------------------------------------------------------------------------------------------------


B2GeomSVDLadderRibs::B2GeomSVDLadderRibs()
{
  volRib = NULL;
}

Bool_t B2GeomSVDLadderRibs::init(GearDir& ribsContent)
{
  // read basic parameters from XML
  if (!initBasicParameters(ribsContent)) return false;

  // get additionally the position of the ribs
  fRib1UPosition = ribsContent.getParamLength("Rib1UPosition");
  fRib2UPosition = ribsContent.getParamLength("Rib2UPosition");

  // initialize the volume for a rib (only one is needed, because it is cloned in the geometry)
  if (!initComponent<B2GeomSVDLadderRib>(&volRib, ribsContent, "Rib")) return false;
  return true;
}

Bool_t B2GeomSVDLadderRibs::make()
{
  if (!makeGeneric()) return false;
  if (!volRib->make()) return false;

  tVolume->AddNode(volRib->getVol(), 1, new TGeoTranslation(0.0, fRib1UPosition, 0.0));
  tVolume->AddNode(volRib->getVol(), 2, new TGeoTranslation(0.0, fRib2UPosition, 0.0));
  return true;
}


// ------------------------------------------------------------------------------------------------
// Rib of SVD ladder
// ------------------------------------------------------------------------------------------------

B2GeomSVDLadderRib::B2GeomSVDLadderRib()
{
  volFoam = NULL;
}

Bool_t B2GeomSVDLadderRib::init(GearDir& ribContent)
{
  // read parameters from XML file
  if (!initBasicParameters(ribContent)) return false;
  if (!initComponent<B2GeomVXDVolume>(&volFoam, ribContent, "Foam")) return false;
  return true;
}

Bool_t B2GeomSVDLadderRib::make()
{
  if (!makeGeneric()) return false;
  if (!makeAndAddComponent(volFoam)) return false;
  return true;
}

// ------------------------------------------------------------------------------------------------
// Cooling pipe of SVD ladder
// ------------------------------------------------------------------------------------------------

B2GeomSVDLadderCoolingpipe::B2GeomSVDLadderCoolingpipe()
{
  volLiquid = NULL;
}

Bool_t B2GeomSVDLadderCoolingpipe::init(GearDir& coolingpipeContent)
{
  // read basic parameters from XML
  if (!initBasicParameters(coolingpipeContent)) return false;
  initComponent<B2GeomVXDVolume>(&volLiquid, coolingpipeContent, "CoolingLiquid");
  return true;
}

Bool_t B2GeomSVDLadderCoolingpipe::make()
{
  if (!makeGeneric()) return false;
  if (!makeAndAddComponent(volLiquid)) return false;
  return true;
}
