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

#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>

#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using namespace Belle2;

/* Register the creator */
//GeoEKLMBelleII regGeoEKLMBelleII;
geometry::CreatorFactory<GeoEKLMBelleII> GeoEKLMFactory("EKLMBelleII");

/* Constructor */
GeoEKLMBelleII::GeoEKLMBelleII()
{
//  setDescription("Creates the TGeo objects for the EKLM geometry "
//    "of the Belle II detector.");

  /* The EKLM subdetector uses the "Strip_" prefix to flag its
     sensitive volumes since all Strips are sensitive and vice versa. */
//  addSensitiveDetector("Strip_", new
//    EKLMSensetiveDetector("EKLMSensetiveDetector", (2*24)*eV, 10*MeV));
}

/* Destructor */
GeoEKLMBelleII::~GeoEKLMBelleII()
{
}

/* Creation of materials */
void GeoEKLMBelleII::createMaterials()
{
  Air = geometry::Materials::get("Air");
  Polystyrene = geometry::Materials::get("Polystyrene");
  Iron = geometry::Materials::get("Iron");
}

/* Read position data from XML database */
void GeoEKLMBelleII::readPositionData(struct EKLMElementPosition& epos,
                                      GearDir& content)
{
  epos.innerR = content.getLength("InnerR");
  epos.outerR = content.getLength("OuterR");
  epos.length = content.getLength("Length");
  epos.X = content.getLength("PositionX");
  epos.Y = content.getLength("PositionY");
  epos.Z = content.getLength("PositionZ");
}

/* Read parameters from XML database */
void GeoEKLMBelleII::readXMLData(const GearDir& content)
{
  int i, StripID;
  GearDir gd(content);
  GearDir EndCap(gd);
  gd.append("EndCap/");
  readPositionData(EndcapPosition, gd);
  EndcapPosition.Z = EKLM_OffsetZ + EndcapPosition.length / 2.0;
  GearDir Layer(gd);
  gd.append("Layer/");
  readPositionData(LayerPosition, gd);
  Layer_shiftZ = gd.getLength("ShiftZ");
  GearDir Sector(gd);
  gd.append("Sector/");
  readPositionData(SectorPosition, gd);
  GearDir Plane(gd);
  gd.append("Plane/");
  readPositionData(PlanePosition, gd);
  GearDir Strip(gd);
  gd.append("../");
  nStrip = gd.getNumberNodes("Strips/Strip");
  Strip_width  = gd.getLength("Strips/Width");
  Strip_thickness = gd.getLength("Strips/Thickness");
  StripPosition = new struct EKLMElementPosition[nStrip];
  for (i = 0; i < nStrip; i++) {
    GearDir StripContent(gd);
    StripContent.append((format("Strips/Strip[%1%]/") % (i + 1)).str());
    string sStrip = StripContent.getString("@id");
    StripID = atoi(sStrip.c_str());
    StripPosition[StripID].length = StripContent.getLength("Length");
    StripPosition[StripID].X = StripContent.getLength("PositionX");
    StripPosition[StripID].Y = StripContent.getLength("PositionY");
  }
}

/* Create endcap */
void GeoEKLMBelleII::createEndcap(int iEndcap, G4LogicalVolume *mother)
{
  int i;
  G4Tubs *solidEndcap;
  G4LogicalVolume *logicEndcap;
  G4PVPlacement *physiEndcap;
  G4Transform3D t;
  Endcap_Name = "Endcap_" + lexical_cast<string>(iEndcap);
  solidEndcap = new G4Tubs(Endcap_Name, EndcapPosition.innerR,
                           EndcapPosition.outerR, EndcapPosition.length / 2, 0. * deg, 360. * deg);
  logicEndcap = new G4LogicalVolume(solidEndcap, Air, Endcap_Name);
  if (iEndcap == 0) {
    t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y,
                      EndcapPosition.Z);
  } else {
    t = G4ReflectZ3D(1) * G4Translate3D(EndcapPosition.X, EndcapPosition.Y,
                                        -EndcapPosition.Z + 47.0 * 2.);
  }
  physiEndcap = new G4PVPlacement(t, logicEndcap, Endcap_Name, mother, false,
                                  1);
  for (i = 0; i < nLayer; i++) {
    createLayer(i, logicEndcap);
  }
}

/* Create layer */
void GeoEKLMBelleII::createLayer(int iLayer, G4LogicalVolume *mother)
{
  int i;
  G4Tubs *solidLayer;
  G4LogicalVolume *logicLayer;
  G4PVPlacement *physiLayer;
  G4Transform3D t;
  Layer_Name = "Layer_" + lexical_cast<string>(iLayer) + "_" + Endcap_Name;
  solidLayer = new G4Tubs(Layer_Name, LayerPosition.innerR,
                          LayerPosition.outerR, LayerPosition.length / 2.0, 0. * deg, 360. * deg);
  logicLayer = new G4LogicalVolume(solidLayer, Air, Layer_Name);
  LayerPosition.Z  = -EndcapPosition.length / 2.0 + (iLayer + 1) *
                     Layer_shiftZ + (iLayer + 0.5) *  LayerPosition.length;
  t = G4Translate3D(0.0, 0.0, LayerPosition.Z);
  physiLayer = new G4PVPlacement(t, logicLayer, Layer_Name, mother, false, 1);
  for (i = 0; i < nSector; i++) {
    createSector(i, logicLayer);
  }
}

/* Create sector */
void GeoEKLMBelleII::createSector(int iSector, G4LogicalVolume *mother)
{
  int i;
  G4Tubs *solidSector;
  G4LogicalVolume *logicSector;
  G4PVPlacement *physiSector;
  G4Transform3D t;
  Sector_Name =  "Sector_" + lexical_cast<string>(iSector) + "_" + Layer_Name;
  solidSector = new G4Tubs(Sector_Name, SectorPosition.innerR,
                           SectorPosition.outerR, SectorPosition.length / 2.0, 0. * deg, 90. * deg);
  logicSector = new G4LogicalVolume(solidSector, Air, Sector_Name);
  switch (iSector) {
    case 1:
      t = G4ReflectX3D(1);
      break;
    case 2:
      t = G4ReflectX3D(1) * G4ReflectY3D(1);
      break;
    case 3:
      t = G4ReflectY3D(1);
      break;
  }
  t = t * G4Translate3D(SectorPosition.X, SectorPosition.Y, SectorPosition.Z);
  physiSector = new G4PVPlacement(t, logicSector, Sector_Name, mother, false,
                                  1);
  for (i = 0; i < nSector; i++) {
    createPlane(i, logicSector);
  }
}

/* Create plane */
void GeoEKLMBelleII::createPlane(int iPlane, G4LogicalVolume *mother)
{
  int i;
  G4Tubs *solidPlane;
  G4LogicalVolume *logicPlane;
  G4PVPlacement *physiPlane;
  G4Transform3D t;
  Plane_Name = "Plane_" + lexical_cast<string>(iPlane) + "_" + Sector_Name;
  solidPlane = new G4Tubs(Plane_Name, PlanePosition.innerR,
                          PlanePosition.outerR, PlanePosition.length / 2.0, 0. * deg, 90. * deg);
  logicPlane = new G4LogicalVolume(solidPlane, Air, Plane_Name);
  if (iPlane == 0)
    t = G4Translate3D(SectorPosition.X, SectorPosition.Y, SectorPosition.Z);
  else
    t = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) *
        G4Translate3D(SectorPosition.X, SectorPosition.Y, -SectorPosition.Z);
  physiPlane = new G4PVPlacement(t, logicPlane, Plane_Name, mother, false, 1);
  for (i = 0; i < nSector; i++) {
    createStrip(i, logicPlane);
  }
}

/* Create strip */
void GeoEKLMBelleII::createStrip(int iStrip, G4LogicalVolume *mother)
{
  G4Box *solidStrip;
  G4LogicalVolume *logicStrip;
  G4PVPlacement *physiStrip;
  G4Transform3D t;
  Strip_Name = "Strip_" + lexical_cast<string>(iStrip) + "_" + Plane_Name;
  solidStrip = new G4Box(Strip_Name, StripPosition[iStrip].length / 2.0,
                         Strip_width / 2.0, Strip_thickness / 2.0);
  logicStrip = new G4LogicalVolume(solidStrip, Polystyrene, Strip_Name);
  t = G4Translate3D(StripPosition[iStrip].X, StripPosition[iStrip].Y, 0.0);
  physiStrip = new G4PVPlacement(t, logicStrip, Strip_Name, mother, false, 1);
}

/* Creation of the detector */
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
  for (i = 0; i < 2; i++) {
    createEndcap(i, &topVolume);
  }
}
