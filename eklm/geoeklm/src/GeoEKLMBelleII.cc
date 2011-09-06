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
}

/*
 * ~GeoEKLMBelleII - GeoEKLMBelleII destructor
 */
GeoEKLMBelleII::~GeoEKLMBelleII()
{
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
  GearDir Layer(EndCap);
  Layer.append("/Layer");
  readPositionData(LayerPosition, Layer);
  Layer_shiftZ = Layer.getLength("ShiftZ") * cm;
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readPositionData(SectorPosition, Sector);
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

/*
 * createEndcap - create endcap
 * @iEndcap: number of endcap
 * @mlv: mother logical volume
 */
void GeoEKLMBelleII::createEndcap(int iEndcap, G4LogicalVolume *mlv)
{
  int i;
  int zor;
  G4Tubs *solidEndcap;
  G4LogicalVolume *logicEndcap;
  G4PVPlacement *physiEndcap;
  G4Transform3D t;
  double z;
  Endcap_Name = "Endcap_" + lexical_cast<string>(iEndcap);
  solidEndcap = new G4Tubs(Endcap_Name, EndcapPosition.innerR,
                           EndcapPosition.outerR, EndcapPosition.length / 2,
                           0. * deg, 360. * deg);
  if (solidEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  if (iEndcap == 1) {
    z = EndcapPosition.Z;
    zor = 1;
  } else {
    z = -EndcapPosition.Z + 94.0 * cm;
    zor = -1;
  }
  t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y, z);
  logicEndcap = new G4LogicalVolume(solidEndcap, Air, Endcap_Name);
  if (logicEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicEndcap, false);
  physiEndcap = new G4PVPlacement(t, logicEndcap, Endcap_Name, mlv, false, 1);
  if (physiEndcap == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  EKLMVolumeTransforms.insert(std::make_pair(physiEndcap, t));
  for (i = 1; i <= nLayer; i++)
    createLayer(i, logicEndcap, &t, zor);
}

/*
 * createLayer - create layer
 * @iLayer: number of layer
 * @mlv: mother logical volume
 * @mtr: transformation from global reference frame to mother volume
 *       reference frame
 * @zOrient: z axis orientation. If > 0, first layer has the smallest z
 *           coordinate. If < 0, first layer has the largest z coordinate.
 */
void GeoEKLMBelleII::createLayer(int iLayer, G4LogicalVolume *mlv,
                                 G4Transform3D *mtr, int zOrient)
{
  int i;
  G4Tubs *solidLayer;
  G4LogicalVolume *logicLayer;
  G4PVPlacement *physiLayer;
  G4Transform3D t;
  Layer_Name = "Layer_" + lexical_cast<string>(iLayer) + "_" + Endcap_Name;
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
  LayerPosition.Z = -EndcapPosition.length / 2.0 + (iLayer + 1) *
                    Layer_shiftZ + (iLayer + 0.5) * LayerPosition.length;
  if (zOrient < 0)
    LayerPosition.Z = -LayerPosition.Z;
  t = G4Translate3D(0.0, 0.0, LayerPosition.Z);
  physiLayer = new G4PVPlacement(t, logicLayer, Layer_Name, mlv, false, 1);
  if (physiLayer == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t = t * (*mtr);
  EKLMVolumeTransforms.insert(std::make_pair(physiLayer, t));
  for (i = 1; i <= nSector; i++)
    createSector(i, logicLayer, &t, zOrient);
}

/*
 * createSector - create sector
 * @iSector: number of sector
 * @mlv: mother logical volume
 * @mtr: transformation from global reference frame to mother volume
 *       reference frame
 * @zOrient: z axis orientation of mother endcap
 */
void GeoEKLMBelleII::createSector(int iSector, G4LogicalVolume *mlv,
                                  G4Transform3D *mtr, int zOrient)
{
  int i;
  G4Tubs *solidSector;
  G4LogicalVolume *logicSector;
  G4PVPlacement *physiSector;
  G4Transform3D t;
  Sector_Name =  "Sector_" + lexical_cast<string>(iSector) + "_" + Layer_Name;
  solidSector = new G4Tubs(Sector_Name, SectorPosition.innerR,
                           SectorPosition.outerR, SectorPosition.length / 2.0,
                           90. * deg *(iSector - 1), 90. * deg * iSector);
  if (solidSector == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t = G4Translate3D(SectorPosition.X, SectorPosition.Y, SectorPosition.Z);
  logicSector = new G4LogicalVolume(solidSector, Air, Sector_Name);
  if (logicSector == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicSector, false);
  physiSector = new G4PVPlacement(t, logicSector, Sector_Name, mlv, false, 1);
  if (physiSector == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t = t * (*mtr);
  EKLMVolumeTransforms.insert(std::make_pair(physiSector, t));
  for (i = 1; i <= nPlane; i++)
    createPlane(i, iSector, logicSector, &t, zOrient);
}

/*
 * createPlane - create plane
 * @iPlane: number of plane
 * @iSector: number of sector
 * @mlv: mother logical volume
 * @mtr: transformation from global reference frame to mother volume
 *       reference frame
 * @zOrient: z axis orientation. If > 0, first plane has the smallest z
 *           coordinate. If < 0, first plane has the largest z coordinate.
 */
void GeoEKLMBelleII::createPlane(int iPlane, int iSector, G4LogicalVolume *mlv,
                                 G4Transform3D *mtr, int zOrient)
{
  int i;
  double z;
  G4Tubs *solidPlane;
  G4LogicalVolume *logicPlane;
  G4PVPlacement *physiPlane;
  G4Transform3D t;
  Plane_Name = "Plane_" + lexical_cast<string>(iPlane) + "_" + Sector_Name;
  solidPlane = new G4Tubs(Plane_Name, PlanePosition.innerR,
                          PlanePosition.outerR, PlanePosition.length / 2.0,
                          90. * deg *(iSector - 1), 90. * deg * iSector);
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
  if (iPlane == 1)
    z = PlanePosition.Z;
  else
    z = -PlanePosition.Z;
  if (zOrient < 0)
    z = -z;
  t = G4Translate3D(PlanePosition.X, PlanePosition.Y, z);
  physiPlane = new G4PVPlacement(t, logicPlane, Plane_Name, mlv, false, 1);
  if (physiPlane == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  t = t * (*mtr);
  EKLMVolumeTransforms.insert(std::make_pair(physiPlane, t));
  for (i = 1; i <= nStrip; i++)
    createStrip(i, iPlane, iSector, logicPlane, &t);
}

/*
 * createStrip - create strip
 * @iStrip: number of strip
 * @iPlane: number of plane
 * @iSector: number of sector
 * @mlv: mother logical volume
 * @mtr: transformation from global reference frame to mother volume
 *       reference frame
 */
void GeoEKLMBelleII::createStrip(int iStrip, int iPlane, int iSector,
                                 G4LogicalVolume *mlv, G4Transform3D *mtr)
{
  double x, y;
  G4Box *solidStrip;
  G4LogicalVolume *logicStrip;
  G4PVPlacement *physiStrip;
  G4Transform3D t;
  Strip_Name = "Strip_" + lexical_cast<string>(iStrip) + "_" + Plane_Name;
  solidStrip = new G4Box(Strip_Name, StripPosition[iStrip].length / 2.0,
                         Strip_width / 2.0, Strip_thickness / 2.0);
  if (solidStrip == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  logicStrip = new G4LogicalVolume(solidStrip, Polystyrene, Strip_Name);
  if (logicStrip == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  geometry::setVisibility(*logicStrip, true);
  if (iPlane == 1)
    geometry::setColor(*logicStrip, "#ffffffff");
  else
    geometry::setColor(*logicStrip, "#0000ffff");
  switch (iSector) {
    case 1:
      if (iPlane == 1) {
        x = StripPosition[iStrip].X;
        y = StripPosition[iStrip].Y;
        t = G4RotateZ3D(0.);
      } else {
        x = StripPosition[iStrip].Y;
        y = StripPosition[iStrip].X;
        t = G4RotateZ3D(90. * deg);
      }
      break;
    case 2:
      if (iPlane == 1) {
        x = -StripPosition[iStrip].X;
        y = StripPosition[iStrip].Y;
        t = G4RotateZ3D(180. * deg);
      } else {
        x = -StripPosition[iStrip].Y;
        y = StripPosition[iStrip].X;
        t = G4RotateZ3D(90. * deg);
      }
      break;
    case 3:
      if (iPlane == 1) {
        x = -StripPosition[iStrip].X;
        y = -StripPosition[iStrip].Y;
        t = G4RotateZ3D(180. * deg);
      } else {
        x = -StripPosition[iStrip].Y;
        y = -StripPosition[iStrip].X;
        t = G4RotateZ3D(270. * deg);
      }
      break;
    case 4:
      if (iPlane == 1) {
        x = StripPosition[iStrip].X;
        y = -StripPosition[iStrip].Y;
        t = G4RotateZ3D(0.);
      } else {
        x = StripPosition[iStrip].Y;
        y = -StripPosition[iStrip].X;
        t = G4RotateZ3D(270. * deg);
      }
      break;
  }

  t = G4Translate3D(x, y, 0.0) * t;
  physiStrip = new G4PVPlacement(t, logicStrip, Strip_Name, mlv, false, 1);
  if (physiStrip == NULL) {
    B2FATAL("Memory allocation error.");
    exit(ENOMEM);
  }
  EKLMVolumeTransforms.insert(std::make_pair(physiStrip, t));
}

/*
 * create - creation of the detector
 * @content: XML directory
 * @topVolume: Geant world volume
 * @type:
 */
void GeoEKLMBelleII::create(const GearDir& content, G4LogicalVolume& topVolume,
                            geometry::GeometryTypes type)
{
  int i;
  EKLM_OffsetZ  = content.getLength("OffsetZ");
  nEndcap = 2;
  nLayer = 14;
  nSector = 4;
  nPlane = 2;
  readXMLData(content);
  createMaterials();
  for (i = 1; i <= nEndcap; i++)
    createEndcap(i, &topVolume);
}
