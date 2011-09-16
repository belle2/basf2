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
  int i, StripID;
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
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(PlanePosition, Plane);
  GearDir Strip(Sector);
  nStrip = Strip.getNumberNodes("Strips/Strip");
  Strip_width  = Strip.getLength("Strips/Width") * cm;
  Strip_thickness = Strip.getLength("Strips/Thickness") * cm;
  StripPosition = new struct EKLMElementPosition[nStrip];
  for (i = 0; i < nStrip; i++) {
    GearDir StripContent(Strip);
    StripContent.append((format("/Strips/Strip[%1%]") % (i + 1)).str());
    string sStrip = StripContent.getString("@id");
    StripID = atoi(sStrip.c_str());
    StripPosition[StripID].length = StripContent.getLength("Length") * cm;
    StripPosition[StripID].X = StripContent.getLength("PositionX") * cm;
    StripPosition[StripID].Y = StripContent.getLength("PositionY") * cm;
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
  G4Tubs *solidEndcap;
  G4LogicalVolume *logicEndcap;
  G4PVPlacementGT *physiEndcap;
  G4Transform3D t;
  std::string Endcap_Name = "Endcap_" + lexical_cast<string>(iEndcap);
  solidEndcap = new G4Tubs(Endcap_Name, EndcapPosition.innerR,
                           EndcapPosition.outerR, EndcapPosition.length / 2,
                           0. * deg, 360. * deg);
  if (solidEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (iEndcap == 1) {
    z = EndcapPosition.Z;
    t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y, z) *
        G4RotateY3D(180. * deg);
  } else {
    z = -EndcapPosition.Z + 94.0 * cm;
    t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y, z);
  }
  logicEndcap = new G4LogicalVolume(solidEndcap, Air, Endcap_Name);
  if (logicEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicEndcap, false);
  physiEndcap = new G4PVPlacementGT(t, t, logicEndcap, Endcap_Name, mlv,
                                    iEndcap);
  if (physiEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  for (i = 1; i <= nLayer; i++)
    createLayer(i, physiEndcap);
}

/*
 * createLayer - create layer
 * @iLayer: number of layer
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createLayer(int iLayer, G4PVPlacementGT *mpvgt)
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
  G4Tubs *solidSector;
  G4LogicalVolume *logicSector;
  G4PVPlacementGT *physiSector;
  G4Transform3D t;
  std::string Sector_Name = "Sector_" + lexical_cast<string>(iSector) + "_" +
                            mpvgt->GetName();
  solidSector = new G4Tubs(Sector_Name, SectorPosition.innerR,
                           SectorPosition.outerR, SectorPosition.length / 2.0,
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
  createSectorSupport(logicSector);
  for (i = 1; i <= nPlane; i++)
    createPlane(i, physiSector);
}

/**
 * createSectorSuportBoxX - create X side of sector support structure
 * @mlv: mother logical volume
 * @t: transformation (output)
 *
 * Sets t to the transformation of the box.
 */
G4Box *GeoEKLMBelleII::createSectorSupportBoxX(G4LogicalVolume *mlv,
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
  return new G4Box("BoxX_Support_" + mlv->GetName(), 0.5 *(x2 - x1),
                   0.5 * SectorSupportThickness,
                   0.5 * SectorSupportPosition.length);
}

/**
 * createSectorSuportBoxY - create Y side of sector support structure
 * @mlv: mother logical volume
 * @t: transformation (output)
 *
 * Sets t to the transformation of the box.
 */
G4Box *GeoEKLMBelleII::createSectorSupportBoxY(G4LogicalVolume *mlv,
                                               G4Transform3D &t)
{
  double y1;
  double y2;
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  y2 = SectorSupportPosition.outerR - SectorSupport_DeltaLY;
  t = G4Translate3D(SectorSupportPosition.X + 0.5 * SectorSupportThickness,
                    0.5 * (y1 + y2), 0.) * G4RotateZ3D(90. * deg);
  return new G4Box("BoxY_Support_" + mlv->GetName(), 0.5 *(y2 - y1),
                   0.5 * SectorSupportThickness,
                   0.5 * SectorSupportPosition.length);
}

/**
 * createSectorSuportBoxTop - create box in the cutted corner of sector
 * support structure
 * @mlv: mother logical volume
 * @t: transformation (output)
 *
 * Sets t to the transformation of the box.
 */
G4Box *GeoEKLMBelleII::createSectorSupportBoxTop(G4LogicalVolume *mlv,
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
  ang = atan2(y2 - y1, x2 - x1);
  t = G4Translate3D(0.5 * (x1 + x2 + SectorSupportThickness * sin(ang)),
                    0.5 * (y1 + y2 - SectorSupportThickness * cos(ang)),
                    0.) * G4RotateZ3D(ang * rad);
  return new G4Box("BoxTop_Support_" + mlv->GetName(), 0.5 * sqrt((x2 - x1) *
                   (x2 - x1) + (y2 - y1) *(y2 - y1)),
                   0.5 * SectorSupportThickness,
                   0.5 * SectorSupportPosition.length);
}

/**
 * createSectorSupportInnerTube - create inner tube of sector support structure
 * @mlv: mother logical volume
 */
G4Tubs *GeoEKLMBelleII::createSectorSupportInnerTube(G4LogicalVolume *mlv)
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
  return new G4Tubs("InnerTube_Support_" + mlv->GetName(),
                    SectorSupportPosition.innerR,
                    SectorSupportPosition.innerR + SectorSupportThickness,
                    SectorSupportPosition.length / 2.0,
                    min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
}

/**
 * createSectorSupportOuterTube - create outer tube of sector support structure
 * @iSector: number of sector
 * @mlv: mother logical volume
 */
G4Tubs *GeoEKLMBelleII::createSectorSupportOuterTube(G4LogicalVolume *mlv)
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
  return new G4Tubs("OuterTube_Support" + mlv->GetName(),
                    SectorSupportPosition.outerR - SectorSupportThickness,
                    SectorSupportPosition.outerR,
                    SectorSupportPosition.length / 2.0,
                    min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
}

/*
 * createSectorSupport - create sector support structure
 * @mlv: mother logical volume
 */
void GeoEKLMBelleII::createSectorSupport(G4LogicalVolume *mlv)
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
  G4PVPlacement *physiSectorSupport;
  G4Transform3D t;
  G4Transform3D tbx;
  G4Transform3D tby;
  G4Transform3D tbt;
  std::string SectorSupportName = "Support_" + mlv->GetName();
  solidBoxX = createSectorSupportBoxX(mlv, tbx);
  solidBoxY = createSectorSupportBoxY(mlv, tby);
  solidBoxTop = createSectorSupportBoxTop(mlv, tbt);
  solidOuterTube = createSectorSupportOuterTube(mlv);
  solidInnerTube = createSectorSupportInnerTube(mlv);
  if (solidBoxX == NULL || solidBoxY == NULL || solidOuterTube == NULL ||
      solidInnerTube == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t = G4Translate3D(0., 0., SectorSupportPosition.Z);
  us1 = new G4UnionSolid("SupportUnion1_" + mlv->GetName(), solidInnerTube,
                         solidBoxY, tby);
  if (us1 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  us2 = new G4UnionSolid("SupportUnion2_" + mlv->GetName(), us1, solidBoxX,
                         tbx);
  if (us2 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  us3 = new G4UnionSolid("SupportUnion3_" + mlv->GetName(), us2, solidOuterTube,
                         G4Translate3D(0., 0., 0.));
  if (us3 == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  solidSectorSupport = new G4UnionSolid("Support_" + mlv->GetName(), us3,
                                        solidBoxTop, tbt);
  if (solidSectorSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicSectorSupport = new G4LogicalVolume(solidSectorSupport, Iron,
                                           SectorSupportName);
  if (logicSectorSupport == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicSectorSupport, true);
  geometry::setColor(*logicSectorSupport, "#ff0000ff");
  physiSectorSupport = new G4PVPlacement(t, logicSectorSupport,
                                         SectorSupportName, mlv, false, 1);
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
  G4Tubs *solidPlane;
  G4LogicalVolume *logicPlane;
  G4PVPlacementGT *physiPlane;
  G4Transform3D t;
  std::string Plane_Name = "Plane_" + lexical_cast<string>(iPlane) + "_" +
                           mpvgt->GetName();
  solidPlane = new G4Tubs(Plane_Name, PlanePosition.innerR,
                          PlanePosition.outerR, PlanePosition.length / 2.0,
                          0.0, 90.0 * deg);
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
  for (i = 1; i <= nStrip; i++)
    createStrip(i, iPlane, physiPlane);
}

/*
 * createStrip - create strip
 * @iStrip: number of strip
 * @iPlane: number of plane
 * @mpvgt: mother physical volume with global transformation
 */
void GeoEKLMBelleII::createStrip(int iStrip, int iPlane, G4PVPlacementGT *mpvgt)
{
  G4Box *solidStrip;
  G4LogicalVolume *logicStrip;
  G4PVPlacementGT *physiStrip;
  G4Transform3D t;
  std::string Strip_Name = "Strip_" + lexical_cast<string>(iStrip) + "_" +
                           mpvgt->GetName();
  solidStrip = new G4Box(Strip_Name, StripPosition[iStrip].length / 2.0,
                         Strip_width / 2.0, Strip_thickness / 2.0);
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
  if (iPlane == 1)
    geometry::setColor(*logicStrip, "#ffff00ff");
  else
    geometry::setColor(*logicStrip, "#0000ffff");
  t = G4Translate3D(StripPosition[iStrip].X, StripPosition[iStrip].Y, 0.0);
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

