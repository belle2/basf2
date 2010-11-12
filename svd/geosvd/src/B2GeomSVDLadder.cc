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
  B2GeomSVDLadder(-1, -1);
}

B2GeomSVDLadder::B2GeomSVDLadder(Int_t iLay, Int_t iLad)
{
  iLayer = iLay;
  iLadder = iLad;
  sprintf(name, "SVD_Layer_%i_Ladder_%i" , iLayer, iLadder);
  resetBasicParameters();
  volBarrelRibs = NULL;
  volSlantedRibs = NULL;
  volCoolingpipe = NULL;
}

B2GeomSVDLadder::~B2GeomSVDLadder()
{
}

Bool_t B2GeomSVDLadder::init(GearDir& content)
{
  //printf("SVDLadder::init start (Lay: %i, Lad: %i)\n", iLayer, iLadder);
  ladderContent = GearDir(content);
  ladderContent.append("Ladders/");

  initBasicParameters(ladderContent);

  // Read parameters for ribs
  if (ladderContent.isParamAvailable("Barrel")) {
    GearDir barrelContent(ladderContent);
    barrelContent.append("Barrel/");
    volBarrelRibs = new B2GeomSVDLadderRibs(iLayer);
    if (!volBarrelRibs->init(barrelContent)) {
      printf("ERROR! Parameter reading for SVD barrel ribs failed!\n");
      return false;
    }
  }
  if (ladderContent.isParamAvailable("Slanted")) {
    GearDir slantedContent(ladderContent);
    slantedContent.append("Slanted/");
    volSlantedRibs = new B2GeomSVDLadderRibs(iLayer);
    if (!volSlantedRibs->init(slantedContent)) {
      printf("ERROR! Parameter reading for SVD slanted ribs failed!\n");
      return false;
    }
  }

  if (ladderContent.isParamAvailable("CoolingPipe")) {
    volCoolingpipe = new B2GeomSVDLadderCoolingpipe(iLayer);
    if (!volCoolingpipe->init(ladderContent)) {
      printf("ERROR! Parameter reading for SVD cooling pipe failed!\n");
      return false;
    }
  }

  // get parameters for the sensors
  GearDir sensorsContent(ladderContent);
  sensorsContent.append("/Sensors/Sensor");
  nComponents = int(sensorsContent.getNumberNodes());

  components = new B2GeomVolume*[nComponents];
  for (int iSensor = 0; iSensor < nComponents; iSensor++) {
    components[iSensor] = new B2GeomSVDSensor(iLayer, iLadder, iSensor);
    if (!components[iSensor]->init(ladderContent)) {
      printf("ERROR! Parameter reading for SVD sensor failed\n");
      return false;
    }
  }
  return true;
  //printf("SVDLadder::init stop\n");
}

Bool_t B2GeomSVDLadder::make()
{
  //printf("SVDLadder::make (Lay: %i, Lad: %i) \n", iLayer, iLadder);
  // create container for SVD ladder
  tVolume = new TGeoVolumeAssembly(name);

  // put SVD sensors
  for (int iSensor = 0; iSensor < nComponents; ++iSensor) {
    components[iSensor]->make();
    // place sensor in the ladder
    tVolume->AddNode(components[iSensor]->getVol(), 1, components[iSensor]->getPosition());
  }

  // put barrel ribs
  if (volBarrelRibs != NULL) {
    volBarrelRibs->make();
    tVolume->AddNode(volBarrelRibs->getVol(), 1, volBarrelRibs->getPosition());
  }

  // put slanted ribs

  // put cooling pipe
  if (volCoolingpipe != NULL) {
    volCoolingpipe->make();
    tVolume->AddNode(volCoolingpipe->getVol(), 1, volCoolingpipe->getPosition());
  }
  //printf("ladder made\n");
  return true;

}

// ------------------------------------------------------------------------------------------------
// 2 x 1Rib of SVD ladder
// ------------------------------------------------------------------------------------------------


B2GeomSVDLadderRibs::B2GeomSVDLadderRibs(Int_t iLay)
{
  iLayer = iLay;
  resetBasicParameters();
  volRib = NULL;
}

Bool_t B2GeomSVDLadderRibs::init(GearDir& content)
{
  //printf("SVDLadderRibs::init start\n");
  GearDir ribsContent(content);
  ribsContent.append("Ribs/");
  initBasicParameters(ribsContent);
  fRib1UPosition = ribsContent.getParamLength("Rib1UPosition");
  fRib2UPosition = ribsContent.getParamLength("Rib2UPosition");
  if (ribsContent.isParamAvailable("Rib")) {
    volRib = new B2GeomSVDLadderRib(iLayer);
    if (!volRib->init(ribsContent)) {
      printf("ERROR! Parameter reading for SVD Rib failed!\n");
      return false;
    }
  }
  //printf("SVDLadderRibs::init stop\n");
  return true;
}

Bool_t B2GeomSVDLadderRibs::make()
{
  if (!volRib->make()) {
    printf("ERROR! Cannot create TGeoVolume for SVD rib ladder!\n");
    return false;
  }

  char nameRibs[200];
  sprintf(nameRibs, "SVD_Layer_%i_Ladder_Ribs_Barrel", iLayer);

  tVolume = (TGeoVolume*) gROOT->FindObjectAny(nameRibs);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTrd2(nameRibs, tMedium,
                                    0.5 * fThickness,
                                    0.5 * fThickness2,
                                    0.5 * fWidth,
                                    0.5 * fWidth2,
                                    0.5 * fLength);

  }

  tVolume->AddNode(volRib->getVol(), 1, new TGeoTranslation(0.0, fRib1UPosition, 0.0));
  tVolume->AddNode(volRib->getVol(), 2, new TGeoTranslation(0.0, fRib2UPosition, 0.0));
  return true;
}


// ------------------------------------------------------------------------------------------------
// Rib of SVD ladder
// ------------------------------------------------------------------------------------------------

B2GeomSVDLadderRib::B2GeomSVDLadderRib(Int_t iLay)
{
  resetBasicParameters();
  iLayer = iLay;
  volFoam = NULL;
}
Bool_t B2GeomSVDLadderRib::init(GearDir& content)
{
  //printf("SVDLadderRib::init start\n");
  GearDir ribContent(content);
  ribContent.append("Rib/");
  initBasicParameters(ribContent);
  sprintf(name, "SVD_Layer_%i_Ladder_Ribs_Barrel_Rib", iLayer);
  if (ribContent.isParamAvailable("Foam")) {
    volFoam = new B2GeomSVDLadderRibFoam(iLayer);
    if (!volFoam->init(ribContent)) {
      printf("ERROR! Parameter reading for SVD RibFoam failed!\n");
      return false;
    }
  }
  //printf("SVDLadderRib::init stop\n");
  return true;
}

Bool_t B2GeomSVDLadderRib::make()
{

  tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                  0.5 * fThickness,
                                  0.5 * fThickness2,
                                  0.5 * fWidth,
                                  0.5 * fWidth2,
                                  0.5 * fLength
                                 );
  tVolume->SetLineColor(kBlack);
  if (volFoam != NULL) {
    volFoam->make();
    tVolume->AddNode(volFoam->getVol(), 1, volFoam->getPosition());
  }
  return true;
}

// ------------------------------------------------------------------------------------------------
// Foam of Rib of SVD ladder
// ------------------------------------------------------------------------------------------------

B2GeomSVDLadderRibFoam::B2GeomSVDLadderRibFoam(Int_t iLay)
{
  iLayer = iLay;
  resetBasicParameters();
}

Bool_t B2GeomSVDLadderRibFoam::init(GearDir& content)
{
  GearDir ribfoamContent(content);
  ribfoamContent.append("Foam/");
  initBasicParameters(ribfoamContent);
  sprintf(name, "SVD_Layer_%i_Ladder_Ribs_Barrel_Rib_Foam", iLayer);
  return true;
}


Bool_t B2GeomSVDLadderRibFoam::make()
{
  tVolume = gGeoManager->MakeTrd2(name, tMedium,
                                  0.5 * fThickness,
                                  0.5 * fThickness2,
                                  0.5 * fWidth,
                                  0.5 * fWidth2,
                                  0.5 * fLength);
  tVolume->SetLineColor(kYellow - 9);
  return true;
}

// ------------------------------------------------------------------------------------------------
// Cooling pipe of SVD ladder
// ------------------------------------------------------------------------------------------------


B2GeomSVDLadderCoolingliquid::B2GeomSVDLadderCoolingliquid(Int_t iLay)
{
  iLayer = iLay;
  resetBasicParameters();
}
Bool_t B2GeomSVDLadderCoolingliquid::init(GearDir& content)
{
  GearDir coolingliquidContent(content);
  coolingliquidContent.append("CoolingLiquid/");
  initBasicParameters(coolingliquidContent);
  sprintf(name, "SVD_Layer_%i_Cool_Liquid", iLayer);

  return true;
}

Bool_t B2GeomSVDLadderCoolingliquid::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTube(name, tMedium,
                                    fInnerRadius,
                                    fOuterRadius,
                                    0.5 * fLength
                                   );
    tVolume->SetLineColor(kCyan - 5);
  }
  return true;
}


B2GeomSVDLadderCoolingpipe::B2GeomSVDLadderCoolingpipe(Int_t iLay)
{
  iLayer = iLay;
  resetBasicParameters();
  volLiquid = NULL;
}
Bool_t B2GeomSVDLadderCoolingpipe::init(GearDir& content)
{
  GearDir coolingpipeContent(content);
  coolingpipeContent.append("CoolingPipe/");
  initBasicParameters(coolingpipeContent);
  sprintf(name, "SVD_Layer_%i_Cool_Pipe", iLayer);
  if (coolingpipeContent.isParamAvailable("CoolingLiquid")) {
    volLiquid = new B2GeomSVDLadderCoolingliquid(iLayer);
    if (!volLiquid->init(coolingpipeContent)) {
      printf("ERROR! Parameter reading for SVD Cooling Liquid failed!\n");
      return false;
    }
  }
  return true;
}

Bool_t B2GeomSVDLadderCoolingpipe::make()
{
  tVolume = (TGeoVolume*) gROOT->FindObjectAny(name);
  if (!tVolume) {
    tVolume = gGeoManager->MakeTube(name, tMedium,
                                    fInnerRadius,
                                    fOuterRadius,
                                    0.5 * fLength
                                   );
    tVolume->SetLineColor(kCyan - 5);
  }
  if (volLiquid != NULL) {
    volLiquid->make();
    tVolume->AddNode(volLiquid->getVol(), 1, volLiquid->getPosition());
  }
  return true;

}





