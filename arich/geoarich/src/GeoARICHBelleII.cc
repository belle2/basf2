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

  double globalRotAngle = (180.0 / M_PI) * content.getParamAngle("Rotation");
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
  double zCenter = (zFront + zBack) / 2. * mm ;
  double contRadius = content.getParamLength("ContainerRadius");
  string contMat = content.getParamString("ContainerMaterial");

  // create aRICH container tube
  TGeoMedium* contMed = gGeoManager->GetMedium(contMat.c_str());
  TGeoTube* contTubeSh = new TGeoTube("contTubeSh", 0.0, contRadius, (zBack - zFront) / 2.);
  TGeoVolume* contTube = new TGeoVolume("contTube", contTubeSh, contMed);
  volGrpARICH->AddNode(contTube, 1, new TGeoTranslation(0.0, 0.0, zCenter));

  /*-------------------------------------------*
   *       Build aerogel planes                *
   *-------------------------------------------*
   *      __                                   *
   *   __/  \__    N aerogel planes are placed.*
   *  /  \__/  \   Composed of hexagon tiles.  *
   *  \__/  \__/   On the edges of aRICH the   *
   *  /  \__/  \   tiles are cut to fit in     *
   *  \__/  \__/   aerogel tube.               *
   *     \__/                                  *
   *___________________________________________*/


  // get aerogel parameters
  ARICHGeometryPar* arichgp = ARICHGeometryPar::Instance();
  int nLayer = arichgp->GetNumberOfAerogelRadiators();
  double TileSize = arichgp->GetAerogelTileSize();
  double TileGap = arichgp->GetAerogelTileGap();
  double TubeInnerRadius  = arichgp->GetAerogelTubeInnerRadius();
  double TubeOuterRadius  = arichgp->GetAerogelTubeOuterRadius();
  double TileFrameSize = TileSize + TileGap / 2.;
  double TFSdiag = TileFrameSize * 2. / sqrt(3.);

  // this is the material that will fill the gaps between tiles
  string TubeMat = aerogel.getParamString("TubeMaterial");
  TGeoMedium* TubeMed = gGeoManager->GetMedium(TubeMat.c_str());

  // building the aerogel support plate
  string SuppMat = aerogel.getParamString("SupportPlate/Material");
  double SuppThick = aerogel.getParamLength("SupportPlate/Thickness");
  TGeoMedium* SuppMed = gGeoManager->GetMedium(SuppMat.c_str());
  TGeoTube* suppTubeSh = new TGeoTube("suppTubeSh", TubeInnerRadius, TubeOuterRadius, SuppThick / 2.);
  TGeoVolume* suppTube = new TGeoVolume("suppTube", suppTubeSh, SuppMed);

  int i = 0;
  TGeoPgon* hexagon = new TGeoPgon("hexagon", 0., 360, 6, 2);
  // creating nLayer of aerogel planes
  for (int iLayer = 1; iLayer <= nLayer; iLayer++) {
    // get material name from gearbox and then from TGeoManager
    GearDir layerContent(content);
    layerContent.append((format("Aerogel/Layers/Layer[%1%]/") % (iLayer)).str());
    string Material = layerContent.getParamString("Material");
    TGeoMedium* AerogelMed = gGeoManager->GetMedium(Material.c_str());
    AerogelMed->GetMaterial()->SetTransparency(50);
    double thickness = arichgp->GetAerogelThickness(iLayer - 1);
    double zPosition = arichgp->GetAerogelZPosition(iLayer - 1);

    TGeoTube* tube = new TGeoTube("tube", TubeInnerRadius, TubeOuterRadius, thickness / 2.);
    TGeoVolume* aeroTube = new TGeoVolume((format("aeroTube_%1%") % iLayer).str().c_str(), tube, TubeMed);
    contTube->AddNode(aeroTube, iLayer, new TGeoTranslation(0.0, 0.0, zPosition + thickness / 2.  - zCenter));

    // placing the aerogel support plate
    if (iLayer == 1) contTube->AddNode(suppTube, 1, new TGeoTranslation(0.0, 0.0, zPosition - SuppThick / 2.  - zCenter));

    // aerogel tiles on the edges are build as a intersection between hexagon ant this tubee. It has to be thicker then hexagons otherwise root has drawing problems.
    TGeoTube* tubee = new TGeoTube("tubee", TubeInnerRadius, TubeOuterRadius, thickness / 2. + 1*mm);

    hexagon->DefineSection(0, -thickness / 2., 0., TileSize);
    hexagon->DefineSection(1, thickness / 2., 0., TileSize);

    int nTile = arichgp->GetNACopies();

    // filling the aerogel tube with hexagon tiles, their positions are in ARICHGeometryPar
    for (int iTile = 0; iTile < nTile; ++iTile) {
      TVector2 tilePos = arichgp->GetTilePos(iTile);
      string name = (format("AeroTile_%1%") % i).str();
      if (iLayer == 1) name = (format("SA_AeroTile_%1%") % i).str();
      // if tile intersects aerogel tube it is cut to fit in
      if (tilePos.Mod() > TubeOuterRadius - TFSdiag || tilePos.Mod() < TubeInnerRadius + TFSdiag) {
        TGeoTranslation* m1 = new TGeoTranslation((format("AeroTrans_%1%") % i).str().c_str(), -tilePos.X(), -tilePos.Y() , 0.0);
        m1->RegisterYourself();
        TGeoCompositeShape* cut = new TGeoCompositeShape((format("AeroShape_%1%") % i).str().c_str(), (format("hexagon*tubee:AeroTrans_%1%") % i).str().c_str());
        TGeoVolume* cutHexagon = new TGeoVolume(name.c_str(), cut, AerogelMed);
        cutHexagon->SetLineColor(7);
        aeroTube->AddNode(cutHexagon, i, new TGeoTranslation("Trans", tilePos.X(), tilePos.Y() , 0.0));
      }
      // else whole hexagon is inserted
      else {
        TGeoVolume* Hexagon = new TGeoVolume(name.c_str(), hexagon, AerogelMed);
        Hexagon->SetLineColor(7);
        aeroTube->AddNode(Hexagon, i, new TGeoTranslation("Trans", tilePos.X(), tilePos.Y(), 0.0));
      }
      i++;
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
  double modXsize = arichgp->GetDetectorModuleSize();
  double modZsize = arichgp->GetDetectorThickness();
  double wallThick =  detModule.getParamLength("ModuleWallThickness");
  double winThick = arichgp->GetDetectorWindowThickness();
  double sensXsize = arichgp->GetSensitiveSurfaceSize();
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
  double zPosition = arichgp->GetDetectorZPosition();
  double tubeRout = arichgp->GetDetectorOuterRadius();
  double tubeRin = arichgp->GetDetectorInnerRadius();

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
  int nMod = arichgp->GetNMCopies();
  for (int iMod = 0; iMod < nMod; ++iMod) {
    TVector3 modPos = arichgp->GetOrigin(iMod);
    double modAngle = arichgp->GetModAngle(iMod);
    TGeoRotation* modRot = new TGeoRotation((format("modRot_%1%") % (iMod)).str().c_str());
    modRot->RotateZ(180. / M_PI*modAngle);
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
  MirrorsCont.append("Mirrors/");
  string mirrMat =  MirrorsCont.getParamString("Material");
  int nMirrors = arichgp->GetNMirrors();
  double outRadius = arichgp->GetMirrorsOuterRadius();
  double Thick = arichgp->GetMirrorsThickness();
  double Length = arichgp->GetMirrorsLength();
  double zPos =  arichgp->GetMirrorsZPosition();

  // creating and placing mirrors volume
  double inRadius = outRadius * cos(M_PI / nMirrors) - Thick;
  TGeoMedium* mirrMed = gGeoManager->GetMedium(mirrMat.c_str());
  TGeoPgon* ngon = new TGeoPgon("mirrors_shape", 0., 360, nMirrors, 2);
  ngon->DefineSection(0, -Length / 2., inRadius, inRadius + Thick);
  ngon->DefineSection(1, Length / 2, inRadius, inRadius + Thick);
  TGeoVolume* Mirrors  = new TGeoVolume("Mirrors", ngon, mirrMed);
  Mirrors->SetLineColor(15);
  contTube->AddNode(Mirrors, 1, new TGeoTranslation(0.0, 0.0, zPos + Length / 2  - zCenter));

}






