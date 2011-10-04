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
#include <eklm/simeklm/EKLMSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/format.hpp>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <G4Box.hh>
#include <G4Tubs.hh>
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
  SectorSupport_CornerAngle = -1;
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
  GearDir SectorSupport(Sector);
  SectorSupport.append("/SectorSupport");
  readPositionData(SectorSupportPosition, SectorSupport);
  SectorSupportThickness = SectorSupport.getLength("Thickness") * cm;
  SectorSupport_DeltaLY = SectorSupport.getLength("DeltaLY") * cm;
  SectorSupport_CornerX = SectorSupport.getLength("CornerX") * cm;
  SectorSupport_TopCornerHeight = SectorSupport.getLength("TopCornerHeight") *
                                  cm;
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
    SectionSupportPosition[j] = new struct EKLMElementPosition[nSection + 1];
    for (i = 0; i <= nSection; i++) {
      GearDir SectionSupportContent(Sections);
      SectionSupportContent.append((format("/SectionSupportData[%1%]") % (j +
                                    1)).str());
      SectionSupportContent.append((format("/SectionSupport[%1%]") % (i +
                                    1)).str());
      SectionSupportPosition[j][i].length = SectionSupportContent.
                                            getLength("Length") * cm;
      SectionSupportPosition[j][i].X = SectionSupportContent.
                                       getLength("PositionX") * cm;
      SectionSupportPosition[j][i].Y = SectionSupportContent.
                                       getLength("PositionY") * cm;
      SectionSupportPosition[j][i].Z = SectionSupportContent.
                                       getLength("PositionZ") * cm;
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
  G4Transform3D t;
  if (iEndcap == 1)
    z = -EndcapPosition.Z + 94.0 * cm;
  else
    z = EndcapPosition.Z;
  t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y, z);
  for (i = 1; i <= nLayer; i++)
    createLayer(i, iEndcap, mlv, &t);
}

/*
 * createLayer - create layer
 * @iLayer: number of layer
 * @iEndcap: number of endcap
 * @mlv: mother logical volume
 * @mtr: transformation of mother volume reference frame from
 *       global reference frame
 */
void GeoEKLMBelleII::createLayer(int iLayer, int iEndcap,
                                 G4LogicalVolume *mlv, G4Transform3D *mtr)
{
  int i;
  G4Tubs *solidLayer;
  G4LogicalVolume *logicLayer;
  G4PVPlacementGT *physiLayer;
  G4Transform3D t;
  std::string Layer_Name = "Layer_" + lexical_cast<string>(iLayer) + "_Endcap_"
                           + lexical_cast<string>(iEndcap);
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
  t = G4Translate3D(0.0, 0.0, LayerPosition.Z) * (*mtr);
  physiLayer = new G4PVPlacementGT(t, t, logicLayer, Layer_Name, mlv, iLayer,
                                   iEndcap);
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
  for (i = 1; i <= nPlane; i++)
    createPlane(i, physiSector);
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
  box = new G4Box(*solidCoverBox);
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
                     SectorSupportPosition.outerR - SectorSupport_DeltaLY +
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
  geometry::setVisibility(*logicCover, true);
  geometry::setColor(*logicCover, "#ff000077");
  z = 0.25 * (SectorPosition.length + SectorSupportPosition.length);
  if (iCover == 1)
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
                     0.5 * SectorSupportThickness, 0.);
  return new G4Box("BoxX_Support_" + mpvgt->GetName(), 0.5 *(x2 - x1),
                   0.5 * SectorSupportThickness,
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
  y2 = SectorSupportPosition.outerR - SectorSupport_DeltaLY;
  t = G4Translate3D(SectorSupportPosition.X + 0.5 * SectorSupportThickness,
                    0.5 * (y1 + y2), 0.) * G4RotateZ3D(90. * deg);
  return new G4Box("BoxY_Support_" + mpvgt->GetName(), 0.5 *(y2 - y1),
                   0.5 * SectorSupportThickness,
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
  if (SectorSupport_CornerAngle < 0) {
    x1 = SectorSupportPosition.X;
    y1 = SectorSupportPosition.outerR - SectorSupport_DeltaLY;
    x2 = SectorSupport_CornerX;
    y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
              x2 * x2);
    SectorSupport_CornerAngle = atan2(y2 - y1, x2 - x1) * rad;
  }
  return SectorSupport_CornerAngle;
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
  y1 = SectorSupportPosition.outerR - SectorSupport_DeltaLY;
  x2 = SectorSupport_CornerX;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang = getSectorSupportCornerAngle();
  t = G4Translate3D(0.5 * (x1 + x2 + SectorSupportThickness * sin(ang)),
                    0.5 * (y1 + y2 - SectorSupportThickness * cos(ang)),
                    0.) * G4RotateZ3D(ang);
  return new G4Box("BoxTop_Support_" + mpvgt->GetName(), 0.5 * sqrt((x2 - x1) *
                   (x2 - x1) + (y2 - y1) *(y2 - y1)),
                   0.5 * SectorSupportThickness,
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
                    SectorSupportPosition.innerR + SectorSupportThickness,
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
  r = SectorSupportPosition.outerR - SectorSupportThickness;
  x1 = sqrt(r * r - SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = SectorSupportPosition.Y;
  x2 = SectorSupport_CornerX;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang1 = atan2(y1, x1);
  ang2 = atan2(y2, x2);
  return new G4Tubs("OuterTube_Support" + mpvgt->GetName(),
                    SectorSupportPosition.outerR - SectorSupportThickness,
                    SectorSupportPosition.outerR,
                    0.5 * SectorSupportPosition.length,
                    min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
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
  solidSectorSupport = new G4UnionSolid("Support_" + mpvgt->GetName(), us3,
                                        solidBoxTop, tbt);
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
  double x;
  double y;
  double box_x;
  double box_lx;
  double ang;
  G4Tubs *solidPlaneTube;
  G4Box *solidPlaneBox;
  G4Box *box;
  G4IntersectionSolid *is;
  G4SubtractionSolid *solidPlane;
  G4LogicalVolume *logicPlane;
  G4PVPlacementGT *physiPlane;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  std::string Plane_Name = "Plane_" + lexical_cast<string>(iPlane) + "_" +
                           mpvgt->GetName();
  solidPlaneTube = new G4Tubs("Tube_" + Plane_Name, PlanePosition.innerR,
                              PlanePosition.outerR, 0.5 * PlanePosition.length,
                              0.0, 90.0 * deg);
  box_x = max(SectorSupportPosition.Y, SectorSupportPosition.X) +
          SectorSupportThickness;
  box_lx =  PlanePosition.outerR - box_x;
  solidPlaneBox = new G4Box("Box_" + Plane_Name, 0.5 * box_lx, 0.5 * box_lx,
                            0.5 * PlanePosition.length);
  if (solidPlaneTube == NULL || solidPlaneBox == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  box = new G4Box(*solidPlaneBox);
  if (box == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  ang = getSectorSupportCornerAngle();
  x = max(SectorSupportPosition.Y, SectorSupportPosition.X);
  y = SectorSupportPosition.outerR -
      SectorSupport_DeltaLY -
      SectorSupport_TopCornerHeight;
  t1 = G4Translate3D(0.5 * (PlanePosition.outerR + box_x),
                     0.5 * (PlanePosition.outerR + box_x), 0.);
  if (iPlane == 1) {
    t2 = G4Translate3D(x + 0.5 * box_lx * cos(ang) - 0.5 * box_lx * sin(ang),
                       y + 0.5 * box_lx * cos(ang) + 0.5 * box_lx * sin(ang),
                       0.) * G4RotateZ3D(ang);
  } else {
    t2 = G4Translate3D(y + 0.5 * box_lx * cos(ang) + 0.5 * box_lx * sin(ang),
                       x + 0.5 * box_lx * cos(ang) - 0.5 * box_lx * sin(ang),
                       0.) * G4RotateZ3D(-ang);
  }
  is = new G4IntersectionSolid("Intersection_" + Plane_Name, solidPlaneTube,
                               solidPlaneBox, t1);
  if (is == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidPlane = new G4SubtractionSolid(Plane_Name, is, box, t2);
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
                          0.5 * SectionSupportPosition[iPlane - 1]
                          [iSectionSupport -
                           1].length,
                          0.5 * SectionSupportTopWidth,
                          0.5 * SectionSupportTopThickness);
  solidBoxMiddle =  new G4Box("BoxMiddle_" + SectionSupportName,
                              0.5 * SectionSupportPosition[iPlane - 1]
                              [iSectionSupport -
                               1].length,
                              0.5 * SectionSupportMiddleWidth,
                              0.5 * SectionSupportMiddleThickness);
  solidBoxBottom = new G4Box("BoxBottom_" + SectionSupportName,
                             0.5 * SectionSupportPosition[iPlane - 1]
                             [iSectionSupport -
                              1].length,
                             0.5 * SectionSupportTopWidth,
                             0.5 * SectionSupportTopThickness);
  if (solidBoxTop == NULL || solidBoxMiddle == NULL || solidBoxBottom == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t1 = G4Translate3D(0., 0., 0.5 * (SectionSupportMiddleThickness +
                                    SectionSupportTopThickness));
  t2 = G4Translate3D(0., 0., -0.5 * (SectionSupportMiddleThickness +
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
  t = G4Translate3D(SectionSupportPosition[iPlane - 1][iSectionSupport - 1].X,
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].Y,
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].Z);
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
  geometry::setVisibility(*logicList, true);
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

