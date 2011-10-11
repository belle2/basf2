/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <eklm/geoeklm/G4TriangularPrism.h>
#include <eklm/simeklm/EKLMSensitiveDetector.h>

#include <eklm/geoeklm/StructureEndcap.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/format.hpp>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4SubtractionSolid.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4ReflectedSolid.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4IntersectionSolid.hh>

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <errno.h>

using namespace std;
using namespace boost;
using namespace Belle2;

/* Register the creator */
//GeoEKLMBelleII regGeoEKLMBelleII;
geometry::CreatorFactory<GeoEKLMBelleII> GeoEKLMFactory("EKLMBelleII");

/*
 * GeoEKLMBelleII - GeoEKLMBelleII constructor
 */
GeoEKLMBelleII::GeoEKLMBelleII()
{
  m_sensitive = new EKLMSensitiveDetector("EKLMSensitiveDetector", (2*24)*eV,
                                          10*MeV);
  if (m_sensitive == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  SectorSupportSize.CornerAngle = -1;
}

/*
 * ~GeoEKLMBelleII - GeoEKLMBelleII destructor
 */
GeoEKLMBelleII::~GeoEKLMBelleII()
{
  delete m_sensitive;
}

/*
 * createMaterials - creation of materials
 */
void GeoEKLMBelleII::createMaterials()
{
  Air = geometry::Materials::get("Air");
  Polystyrene = geometry::Materials::get("Polystyrene");
  Iron = geometry::Materials::get("Iron");
  Aluminium = geometry::Materials::get("Aluminium");
}

/*
 * readPositionData - read position data from XML database
 */
void GeoEKLMBelleII::readPositionData(struct EKLMElementPosition& epos,
                                      GearDir& content)
{
  epos.innerR = content.getLength("InnerR") * cm;
  epos.outerR = content.getLength("OuterR") * cm;
  epos.length = content.getLength("Length") * cm;
  epos.X = content.getLength("PositionX") * cm;
  epos.Y = content.getLength("PositionY") * cm;
  epos.Z = content.getLength("PositionZ") * cm;
}

/*
 * readXMLData - read parameters from XML database
 * @content: GearDir to read from
 */
void GeoEKLMBelleII::readXMLData(const GearDir& content)
{
  int i;
  int j;
  GearDir gd(content);
  m_mode = gd.getInt("Mode");
  GearDir EndCap(gd);
  EndCap.append("/EndCap");
  readPositionData(EndcapPosition, EndCap);
  EndcapPosition.Z = EndcapPosition.Z + EndcapPosition.length / 2.0;
  nLayer = EndCap.getInt("nLayer");
  GearDir Layer(EndCap);
  Layer.append("/Layer");
  readPositionData(LayerPosition, Layer);
  Layer_shiftZ = Layer.getLength("ShiftZ") * cm;
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readPositionData(SectorPosition, Sector);
  nPlane = Sector.getInt("nPlane");
  nBoard = Sector.getInt("nBoard");
  GearDir Boards(Sector);
  Boards.append("/Boards");
  BoardSize.length = Boards.getLength("Length") * cm;
  BoardSize.width = Boards.getLength("Width") * cm;
  BoardSize.height = Boards.getLength("Height") * cm;
  BoardSize.base_width = Boards.getLength("BaseWidth") * cm;
  BoardSize.base_height = Boards.getLength("BaseHeight") * cm;
  BoardSize.strip_length = Boards.getLength("StripLength") * cm;
  BoardSize.strip_width = Boards.getLength("StripWidth") * cm;
  BoardSize.strip_height = Boards.getLength("StripHeight") * cm;
  nStripBoard = Boards.getInt("nStripBoard");
  for (j = 0; j < nPlane; j++) {
    BoardPosition[j] = new struct EKLMBoardPosition[nBoard];
    if (BoardPosition[j] == NULL) {
      B2FATAL("Memory allocation error.");
      exit(ENOMEM);
    }
    for (i = 0; i < nBoard; i++) {
      GearDir BoardContent(Boards);
      BoardContent.append((format("/BoardData[%1%]") % (j + 1)).str());
      BoardContent.append((format("/Board[%1%]") % (i + 1)).str());
      BoardPosition[j][i].phi = BoardContent.getLength("Phi") * rad;
      BoardPosition[j][i].r = BoardContent.getLength("Radius") * cm;
    }
  }
  StripBoardPosition = new EKLMStripBoardPosition[nStripBoard];
  if (StripBoardPosition == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  for (i = 0; i < nStripBoard; i++) {
    GearDir StripBoardContent(Boards);
    StripBoardContent.append((format("/StripBoardData/Board[%1%]") %
                              (i + 1)).str());
    StripBoardPosition[i].x = StripBoardContent.getLength("PositionX") * cm;
  }
  GearDir SectorSupport(Sector);
  SectorSupport.append("/SectorSupport");
  readPositionData(SectorSupportPosition, SectorSupport);
  SectorSupportSize.Thickness = SectorSupport.getLength("Thickness") * cm;
  SectorSupportSize.DeltaLY = SectorSupport.getLength("DeltaLY") * cm;
  SectorSupportSize.CornerX = SectorSupport.getLength("CornerX") * cm;
  SectorSupportSize.TopCornerHeight = SectorSupport.
                                      getLength("TopCornerHeight") * cm;
  SectorSupportSize.Corner1LX = SectorSupport.getLength("Corner1LX") * cm;
  SectorSupportSize.Corner1Width = SectorSupport.getLength("Corner1Width") * cm;
  SectorSupportSize.Corner1Thickness = SectorSupport.
                                       getLength("Corner1Thickness") * cm;
  SectorSupportSize.Corner1Z = SectorSupport.getLength("Corner1Z") * cm;
  SectorSupportSize.Corner2LX = SectorSupport.getLength("Corner2LX") * cm;
  SectorSupportSize.Corner2LY = SectorSupport.getLength("Corner2LY") * cm;
  SectorSupportSize.Corner2Thickness = SectorSupport.
                                       getLength("Corner2Thickness") * cm;
  SectorSupportSize.Corner2Z = SectorSupport.getLength("Corner2Z") * cm;
  SectorSupportSize.Corner3LX = SectorSupport.getLength("Corner3LX") * cm;
  SectorSupportSize.Corner3LY = SectorSupport.getLength("Corner3LY") * cm;
  SectorSupportSize.Corner3Thickness = SectorSupport.
                                       getLength("Corner3Thickness") * cm;
  SectorSupportSize.Corner3Z = SectorSupport.getLength("Corner3Z") * cm;
  SectorSupportSize.Corner4LX = SectorSupport.getLength("Corner4LX") * cm;
  SectorSupportSize.Corner4LY = SectorSupport.getLength("Corner4LY") * cm;
  SectorSupportSize.Corner4Thickness = SectorSupport.
                                       getLength("Corner4Thickness") * cm;
  SectorSupportSize.Corner4Z = SectorSupport.getLength("Corner4Z") * cm;
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(PlanePosition, Plane);
  nSection = Plane.getInt("nSection");
  PlasticListWidth = Plane.getLength("PlasticListWidth") * cm;
  PlasticListDeltaL = Plane.getLength("PlasticListDeltaL") * cm;
  GearDir Strips(Plane);
  Strips.append("/Strips");
  nStrip = Strips.getNumberNodes("Strip");
  Strip_width  = Strips.getLength("Width") * cm;
  Strip_thickness = Strips.getLength("Thickness") * cm;
  StripPosition = new struct EKLMElementPosition[nStrip];
  if (StripPosition == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  for (i = 0; i < nStrip; i++) {
    GearDir StripContent(Strips);
    StripContent.append((format("/Strip[%1%]") % (i + 1)).str());
    StripPosition[i].length = StripContent.getLength("Length") * cm;
    StripPosition[i].X = StripContent.getLength("PositionX") * cm;
    StripPosition[i].Y = StripContent.getLength("PositionY") * cm;
    StripPosition[i].Z = StripContent.getLength("PositionZ") * cm;
  }
  GearDir Sections(Plane);
  Sections.append("/Sections");
  SectionSupportTopWidth = Sections.getLength("TopWidth") * cm;
  SectionSupportTopThickness = Sections.getLength("TopThickness") * cm;
  SectionSupportMiddleWidth = Sections.getLength("MiddleWidth") * cm;
  SectionSupportMiddleThickness = Sections.getLength("MiddleThickness") * cm;
  for (j = 0; j < nPlane; j++) {
    SectionSupportPosition[j] = new struct
          EKLMSectionSupportPosition[nSection + 1];
    if (SectionSupportPosition[j] == NULL) {
      B2FATAL("Memory allocation error.");
      exit(ENOMEM);
    }
    for (i = 0; i <= nSection; i++) {
      GearDir SectionSupportContent(Sections);
      SectionSupportContent.append((format("/SectionSupportData[%1%]") % (j +
                                    1)).str());
      SectionSupportContent.append((format("/SectionSupport[%1%]") % (i +
                                    1)).str());
      SectionSupportPosition[j][i].length = SectionSupportContent.
                                            getLength("Length") * cm;
      SectionSupportPosition[j][i].x = SectionSupportContent.
                                       getLength("PositionX") * cm;
      SectionSupportPosition[j][i].y = SectionSupportContent.
                                       getLength("PositionY") * cm;
      SectionSupportPosition[j][i].z = SectionSupportContent.
                                       getLength("PositionZ") * cm;
      SectionSupportPosition[j][i].deltal_right = SectionSupportContent.
                                                  getLength("DeltaLRight") *
                                                  cm;
      SectionSupportPosition[j][i].deltal_left = SectionSupportContent.
                                                 getLength("DeltaLLeft") * cm;
    }
  }
}

/**
 * ReflectCoordinates - get reflected coordinates
 */
void GeoEKLMBelleII::ReflectCoordinates(double xin, double yin, double *xout,
                                        double *yout, int quarter)
{
  switch (quarter) {
    case 1:
      *xout = xin;
      *yout = yin;
      break;
    case 2:
      *xout = -xin;
      *yout = yin;
      break;
    case 3:
      *xout = -xin;
      *yout = -yin;
      break;
    case 4:
      *xout = xin;
      *yout = -yin;
      break;
  }
}

/*
 * createEndcap - create endcap
 * @iEndcap: number of endcap
 * @mlv: mother logical volume
 */
void GeoEKLMBelleII::createEndcap(int iEndcap, G4LogicalVolume *mlv)
{
  int i;
  double z;
  double phi;
  double dphi;
  int nsides;
  int nBoundary;
  double zsub;
  double rminsub;
  double rmaxsub;
  G4Polyhedra *boct;
  G4Tubs *atube;
  G4SubtractionSolid *solidEndcap;
  G4LogicalVolume *logicEndcap;
  G4PVPlacementGT *physiEndcap;
  G4Transform3D t;
  StructureEndcap EndcapMgr;
  EndcapMgr.read();
  phi = EndcapMgr.phi();
  dphi = EndcapMgr.dphi();
  nsides = EndcapMgr.nsides();
  nBoundary = EndcapMgr.nBoundary();
  double struct_z[nBoundary];
  double rmin[nBoundary];
  double rmax[nBoundary];
  for (int izBoundary  = 0; izBoundary < nBoundary; izBoundary++) {
    struct_z[izBoundary] = EndcapMgr.z(izBoundary);
    rmin[izBoundary] = EndcapMgr.rmin(izBoundary);
    rmax[izBoundary] = EndcapMgr.rmax(izBoundary);
  }
  //m_matnamesub = EndcapMgr.matnamesub();
  zsub = EndcapMgr.zsub();
  rminsub = EndcapMgr.rminsub();
  rmaxsub = EndcapMgr.rmaxsub();
  std::string Endcap_Name = "Endcap_" + lexical_cast<string>(iEndcap);
  boct = new G4Polyhedra("tempoct", phi, dphi, nsides, nBoundary, struct_z,
                         rmin, rmax);
  atube = new G4Tubs("tempatube", rminsub, rmaxsub, zsub, 0.0, 360.0 * deg);
  solidEndcap = new G4SubtractionSolid(Endcap_Name, boct, atube);
  if (solidEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (iEndcap == 1)
    z = -EndcapPosition.Z + 94.0 * cm;
  else
    z = EndcapPosition.Z;
  t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y, z);
  logicEndcap = new G4LogicalVolume(solidEndcap, Iron, Endcap_Name);
  if (logicEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicEndcap, true);
  geometry::setColor(*logicEndcap, "#ffffff22");
  physiEndcap = new G4PVPlacementGT(t, t, logicEndcap, Endcap_Name, mlv,
                                    iEndcap);
  if (physiEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  for (i = 1; i <= nLayer; i++)
    createLayer(i, iEndcap, physiEndcap);
}

/*
 * createLayer - create layer
 * @iLayer: number of layer
 * @iEndcap: number of endcap
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createLayer(int iLayer, int iEndcap,
                                 G4PVPlacementGT *mpvgt)
{
  int i;
  G4Tubs *solidLayer;
  G4LogicalVolume *logicLayer;
  G4PVPlacementGT *physiLayer;
  G4Transform3D t;
  std::string Layer_Name = "Layer_" + lexical_cast<string>(iLayer) + "_" +
                           mpvgt->GetName();
  solidLayer = new G4Tubs(Layer_Name, LayerPosition.innerR,
                          LayerPosition.outerR, LayerPosition.length / 2.0,
                          0. * deg, 360. * deg);
  if (solidLayer == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicLayer = new G4LogicalVolume(solidLayer, Air, Layer_Name);
  if (logicLayer == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicLayer, false);
  LayerPosition.Z = -EndcapPosition.length / 2.0 + iLayer * Layer_shiftZ +
                    0.5 * LayerPosition.length;
  if (iEndcap == 1)
    LayerPosition.Z = -LayerPosition.Z;
  t = G4Translate3D(0.0, 0.0, LayerPosition.Z);
  physiLayer = new G4PVPlacementGT(mpvgt, t, logicLayer, Layer_Name, iLayer);
  if (physiLayer == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  for (i = 1; i <= 4; i++)
    createSector(i, physiLayer);
}

/*
 * createSector - create sector
 * @iSector: number of sector
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSector(int iSector, G4PVPlacementGT *mpvgt)
{
  int i;
  int j;
  G4Tubs *solidSector;
  G4LogicalVolume *logicSector;
  G4PVPlacementGT *physiSector;
  G4Transform3D t;
  std::string Sector_Name = "Sector_" + lexical_cast<string>(iSector) + "_" +
                            mpvgt->GetName();
  solidSector = new G4Tubs(Sector_Name, SectorPosition.innerR,
                           SectorPosition.outerR, 0.5 * SectorPosition.length,
                           0.0, 90.0 * deg);
  if (solidSector == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicSector = new G4LogicalVolume(solidSector, Air, Sector_Name);
  if (logicSector == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicSector, false);
  switch (iSector) {
    case 1:
      t = G4Translate3D(0., 0., 0.);
      break;
    case 2:
      t = G4RotateY3D(180.0 * deg);
      break;
    case 3:
      t = G4RotateZ3D(90.0 * deg) * G4RotateY3D(180.0 * deg);
      break;
    case 4:
      t = G4RotateZ3D(-90.0 * deg);
      break;
  }
  physiSector = new G4PVPlacementGT(mpvgt, t, logicSector, Sector_Name,
                                    iSector);
  if (physiSector == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  createSectorSupport(physiSector);
  for (i = 1; i <= 2; i++)
    createSectorCover(i, physiSector);
  calcBoardTransform();
  for (i = 1; i <= nPlane; i++)
    createPlane(i, physiSector);
  for (i = 1; i <= nPlane; i++)
    for (j = 1; j <= nBoard; j++)
      createSectionReadoutBoard(i, j, physiSector);
}

/**
 * Calculate board transformations
 */
void GeoEKLMBelleII::calcBoardTransform()
{
  int i;
  int j;
  for (i = 0; i < nPlane; i++) {
    BoardTransform[i] = new G4Transform3D[nBoard];
    if (BoardTransform[i] == NULL) {
      B2FATAL("Memory allocation error.");
      exit(ENOMEM);
    }
    for (j = 0; j < nBoard; j++) {
      BoardTransform[i][j] = G4RotateZ3D(BoardPosition[i][j].phi) *
                             G4Translate3D(BoardPosition[i][j].r -
                                           0.5 * BoardSize.height, 0., 0.) *
                             G4RotateZ3D(90.0 * deg);
    }
  }
}

/**
 * createSectorCover - create sector cover
 * @iCover: number of cover
 * @@mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectorCover(int iCover, G4PVPlacementGT *mpvgt)
{
  double z;
  double lz;
  double ang;
  G4Tubs *solidCoverTube;
  G4Box *solidCoverBox;
  G4Box *box;
  G4IntersectionSolid *is;
  G4SubtractionSolid *solidCover;
  G4LogicalVolume *logicCover;
  G4PVPlacementGT *physiCover;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t;
  lz = 0.5 * (SectorPosition.length - SectorSupportPosition.length);
  std::string Cover_Name = "Cover_" + lexical_cast<string>(iCover) + "_" +
                           mpvgt->GetName();
  solidCoverTube = new G4Tubs("Tube_" + Cover_Name,
                              SectorSupportPosition.innerR,
                              SectorSupportPosition.outerR, 0.5 * lz, 0.0,
                              90.0 * deg);
  solidCoverBox = new G4Box("Box_" + Cover_Name,
                            0.5 * SectorSupportPosition.outerR,
                            0.5 * SectorSupportPosition.outerR,
                            0.5 * lz);
  if (solidCoverTube == NULL || solidCoverBox == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  box = new G4Box("SubtractionBox_" + Cover_Name,
                  0.5 * SectorSupportPosition.outerR,
                  0.5 * SectorSupportPosition.outerR,
                  lz);
  if (box == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  ang = getSectorSupportCornerAngle();
  t1 = G4Translate3D(0.5 * PlanePosition.outerR + SectorSupportPosition.X,
                     0.5 * PlanePosition.outerR + SectorSupportPosition.Y,
                     0.);
  t2 = G4Translate3D(SectorSupportPosition.X +
                     0.5 * SectorSupportPosition.outerR * cos(ang) -
                     0.5 * SectorSupportPosition.outerR * sin(ang),
                     SectorSupportPosition.outerR - SectorSupportSize.DeltaLY +
                     0.5 * SectorSupportPosition.outerR * cos(ang) +
                     0.5 * SectorSupportPosition.outerR * sin(ang),
                     0.) * G4RotateZ3D(ang);
  is = new G4IntersectionSolid("Intersection_" + Cover_Name, solidCoverTube,
                               solidCoverBox, t1);
  if (is == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidCover = new G4SubtractionSolid(Cover_Name, is, box, t2);
  if (solidCover == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicCover = new G4LogicalVolume(solidCover, Aluminium, Cover_Name);
  if (logicCover == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicCover, false);
  geometry::setColor(*logicCover, "#ff000022");
  z = 0.25 * (SectorPosition.length + SectorSupportPosition.length);
  if (iCover == 2)
    z = -z;
  t = G4Translate3D(0., 0., z);
  physiCover = new G4PVPlacementGT(mpvgt, t, logicCover, Cover_Name, iCover);
  if (physiCover == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/**
 * createSectorSuportBoxX - create X side of sector support structure
 * @mpvgt: mother physical volume with global transformation
 * @t: transformation (output)
 *
 * Sets t to the transformation of the box.
 */
G4Box *GeoEKLMBelleII::createSectorSupportBoxX(G4PVPlacementGT *mpvgt,
                                               G4Transform3D &t)
{
  double x1;
  double x2;
  x1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  x2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  t =  G4Translate3D(0.5 * (x1 + x2), SectorSupportPosition.Y +
                     0.5 * SectorSupportSize.Thickness, 0.);
  return new G4Box("BoxX_Support_" + mpvgt->GetName(), 0.5 *(x2 - x1),
                   0.5 * SectorSupportSize.Thickness,
                   0.5 * SectorSupportPosition.length);
}

/**
 * createSectorSuportBoxY - create Y side of sector support structure
 * @mpvgt: mother physical volume with global transformation
 * @t: transformation (output)
 *
 * Sets t to the transformation of the box.
 */
G4Box *GeoEKLMBelleII::createSectorSupportBoxY(G4PVPlacementGT *mpvgt,
                                               G4Transform3D &t)
{
  double y1;
  double y2;
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  y2 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  t = G4Translate3D(SectorSupportPosition.X + 0.5 * SectorSupportSize.Thickness,
                    0.5 * (y1 + y2), 0.) * G4RotateZ3D(90. * deg);
  return new G4Box("BoxY_Support_" + mpvgt->GetName(), 0.5 *(y2 - y1),
                   0.5 * SectorSupportSize.Thickness,
                   0.5 * SectorSupportPosition.length);
}

/**
 * getSectorSupportCornerAngle - get cutted corner angle
 */
double GeoEKLMBelleII::getSectorSupportCornerAngle()
{
  double x1;
  double y1;
  double x2;
  double y2;
  if (SectorSupportSize.CornerAngle < 0) {
    x1 = SectorSupportPosition.X;
    y1 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
    x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
    y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
              x2 * x2);
    SectorSupportSize.CornerAngle = atan2(y2 - y1, x2 - x1) * rad;
  }
  return SectorSupportSize.CornerAngle;
}

/**
 * createSectorSuportBoxTop - create box in the cutted corner of sector
 * support structure
 * @mpvgt: mother physical volume with global transformation
 * @t: transformation (output)
 *
 * Sets t to the transformation of the box.
 */
G4Box *GeoEKLMBelleII::createSectorSupportBoxTop(G4PVPlacementGT *mpvgt,
                                                 G4Transform3D &t)
{
  double x1;
  double y1;
  double x2;
  double y2;
  double ang;
  x1 = SectorSupportPosition.X;
  y1 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang = getSectorSupportCornerAngle();
  t = G4Translate3D(0.5 * (x1 + x2 + SectorSupportSize.Thickness * sin(ang)),
                    0.5 * (y1 + y2 - SectorSupportSize.Thickness * cos(ang)),
                    0.) * G4RotateZ3D(ang);
  return new G4Box("BoxTop_Support_" + mpvgt->GetName(), 0.5 * sqrt((x2 - x1) *
                   (x2 - x1) + (y2 - y1) *(y2 - y1)),
                   0.5 * SectorSupportSize.Thickness,
                   0.5 * SectorSupportPosition.length);
}

/**
 * createSectorSupportInnerTube - create inner tube of sector support structure
 * @mpvgt: mother physical volume with global transformation
 * @mlv: mother logical volume
 */
G4Tubs *GeoEKLMBelleII::createSectorSupportInnerTube(G4PVPlacementGT *mpvgt)
{
  double x1;
  double y1;
  double ang1;
  double ang2;
  x1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  ang1 = atan2(SectorSupportPosition.Y, x1);
  ang2 = atan2(y1, SectorSupportPosition.X);
  return new G4Tubs("InnerTube_Support_" + mpvgt->GetName(),
                    SectorSupportPosition.innerR,
                    SectorSupportPosition.innerR + SectorSupportSize.Thickness,
                    0.5 * SectorSupportPosition.length,
                    min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
}

/**
 * createSectorSupportOuterTube - create outer tube of sector support structure
 * @iSector: number of sector
 * @mpvgt: mother physical volume with global transformation
 */
G4Tubs *GeoEKLMBelleII::createSectorSupportOuterTube(G4PVPlacementGT *mpvgt)
{
  double x1;
  double y1;
  double x2;
  double y2;
  double ang1;
  double ang2;
  double r;
  r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
  x1 = sqrt(r * r - SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = SectorSupportPosition.Y;
  x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang1 = atan2(y1, x1);
  ang2 = atan2(y2, x2);
  return new G4Tubs("OuterTube_Support" + mpvgt->GetName(),
                    SectorSupportPosition.outerR - SectorSupportSize.Thickness,
                    SectorSupportPosition.outerR,
                    0.5 * SectorSupportPosition.length,
                    min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
}

/**
 * createSectorSupportCorner1 - create sector support corner 1
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectorSupportCorner1(G4PVPlacementGT *mpvgt)
{
  double lx;
  double x;
  G4Tubs *solidCorner1Tube;
  G4Box *solidCorner1Box1;
  G4Box *solidCorner1Box2;
  G4IntersectionSolid *is1;
  G4IntersectionSolid *solidCorner1;
  G4LogicalVolume *logicCorner1;
  G4PVPlacementGT *physiCorner1;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  std::string Corner1_Name = "Corner1_" + mpvgt->GetName();
  lx = SectorSupportSize.CornerX + SectorSupportSize.Corner1LX -
       SectorSupportSize.Thickness;
  solidCorner1Tube = new G4Tubs("Tube_" + Corner1_Name, 0.,
                                SectorSupportPosition.outerR -
                                SectorSupportSize.Thickness,
                                0.5 * SectorSupportSize.Corner1Thickness,
                                0., 90. * deg);
  solidCorner1Box1 = new G4Box("Box1_" + Corner1_Name, 0.5 * lx,
                               0.5 * SectorSupportPosition.outerR,
                               0.5 * SectorSupportSize.Corner1Thickness);
  solidCorner1Box2 = new G4Box("Box2_" + Corner1_Name,
                               0.5 *(lx / cos(getSectorSupportCornerAngle()) +
                                     SectorSupportSize.Corner1Width *
                                     sin(getSectorSupportCornerAngle())),
                               0.5 * SectorSupportSize.Corner1Width,
                               0.5 * SectorSupportSize.Corner1Thickness);
  if (solidCorner1Tube == NULL || solidCorner1Box1 == NULL ||
      solidCorner1Box2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  x = SectorSupportPosition.X + 0.5 * (SectorSupportSize.CornerX +
                                       SectorSupportSize.Corner1LX +
                                       SectorSupportSize.Thickness);
  t1 = G4Translate3D(x, 0.5 * SectorSupportPosition.outerR, 0.);
  t2 = G4Translate3D(x, SectorSupportPosition.outerR -
                     SectorSupportSize.DeltaLY -
                     (SectorSupportSize.Thickness +
                      0.5 * SectorSupportSize.Corner1Width) /
                     cos(getSectorSupportCornerAngle()) +
                     (SectorSupportSize.Thickness + 0.5 * lx) *
                     tan(getSectorSupportCornerAngle()), 0.) *
       G4RotateZ3D(getSectorSupportCornerAngle());
  is1 = new G4IntersectionSolid("Intersection1_" + Corner1_Name,
                                solidCorner1Tube, solidCorner1Box1, t1);
  if (is1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidCorner1 = new G4IntersectionSolid(Corner1_Name, is1, solidCorner1Box2,
                                         t2);
  if (solidCorner1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicCorner1 = new G4LogicalVolume(solidCorner1, Aluminium, Corner1_Name);
  if (logicCorner1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicCorner1, true);
  geometry::setColor(*logicCorner1, "#ff0000ff");
  t = G4Translate3D(0., 0., SectorSupportSize.Corner1Z);
  physiCorner1 = new G4PVPlacementGT(mpvgt, t, logicCorner1, Corner1_Name);
  if (physiCorner1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/*
 * createSectorSupportCorner2 - create sector support corner 2
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectorSupportCorner2(G4PVPlacementGT *mpvgt)
{
  double r;
  double x;
  double y;
  G4TriangularPrism *solidCorner2Prism;
  G4Tubs *solidCorner2Tubs;
  G4SubtractionSolid *solidCorner2;
  G4LogicalVolume *logicCorner2;
  G4PVPlacementGT *physiCorner2;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner2_Name = "Corner2_" + mpvgt->GetName();
  r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
  x = sqrt(r * r - y * y);
  solidCorner2Prism = new G4TriangularPrism("Prism_" + Corner2_Name,
                                            SectorSupportSize.Corner2LY,
                                            90. * deg,
                                            SectorSupportSize.Corner2LX,
                                            180. * deg,
                                            0.5 * SectorSupportSize.
                                            Corner2Thickness);
  solidCorner2Tubs = createSectorSupportOuterTube(mpvgt);
  if (solidCorner2Prism == NULL || solidCorner2Tubs == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (solidCorner2Prism->getSolid() == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t1 = G4Translate3D(-x, -y, 0.);
  solidCorner2 = new G4SubtractionSolid(Corner2_Name,
                                        solidCorner2Prism->getSolid(),
                                        solidCorner2Tubs, t1);
  if (solidCorner2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicCorner2 = new G4LogicalVolume(solidCorner2, Aluminium, Corner2_Name);
  if (logicCorner2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicCorner2, true);
  geometry::setColor(*logicCorner2, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner2Z);
  physiCorner2 = new G4PVPlacementGT(mpvgt, t, logicCorner2, Corner2_Name);
  if (physiCorner2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/*
 * createSectorSupportCorner3 - create sector support corner 3
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectorSupportCorner3(G4PVPlacementGT *mpvgt)
{
  double r;
  double x;
  double y;
  G4TriangularPrism *solidCorner3Prism;
  G4Tubs *solidCorner3Tubs;
  G4SubtractionSolid *solidCorner3;
  G4LogicalVolume *logicCorner3;
  G4PVPlacementGT *physiCorner3;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner3_Name = "Corner3_" + mpvgt->GetName();
  r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
      SectorSupportSize.Corner3LY;
  x = sqrt(r * r - y * y);
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
  solidCorner3Prism = new G4TriangularPrism("Prism_" + Corner3_Name,
                                            SectorSupportSize.Corner3LX, 0.,
                                            SectorSupportSize.Corner3LY,
                                            90. * deg,
                                            0.5 * SectorSupportSize.
                                            Corner3Thickness);
  solidCorner3Tubs = createSectorSupportInnerTube(mpvgt);
  if (solidCorner3Prism == NULL || solidCorner3Tubs == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (solidCorner3Prism->getSolid() == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t1 = G4Translate3D(-x, -y, 0.);
  solidCorner3 = new G4SubtractionSolid(Corner3_Name,
                                        solidCorner3Prism->getSolid(),
                                        solidCorner3Tubs, t1);
  if (solidCorner3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicCorner3 = new G4LogicalVolume(solidCorner3, Aluminium, Corner3_Name);
  if (logicCorner3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicCorner3, true);
  geometry::setColor(*logicCorner3, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner3Z);
  physiCorner3 = new G4PVPlacementGT(mpvgt, t, logicCorner3, Corner3_Name);
  if (physiCorner3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/*
 * createSectorSupportCorner4 - create sector support corner 4
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectorSupportCorner4(G4PVPlacementGT *mpvgt)
{
  double r;
  double x;
  double y;
  G4TriangularPrism *solidCorner4Prism;
  G4Tubs *solidCorner4Tubs;
  G4SubtractionSolid *solidCorner4;
  G4LogicalVolume *logicCorner4;
  G4PVPlacementGT *physiCorner4;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner4_Name = "Corner4_" + mpvgt->GetName();
  r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
  x = SectorSupportPosition.X + SectorSupportSize.Thickness +
      SectorSupportSize.Corner4LX;
  y = sqrt(r * r - x * x);
  x = SectorSupportPosition.X + SectorSupportSize.Thickness;
  solidCorner4Prism = new G4TriangularPrism("Corner4_" + mpvgt->GetName(),
                                            SectorSupportSize.Corner4LX, 0.,
                                            SectorSupportSize.Corner4LY,
                                            90. * deg,
                                            0.5 * SectorSupportSize.
                                            Corner4Thickness);
  solidCorner4Tubs = createSectorSupportInnerTube(mpvgt);
  if (solidCorner4Prism == NULL || solidCorner4Tubs == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (solidCorner4Prism->getSolid() == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t1 = G4Translate3D(-x, -y, 0.);
  solidCorner4 = new G4SubtractionSolid(Corner4_Name,
                                        solidCorner4Prism->getSolid(),
                                        solidCorner4Tubs, t1);
  if (solidCorner4 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicCorner4 = new G4LogicalVolume(solidCorner4, Aluminium, Corner4_Name);
  if (logicCorner4 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicCorner4, true);
  geometry::setColor(*logicCorner4, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner4Z);
  physiCorner4 = new G4PVPlacementGT(mpvgt, t, logicCorner4, Corner4_Name);
  if (physiCorner4 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/*
 * createSectorSupport - create sector support structure
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectorSupport(G4PVPlacementGT *mpvgt)
{
  G4Box *solidBoxX;
  G4Box *solidBoxY;
  G4Box *solidBoxTop;
  G4Tubs *solidOuterTube;
  G4Tubs *solidInnerTube;
  G4UnionSolid *us1;
  G4UnionSolid *us2;
  G4UnionSolid *us3;
  G4UnionSolid *solidSectorSupport;
  G4LogicalVolume *logicSectorSupport;
  G4PVPlacementGT *physiSectorSupport;
  G4Transform3D t;
  G4Transform3D tbx;
  G4Transform3D tby;
  G4Transform3D tbt;
  std::string SectorSupportName = "Support_" + mpvgt->GetName();
  solidBoxX = createSectorSupportBoxX(mpvgt, tbx);
  solidBoxY = createSectorSupportBoxY(mpvgt, tby);
  solidBoxTop = createSectorSupportBoxTop(mpvgt, tbt);
  solidOuterTube = createSectorSupportOuterTube(mpvgt);
  solidInnerTube = createSectorSupportInnerTube(mpvgt);
  if (solidBoxX == NULL || solidBoxY == NULL || solidOuterTube == NULL ||
      solidInnerTube == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t = G4Translate3D(0., 0., SectorSupportPosition.Z);
  us1 = new G4UnionSolid("Union1_Support_" + mpvgt->GetName(), solidInnerTube,
                         solidBoxY, tby);
  if (us1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  us2 = new G4UnionSolid("Union2_Support_" + mpvgt->GetName(), us1, solidBoxX,
                         tbx);
  if (us2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  us3 = new G4UnionSolid("Union3_Support_" + mpvgt->GetName(), us2,
                         solidOuterTube, G4Translate3D(0., 0., 0.));
  if (us3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidSectorSupport = new G4UnionSolid("Union4_Support_" + mpvgt->GetName(),
                                        us3, solidBoxTop, tbt);
  if (solidSectorSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicSectorSupport = new G4LogicalVolume(solidSectorSupport, Aluminium,
                                           SectorSupportName);
  if (logicSectorSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicSectorSupport, true);
  geometry::setColor(*logicSectorSupport, "#ff0000ff");
  physiSectorSupport = new G4PVPlacementGT(mpvgt, t, logicSectorSupport,
                                           SectorSupportName);
  if (physiSectorSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  createSectorSupportCorner1(mpvgt);
  createSectorSupportCorner2(mpvgt);
  createSectorSupportCorner3(mpvgt);
  createSectorSupportCorner4(mpvgt);
}

/**
 * subtractBoardSolids - subtract board solids from planes
 * @plane: plane solid without boards subtracted
 * @iPlane: plane number
 * @Plane_name: plane name
 */
G4SubtractionSolid *GeoEKLMBelleII::
subtractBoardSolids(G4SubtractionSolid *plane, int iPlane,
                    std::string Plane_Name)
{
  int i;
  int j;
  G4Transform3D t;
  G4Box *solidBoardBox;
  G4SubtractionSolid **ss[2];
  G4SubtractionSolid *prev_solid;
  solidBoardBox = new G4Box("PlateBox_" + Plane_Name, 0.5 * BoardSize.length,
                            0.5 * BoardSize.height,
                            0.5 *(PlanePosition.length + PlanePosition.Z));
  for (i = 0; i < nPlane; i++) {
    ss[i] = new G4SubtractionSolid*[nBoard];
    if (ss[i] == NULL)
      return NULL;
    for (j = 0; j < nBoard; j++) {
      t = BoardTransform[i][j];
      if (iPlane == 2)
        t = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t;
      if (i == 0) {
        if (j == 0)
          prev_solid = plane;
        else
          prev_solid = ss[0][j - 1];
      } else {
        if (j == 0)
          prev_solid = ss[0][nBoard - 1];
        else
          prev_solid = ss[1][j - 1];
      }
      ss[i][j] = new G4SubtractionSolid("BoardSubtraction_" +
                                        lexical_cast<string>(i) + "_" +
                                        lexical_cast<string>(j) + Plane_Name,
                                        prev_solid, solidBoardBox, t);
    }
  }
  return ss[nPlane - 1][nBoard - 1];
}

/*
 * createPlane - create plane
 * @iPlane: number of plane
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createPlane(int iPlane, G4PVPlacementGT *mpvgt)
{
  int i;
  int j;
  double r;
  double x;
  double y;
  double box_x;
  double box_lx;
  double ang;
  G4Tubs *solidPlaneTube;
  G4Box *solidPlaneBox;
  G4Box *box1;
  G4TriangularPrism *solidPlanePrism1;
  G4TriangularPrism *solidPlanePrism2;
  G4TriangularPrism *solidPlanePrism3;
  G4IntersectionSolid *is;
  G4SubtractionSolid *ss1;
  G4SubtractionSolid *ss2;
  G4SubtractionSolid *ss3;
  G4SubtractionSolid *ss4;
  G4SubtractionSolid *solidPlane;
  G4LogicalVolume *logicPlane;
  G4PVPlacementGT *physiPlane;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t3;
  G4Transform3D t4;
  G4Transform3D t5;
  std::string Plane_Name = "Plane_" + lexical_cast<string>(iPlane) + "_" +
                           mpvgt->GetName();
  solidPlaneTube = new G4Tubs("Tube_" + Plane_Name, PlanePosition.innerR,
                              PlanePosition.outerR, 0.5 * PlanePosition.length,
                              0.0, 90.0 * deg);
  box_x = max(SectorSupportPosition.Y, SectorSupportPosition.X) +
          SectorSupportSize.Thickness;
  box_lx =  PlanePosition.outerR - box_x;
  solidPlaneBox = new G4Box("Box_" + Plane_Name, 0.5 * box_lx, 0.5 * box_lx,
                            0.5 * PlanePosition.length);
  solidPlanePrism1 = new G4TriangularPrism("TriangularPrism1_" + Plane_Name,
                                           SectorSupportSize.Corner2LY,
                                           90. * deg,
                                           SectorSupportSize.Corner2LX,
                                           180. * deg, PlanePosition.length);
  solidPlanePrism2 = new G4TriangularPrism("TriangularPrism2_" + Plane_Name,
                                           SectorSupportSize.Corner3LX, 0.,
                                           SectorSupportSize.Corner3LY,
                                           90. * deg, PlanePosition.length);
  solidPlanePrism3 = new G4TriangularPrism("TriangularPrism3_" + Plane_Name,
                                           SectorSupportSize.Corner4LX, 0.,
                                           SectorSupportSize.Corner4LY,
                                           90. * deg, PlanePosition.length);
  if (solidPlaneTube == NULL || solidPlaneBox == NULL ||
      solidPlanePrism1 == NULL || solidPlanePrism2 == NULL ||
      solidPlanePrism3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (solidPlanePrism1->getSolid() == NULL ||
      solidPlanePrism2->getSolid() == NULL ||
      solidPlanePrism3->getSolid() == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  box1 = new G4Box("Box1_" + Plane_Name, 0.5 * box_lx, 0.5 * box_lx,
                   PlanePosition.length);
  if (box1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t1 = G4Translate3D(0.5 * (PlanePosition.outerR + box_x),
                     0.5 * (PlanePosition.outerR + box_x), 0.);
  if (iPlane == 2)
    t1 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t1;
  ang = getSectorSupportCornerAngle();
  x = max(SectorSupportPosition.Y, SectorSupportPosition.X);
  y = SectorSupportPosition.outerR -
      SectorSupportSize.DeltaLY -
      SectorSupportSize.TopCornerHeight;
  if (iPlane == 1) {
    t2 = G4Translate3D(x + 0.5 * box_lx * cos(ang) - 0.5 * box_lx * sin(ang),
                       y + 0.5 * box_lx * cos(ang) + 0.5 * box_lx * sin(ang),
                       0.) * G4RotateZ3D(ang);
  } else {
    t2 = G4Translate3D(y + 0.5 * box_lx * cos(ang) + 0.5 * box_lx * sin(ang),
                       x + 0.5 * box_lx * cos(ang) - 0.5 * box_lx * sin(ang),
                       0.) * G4RotateZ3D(-ang);
  }
  r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
  x = sqrt(r * r - y * y);
  t3 = G4Translate3D(x, y, 0.);
  if (iPlane == 2)
    t3 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t3;
  r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
      SectorSupportSize.Corner3LY;
  x = sqrt(r * r - y * y);
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
  t4 = G4Translate3D(x, y, 0.);
  if (iPlane == 2)
    t4 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t4;
  x = SectorSupportPosition.X + SectorSupportSize.Thickness +
      SectorSupportSize.Corner4LX;
  y = sqrt(r * r - x * x);
  x = SectorSupportPosition.X + SectorSupportSize.Thickness;
  t5 = G4Translate3D(x, y, 0.);
  if (iPlane == 2)
    t5 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t5;
  is = new G4IntersectionSolid("Intersection_" + Plane_Name, solidPlaneTube,
                               solidPlaneBox, t1);
  if (is == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  ss1 = new G4SubtractionSolid("Subtraction1_" + Plane_Name, is, box1, t2);
  if (ss1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  ss2 = new G4SubtractionSolid("Subtraction2_" + Plane_Name, ss1,
                               solidPlanePrism1->getSolid(), t3);
  if (ss2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  ss3 = new G4SubtractionSolid("Subtraction3_" + Plane_Name, ss2,
                               solidPlanePrism2->getSolid(), t4);
  if (ss3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  ss4 = new G4SubtractionSolid("Subtraction4_" + Plane_Name, ss3,
                               solidPlanePrism3->getSolid(), t5);
  if (ss4 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidPlane = subtractBoardSolids(ss4, iPlane, Plane_Name);
  if (solidPlane == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicPlane = new G4LogicalVolume(solidPlane, Air, Plane_Name);
  if (logicPlane == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicPlane, false);
  if (iPlane == 1) {
    t = G4Translate3D(PlanePosition.X, PlanePosition.Y, PlanePosition.Z);
  } else {
    t = G4Translate3D(PlanePosition.X, PlanePosition.Y, -PlanePosition.Z) *
        G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.));
  }
  physiPlane = new G4PVPlacementGT(mpvgt, t, logicPlane, Plane_Name, iPlane);
  if (physiPlane == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  for (i = 1; i <= nSection + 1; i++)
    createSectionSupport(i, iPlane, physiPlane);
  for (i = 1; i <= 2; i++)
    for (j = 1; j <= nStrip; j++)
      createPlasticListElement(i, j, physiPlane);
  for (i = 1; i <= nStrip; i++)
    createStrip(i, physiPlane);
}

/**
 * createSectionReadoutBoard - create readout board
 * @iPlane: number of plane
 * @iBoard: number of board
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectionReadoutBoard(int iPlane, int iBoard,
                                               G4PVPlacementGT *mpvgt)
{
  int i;
  G4Box *solidSectionReadoutBoard;
  G4LogicalVolume *logicSectionReadoutBoard;
  G4PVPlacementGT *physiSectionReadoutBoard;
  std::string Board_Name = "SectionReadoutBoard_" +
                           lexical_cast<string>(iBoard) + "_Plane_" +
                           lexical_cast<string>(iPlane) + "_" +
                           mpvgt->GetName();
  solidSectionReadoutBoard = new G4Box(Board_Name,
                                       0.5 * BoardSize.length,
                                       0.5 * BoardSize.height,
                                       0.5 * BoardSize.width);
  if (solidSectionReadoutBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicSectionReadoutBoard = new G4LogicalVolume(solidSectionReadoutBoard, Air,
                                                 Board_Name);
  if (logicSectionReadoutBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicSectionReadoutBoard, false);
  physiSectionReadoutBoard = new G4PVPlacementGT(mpvgt,
                                                 BoardTransform[iPlane - 1]
                                                 [iBoard - 1],
                                                 logicSectionReadoutBoard,
                                                 Board_Name, iBoard, iPlane);
  if (physiSectionReadoutBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  createBaseBoard(physiSectionReadoutBoard);
  for (i = 1; i <= nStripBoard; i++)
    createStripBoard(i, physiSectionReadoutBoard);
}

/**
 * createBaseBoard - create base board of section readout board
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createBaseBoard(G4PVPlacementGT *mpvgt)
{
  G4Box *solidBaseBoard;
  G4LogicalVolume *logicBaseBoard;
  G4PVPlacementGT *physiBaseBoard;
  G4Transform3D t;
  std::string Board_Name = "BaseBoard_" + mpvgt->GetName();
  solidBaseBoard = new G4Box(Board_Name, 0.5 * BoardSize.length,
                             0.5 * BoardSize.base_height,
                             0.5 * BoardSize.base_width);
  if (solidBaseBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicBaseBoard = new G4LogicalVolume(solidBaseBoard, Aluminium,
                                       Board_Name);
  if (logicBaseBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicBaseBoard, true);
  geometry::setColor(*logicBaseBoard, "#0000ffff");
  t = G4Translate3D(0., -0.5 * BoardSize.height + 0.5 * BoardSize.base_height,
                    0.);
  physiBaseBoard = new G4PVPlacementGT(mpvgt, t, logicBaseBoard, Board_Name);
  if (physiBaseBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/**
 * createStripBoard - create strip readout board
 * @iBoard: number of board
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createStripBoard(int iBoard, G4PVPlacementGT *mpvgt)
{
  G4Box *solidStripBoard;
  G4LogicalVolume *logicStripBoard;
  G4PVPlacementGT *physiStripBoard;
  G4Transform3D t;
  std::string Board_Name = "StripBoard_" + lexical_cast<string>(iBoard) + "_" +
                           mpvgt->GetName();
  solidStripBoard = new G4Box(Board_Name, 0.5 * BoardSize.strip_length,
                              0.5 * BoardSize.strip_height,
                              0.5 * BoardSize.strip_width);
  if (solidStripBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicStripBoard = new G4LogicalVolume(solidStripBoard, Aluminium,
                                        Board_Name);
  if (logicStripBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicStripBoard, true);
  geometry::setColor(*logicStripBoard, "#0000ffff");
  t = G4Translate3D(-0.5 * BoardSize.length + StripBoardPosition[iBoard - 1].x,
                    -0.5 * BoardSize.height + BoardSize.base_height +
                    0.5 * BoardSize.strip_height, 0.);
  physiStripBoard = new G4PVPlacementGT(mpvgt, t, logicStripBoard, Board_Name);
  if (physiStripBoard == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/**
 * createSectionSupport - create section support
 * @iSectionSupport: number of section support
 * @iPlane: number of plane
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createSectionSupport(int iSectionSupport, int iPlane,
                                          G4PVPlacementGT *mpvgt)
{
  G4Box *solidBoxTop;
  G4Box *solidBoxMiddle;
  G4Box *solidBoxBottom;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4UnionSolid *us;
  G4UnionSolid *solidSectionSupport;
  G4LogicalVolume *logicSectionSupport;
  G4PVPlacementGT *physiSectionSupport;
  std::string SectionSupportName = "SectionSupport_" +
                                   lexical_cast<string>(iSectionSupport) +
                                   "_" + mpvgt->GetName();
  solidBoxTop = new G4Box("BoxTop_" + SectionSupportName,
                          0.5 *(SectionSupportPosition[iPlane - 1]
                                [iSectionSupport - 1].length -
                                SectionSupportPosition[iPlane - 1]
                                [iSectionSupport - 1].deltal_left -
                                SectionSupportPosition[iPlane - 1]
                                [iSectionSupport - 1].deltal_right),
                          0.5 * SectionSupportTopWidth,
                          0.5 * SectionSupportTopThickness);
  solidBoxMiddle =  new G4Box("BoxMiddle_" + SectionSupportName,
                              0.5 *(SectionSupportPosition[iPlane - 1]
                                    [iSectionSupport - 1].length -
                                    SectionSupportPosition[iPlane - 1]
                                    [iSectionSupport - 1].deltal_left -
                                    SectionSupportPosition[iPlane - 1]
                                    [iSectionSupport - 1].deltal_right),
                              0.5 * SectionSupportMiddleWidth,
                              0.5 * SectionSupportMiddleThickness);
  solidBoxBottom = new G4Box("BoxBottom_" + SectionSupportName,
                             0.5 * SectionSupportPosition[iPlane - 1]
                             [iSectionSupport - 1].length,
                             0.5 * SectionSupportTopWidth,
                             0.5 * SectionSupportTopThickness);
  if (solidBoxTop == NULL || solidBoxMiddle == NULL || solidBoxBottom == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t1 = G4Translate3D(0., 0., 0.5 * (SectionSupportMiddleThickness +
                                    SectionSupportTopThickness));
  t2 = G4Translate3D(0.5 * (SectionSupportPosition[iPlane - 1]
                            [iSectionSupport - 1].deltal_right -
                            SectionSupportPosition[iPlane - 1]
                            [iSectionSupport - 1].deltal_left),
                     0., -0.5 * (SectionSupportMiddleThickness +
                                 SectionSupportTopThickness));
  us = new G4UnionSolid("Union1_" + SectionSupportName, solidBoxMiddle,
                        solidBoxTop, t1);
  if (us == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidSectionSupport = new G4UnionSolid(SectionSupportName, us, solidBoxBottom,
                                         t2);
  if (solidSectionSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicSectionSupport = new G4LogicalVolume(solidSectionSupport, Aluminium,
                                            SectionSupportName);
  if (logicSectionSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicSectionSupport, true);
  geometry::setColor(*logicSectionSupport, "#ff0000ff");
  t = G4Translate3D(0.5 * (SectionSupportPosition[iPlane - 1]
                           [iSectionSupport - 1].deltal_left -
                           SectionSupportPosition[iPlane - 1]
                           [iSectionSupport - 1].deltal_right) +
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].x,
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].y,
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].z);
  physiSectionSupport = new G4PVPlacementGT(mpvgt, t, logicSectionSupport,
                                            SectionSupportName,
                                            iSectionSupport);
  if (physiSectionSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/**
 * createPlasticListElement - create plastic list element
 * @iListPlane: number of list plane
 * @iList: number of list element
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createPlasticListElement(int iListPlane, int iList,
                                              G4PVPlacementGT *mpvgt)
{
  double ly;
  double y;
  double z;
  G4Box *solidList;
  G4LogicalVolume *logicList;
  G4PVPlacementGT *physiList;
  G4Transform3D t;
  std::string List_Name = "List_" + lexical_cast<string>(iList) + "_ListPlane_"
                          + lexical_cast<string>(iListPlane) + "_" +
                          mpvgt->GetName();
  ly = Strip_width;
  if (iList % 15 <= 1)
    ly = ly - PlasticListDeltaL;
  solidList = new G4Box(List_Name, 0.5 * StripPosition[iList - 1].length,
                        0.5 * ly, 0.5 * PlasticListWidth);
  if (solidList == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicList = new G4LogicalVolume(solidList, Polystyrene, List_Name);
  if (logicList == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicList, false);
  geometry::setColor(*logicList, "#00ff00ff");
  y = StripPosition[iList - 1].Y;
  if (iList % 15 == 1)
    y = y + 0.5 * PlasticListDeltaL;
  else if (iList % 15 == 0)
    y = y - 0.5 * PlasticListDeltaL;
  z = 0.5 * (Strip_thickness + PlasticListWidth);
  if (iListPlane == 2)
    z = -z;
  t = G4Translate3D(StripPosition[iList - 1].X, y, z);
  physiList = new G4PVPlacementGT(mpvgt, t, logicList, List_Name, iList,
                                  iListPlane);
  if (physiList == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/*
 * createStrip - create strip
 * @iStrip: number of strip
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createStrip(int iStrip, G4PVPlacementGT *mpvgt)
{
  G4Box *solidStrip;
  G4LogicalVolume *logicStrip;
  G4PVPlacementGT *physiStrip;
  G4Transform3D t;
  std::string Strip_Name = "Strip_" + lexical_cast<string>(iStrip) + "_" +
                           mpvgt->GetName();
  solidStrip = new G4Box(Strip_Name, 0.5 * StripPosition[iStrip - 1].length,
                         0.5 * Strip_width, 0.5 * Strip_thickness);
  if (solidStrip == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicStrip = new G4LogicalVolume(solidStrip, Polystyrene, Strip_Name, 0,
                                   m_sensitive, 0);
  if (logicStrip == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicStrip, true);
  geometry::setColor(*logicStrip, "#ffffffff");
  t = G4Translate3D(StripPosition[iStrip - 1].X, StripPosition[iStrip - 1].Y,
                    0.0);
  physiStrip = new G4PVPlacementGT(mpvgt, t, logicStrip, Strip_Name, iStrip);
  if (physiStrip == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
}

/*
 * create - creation of the detector geometry
 * @content: XML directory
 * @topVolume: Geant world volume
 * @type:
 */
void GeoEKLMBelleII::create(const GearDir& content, G4LogicalVolume& topVolume,
                            geometry::GeometryTypes type)
{
  int i;
  readXMLData(content);
  createMaterials();
  for (i = 1; i <= 2; i++)
    createEndcap(i, &topVolume);
}

