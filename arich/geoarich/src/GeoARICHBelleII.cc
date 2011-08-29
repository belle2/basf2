/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/geoarich/GeoARICHBelleII.h>
#include <arich/geoarich/ARICHGeometryPar.h>
#include <arich/simarich/ARICHSensitiveDetector.h>
#include <arich/simarich/ARICHSensitiveAero.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <geometry/dataobjects/OpticalUserInfo.h>
#include <geometry/utilities/GeoReader.h>

#include <cmath>
#include <boost/format.hpp>

#include <TVector3.h>
#include <TVector2.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoPgon.h>
#include <TGeoTube.h>
#include <TGeoBBox.h>
#include <TGeoCompositeShape.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------
GeoARICHBelleII regGeoARICHBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoARICHBelleII::GeoARICHBelleII() : CreatorBase("ARICHBelleII")
{
  setDescription("Creates the TGeo objects for the aRICH geometry of the Belle II detector.");
  addSensitiveDetector("SD_", new ARICHSensitiveDetector("ARICHSensitiveDetector"));
  addSensitiveDetector("SA_", new ARICHSensitiveAero("ARICHSensitiveAero"));

}


GeoARICHBelleII::~GeoARICHBelleII()
{

}


void GeoARICHBelleII::create(GearDir& content)
{

  GearDir aerogel(content);
  aerogel.append("Aerogel/");
  double globalRotAngle = content.getParamAngle("Rotation") / Unit::deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");
  TGeoRotation* geoRot = new TGeoRotation("aRichRot", 0.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volGrpARICH = addSubdetectorGroup("ARICH", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));

  /*------------------------------------------------*
   *       Build aRICH container                    *
   *------------------------------------------------*
   *  aRICH container is a space available for      *
   *  aRICH detector. It is filled with air that    *
   *  has specified refractive index                *
   *  (to enable propagation of photons).           *
   *  Photons getting out of this volume are killed.*
   *________________________________________________*/

  // get aRICH container parameters
  double zFront = content.getParamLength("ContainerZfront");
  double zBack = content.getParamLength("ContainerZback");
  double zCenter = (zFront + zBack) / 2.;
  double contOutRadius = content.getParamLength("ContainerOuterRadius");
  double contInRadius = content.getParamLength("ContainerInnerRadius");
  string contMat = content.getParamString("ContainerMaterial");

  // create aRICH container tube
  TGeoMedium* contMed = gGeoManager->GetMedium(contMat.c_str());
  TGeoTube* contTubeSh = new TGeoTube("contTubeSh", contInRadius, contOutRadius, (zBack - zFront) / 2.);
  TGeoVolume* contTube = new TGeoVolume("contTube", contTubeSh, contMed);
  volGrpARICH->AddNode(contTube, 1, new TGeoTranslation(0.0, 0.0, zCenter));

  /*-------------------------------------------*
   *       Build aerogel planes                *
   *-------------------------------------------*
   *  N aerogel layers of "wedge" tiles are    *
   *  placed. There is an aluminum support     *
   *  plate in front of the first layer.       *
   *___________________________________________*/

  // get aerogel parameters
  ARICHGeometryPar* arichgp = ARICHGeometryPar::Instance();
  int nLayer = arichgp->getNumberOfAerogelRadiators();
  double TileXSize = aerogel.getParamLength("TileXSize");
  double TileYSize = aerogel.getParamLength("TileYSize");
  double TileGap = aerogel.getParamLength("TileGap");
  double TubeInnerRadius  = arichgp->getAerogelTubeInnerRadius();
  double TubeOuterRadius  = arichgp->getAerogelTubeOuterRadius();

  // this is the material that will fill the gaps between tiles
  string TubeMat = aerogel.getParamString("TubeMaterial");
  TGeoMedium* TubeMed = gGeoManager->GetMedium(TubeMat.c_str());

  // building the aerogel support plate
  string SuppMat = aerogel.getParamString("SupportPlate/Material");
  double SuppThick = aerogel.getParamLength("SupportPlate/Thickness");
  TGeoMedium* SuppMed = gGeoManager->GetMedium(SuppMat.c_str());
  TGeoTube* suppTubeSh = new TGeoTube("suppTubeSh", TubeInnerRadius, TubeOuterRadius, SuppThick / 2.);
  TGeoVolume* suppTube = new TGeoVolume("suppTube", suppTubeSh, SuppMed);

  // creating nLayer of aerogel planes
  for (int iLayer = 1; iLayer <= nLayer; iLayer++) {
    // get material name from gearbox and then from TGeoManager
    GearDir layerContent(content);
    layerContent.append((format("Aerogel/Layers/Layer[%1%]/") % (iLayer)).str());
    string Material = layerContent.getParamString("Material");
    TGeoMedium* AerogelMed = gGeoManager->GetMedium(Material.c_str());
    AerogelMed->GetMaterial()->SetTransparency(50);
    double thickness = arichgp->getAerogelThickness(iLayer - 1);
    double zPosition = arichgp->getAerogelZPosition(iLayer - 1);

    TGeoTube* tube = new TGeoTube("tube", TubeInnerRadius, TubeOuterRadius, thickness / 2.);
    TGeoVolume* aeroTube = new TGeoVolume((format("aeroTube_%1%") % iLayer).str().c_str(), tube, TubeMed);
    contTube->AddNode(aeroTube, iLayer, new TGeoTranslation(0.0, 0.0, zPosition + thickness / 2.  - zCenter));

    // placing the aerogel support plate
    if (iLayer == 1) contTube->AddNode(suppTube, 1, new TGeoTranslation(0.0, 0.0, zPosition - SuppThick / 2.  - zCenter));

    // build and place aerogel tiles

    // number of tiles in radial direction
    int nRTiles = int((TubeOuterRadius - TubeInnerRadius) / (TileXSize + TileGap)) + 1;
    // size of the tile in radial direction
    double rSize = (TubeOuterRadius - TubeInnerRadius) / nRTiles - TileGap;
    for (int iRad = 0; iRad < nRTiles; iRad++) {
      double rout = TubeInnerRadius + TileGap / 2. + rSize + iRad * (rSize + TileGap);
      double circ = 2.*M_PI * rout;
      int nPhiTiles = int(circ / (TileYSize + TileGap)) + 1;
      double dphi = 2.*M_PI / double(nPhiTiles)  / Unit::deg;
      TGeoTubeSeg* tileShape = new TGeoTubeSeg("tileShape", rout - rSize, rout, thickness / 2., -dphi / 2. + TileGap / 2. / rout  / Unit::deg , dphi / 2. - TileGap / 2. / rout / Unit::deg);
      string name = (format("AeroTile_%1%") % iRad).str();
      if (iLayer == 1) name = (format("SA_AeroTile_%1%") % iRad).str();
      TGeoVolume* tile = new TGeoVolume(name.c_str(), tileShape, AerogelMed);
      tile->SetLineColor(7);
      for (int iPhi = 0; iPhi < nPhiTiles; iPhi++) {
        double phi = dphi / 2. + iPhi * dphi;
        aeroTube->AddNode(tile, iPhi, new TGeoRotation("tileRot", phi, 0, 0));
      }
    }
  }

  /*-------------------------------------------*
   *       Build detector module               *
   *-------------------------------------------*
   *                                           *
   *    ______________  ___window              *
   *   | |==========| | ___sensitive surface   *
   *   | |          | | ___ceramic wall        *
   *   |_|__________|_|                        *
   *         \_                                *
   *           reflective bottom               *
   *___________________________________________*/


  // get detector module parameters
  GearDir Detector(content);
  Detector.append("Detector/");
  GearDir detModule(Detector);
  detModule.append("Module/");
  string wallMat =  detModule.getParamString("wallMaterial");
  string winMat =  detModule.getParamString("winMaterial");
  string sensMat =  detModule.getParamString("sensMaterial");
  string botMat =  detModule.getParamString("botMaterial");
  double modXsize = arichgp->getDetectorModuleSize();
  double modZsize = arichgp->getDetectorThickness();
  double wallThick =  detModule.getParamLength("ModuleWallThickness");
  double winThick = arichgp->getDetectorWindowThickness();
  double sensXsize = arichgp->getSensitiveSurfaceSize();
  double sensThick = detModule.getParamLength("SensThickness");
  double botThick =  detModule.getParamLength("BottomThickness");

  // get material
  TGeoMedium* wallMed = gGeoManager->GetMedium(wallMat.c_str());
  TGeoMedium* winMed = gGeoManager->GetMedium(winMat.c_str());
  TGeoMedium* sensMed = gGeoManager->GetMedium(sensMat.c_str());
  TGeoMedium* botMed = gGeoManager->GetMedium(botMat.c_str());
  TGeoMaterial* winMaterial = winMed->GetMaterial();

  winMaterial->SetTransparency(50);
  TGeoVolumeAssembly* volGrpDetector = new TGeoVolumeAssembly("DetectorPlane");
  contTube->AddNode(volGrpDetector, 1, new TGeoTranslation(0.0, 0.0, 0.0));
  TGeoVolumeAssembly* volGrpModule = new TGeoVolumeAssembly("DetectorModule");


  // creating and placing detector wall
  TGeoPgon* wallShape = new TGeoPgon("wallShape", 0., 360, 4, 2);
  wallShape->DefineSection(0, -modZsize / 2., modXsize / 2. - wallThick, modXsize / 2.);
  wallShape->DefineSection(1, modZsize / 2., modXsize / 2. - wallThick, modXsize / 2.);
  TGeoVolume* detWall = new TGeoVolume("detWall", wallShape, wallMed);
  detWall->SetLineColor(2);
  volGrpModule->AddNode(detWall, 1, new TGeoRotation("wallRot", 45, 0.0, 0.0));
  // creating ang placing detector window
  TGeoBBox* winShape = new TGeoBBox("winShape", modXsize / 2. - wallThick, modXsize / 2. - wallThick, winThick / 2.);
  TGeoVolume* detWin = new TGeoVolume("detWin", winShape, winMed);
  detWin->SetLineColor(38);
  volGrpModule->AddNode(detWin, 1, new TGeoTranslation(0.0, 0.0, (-modZsize + winThick) / 2.));
  // creating and placing sensitive surface
  TGeoBBox* sensShape = new TGeoBBox("sensShape", sensXsize / 2., sensXsize / 2., sensThick / 2);
  TGeoVolume* detSens = new TGeoVolume("SD_detSens", sensShape, sensMed);

  detSens->SetLineColor(29);
  volGrpModule->AddNode(detSens, 1, new TGeoTranslation(0.0, 0.0, -modZsize / 2. + winThick + sensThick / 2.));

  // creating and placing bottom reflective surface
  TGeoBBox* bottomShape = new TGeoBBox("sensShape", modXsize / 2. - wallThick, modXsize / 2. - wallThick, botThick / 2.);
  TGeoVolume* detBot = new TGeoVolume("detBottom", bottomShape, botMed);
  detBot->SetLineColor(8);
  volGrpModule->AddNode(detBot, 1, new TGeoTranslation(0.0, 0.0, modZsize / 2. - botThick / 2.));

  // get modules placement parameters
  double zPosition = arichgp->getDetectorZPosition();
  double tubeRout = arichgp->getDetectorOuterRadius();
  double tubeRin = arichgp->getDetectorInnerRadius();

  // preparing tube for support plate (holes are then placed as daughter volumes filled with air)
  string detSuppMat = Detector.getParamString("Plane/SupportPlate/Material");
  double detSuppThick = Detector.getParamLength("Plane/SupportPlate/Thickness");
  double detSuppHoleX = Detector.getParamLength("Plane/SupportPlate/HoleXSize");
  double detSuppHoleY = Detector.getParamLength("Plane/SupportPlate/HoleYSize");
  TGeoTube* detSuppTubeSh = new TGeoTube("detSuppTubeSh", tubeRin, tubeRout, detSuppThick / 2.);
  TGeoMedium* detSuppMed = gGeoManager->GetMedium(detSuppMat.c_str());
  TGeoMedium* holeMed = gGeoManager->GetMedium("Air");
  TGeoVolume* detSuppTube = new TGeoVolume("detSuppTube", detSuppTubeSh, detSuppMed);
  TGeoBBox* holeSh = new TGeoBBox("holeSh", detSuppHoleX / 2. , detSuppHoleY / 2., detSuppThick / 2.);
  TGeoVolume* hole = new TGeoVolume("hole", holeSh, holeMed);

  // placing modules
  // modules positions are in ARICHGeometryPar class
  int nMod = arichgp->getNMCopies();
  for (int iMod = 0; iMod < nMod; ++iMod) {
    TVector3 modPos = arichgp->getOrigin(iMod);
    double modAngle = arichgp->getModAngle(iMod) / Unit::deg;
    TGeoRotation* modRot = new TGeoRotation((format("modRot_%1%") % (iMod)).str().c_str());
    modRot->RotateZ(modAngle);
    volGrpDetector->AddNode(volGrpModule, iMod, new TGeoCombiTrans(modPos.X(), modPos.Y(), modPos.Z() - zCenter, modRot));
    detSuppTube->AddNode(hole, iMod, new TGeoCombiTrans(modPos.X(), modPos.Y(), 0.0, modRot));
  }
  volGrpDetector->AddNode(detSuppTube, 1, new TGeoTranslation(0.0, 0.0, zPosition + modZsize + detSuppThick / 2. - zCenter));

  /*-------------------------------------------*
   *       Build mirrors                       *
   *-------------------------------------------*
   *  Mirrors are placed as a regular polygon, *
   *  on the outer edge of detector tube.      *
   *  Filling the space between aerogel and    *
   *  detector plane.                          *
   *___________________________________________*/


  // get mirrors parameters
  GearDir MirrorsCont(content);
  GearDir Mirror(content);
  MirrorsCont.append("Mirrors/");
  Mirror.append("Mirrors");
  OpticalUserInfo* surfaceInfo = GeoReader::readOpticalSurface(Mirror);
  string mirrMat =  MirrorsCont.getParamString("Material");
  int nMirrors = arichgp->getNMirrors();
  double outRadius = arichgp->getMirrorsOuterRadius();
  double Thick = arichgp->getMirrorsThickness();
  double Length = arichgp->getMirrorsLength();
  double zPos =  arichgp->getMirrorsZPosition();

  // creating and placing mirrors volume
  double inRadius = outRadius * cos(M_PI / nMirrors) - Thick;
  TGeoMedium* mirrMed = gGeoManager->GetMedium(mirrMat.c_str());
  TGeoPgon* ngon = new TGeoPgon("mirrors_shape", 0., 360, nMirrors, 2);
  ngon->DefineSection(0, -Length / 2., inRadius, inRadius + Thick);
  ngon->DefineSection(1, Length / 2., inRadius, inRadius + Thick);
  TGeoVolume* Mirrors  = new TGeoVolume("Mirrors", ngon, mirrMed);
  Mirrors->SetField(surfaceInfo);
  Mirrors->SetLineColor(15);
  TGeoRotation* mirRot = new TGeoRotation("mirrorRot");
  double mirAngle = arichgp->getMirrorsStartAngle() / Unit::deg;
  mirRot->RotateZ(mirAngle);
  contTube->AddNode(Mirrors, 1, new TGeoCombiTrans(0.0, 0.0, zPos + Length / 2.  - zCenter, mirRot));
}






