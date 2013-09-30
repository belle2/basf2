
/**************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors:                                                         *
 *                                                                        *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 **************************************************************************/

#include <structure/geometry/GeoSTRCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4Polyhedra.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4Transform3D.hh>
#include <G4UserLimits.hh>
#include <G4VisAttributes.hh>

using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace structure {
    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoSTRCreator> GeoSTRFactory("STRCreator");
    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoSTRCreator::GeoSTRCreator()
    {

    }


    GeoSTRCreator::~GeoSTRCreator()
    {

    }

    void GeoSTRCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      GearDir cPolePieceL(content, "PolePieceL");
      double PolePieceL_minZ(0), PolePieceL_maxZ(0);
      G4Polycone* geo_PolePieceL = geometry::createPolyCone("geo_PolePieceL_name", cPolePieceL, PolePieceL_minZ, PolePieceL_maxZ);
      string strMat_PolePieceL = cPolePieceL.getString("Material", "Air");
      G4Material* mat_PolePieceL = Materials::get(strMat_PolePieceL);
      G4LogicalVolume* logi_PolePieceL = new G4LogicalVolume(geo_PolePieceL, mat_PolePieceL, "logi_PolePieceL_name");
      setColor(*logi_PolePieceL, "#CC0000");
      //setVisibility(*logi_PolePieceL, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_PolePieceL, "phys_PolePieceL", &topVolume, false, 0);

      GearDir cPolePieceR(content, "PolePieceR");
      double PolePieceR_minZ(0), PolePieceR_maxZ(0);
      G4Polycone* geo_PolePieceR = geometry::createPolyCone("geo_PolePieceR_name", cPolePieceR, PolePieceR_minZ, PolePieceR_maxZ);
      string strMat_PolePieceR = cPolePieceR.getString("Material", "Air");
      G4Material* mat_PolePieceR = Materials::get(strMat_PolePieceR);
      G4LogicalVolume* logi_PolePieceR = new G4LogicalVolume(geo_PolePieceR, mat_PolePieceR, "logi_PolePieceR_name");
      setColor(*logi_PolePieceR, "#CC0000");
      //setVisibility(*logi_PolePieceR, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_PolePieceR, "phys_PolePieceR", &topVolume, false, 0);


      /*

      double m_phi = content.getAngle("Phi") / Unit::rad;
      double m_dphi = content.getAngle("Dphi") / Unit::rad;

      int m_nBoundarySR1 = content.getNumberNodes("ShieldR1/ZBoundary");
      double m_zSR1[m_nBoundarySR1];
      double m_rminSR1[m_nBoundarySR1];
      double m_rmaxSR1[m_nBoundarySR1];
      for (int izBoundary  = 0; izBoundary < m_nBoundarySR1; izBoundary++) {
        m_zSR1[izBoundary]    = content.getLength((boost::format("ShieldR1/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rminSR1[izBoundary] = content.getLength((boost::format("ShieldR1/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmaxSR1[izBoundary] = content.getLength((boost::format("ShieldR1/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }
      int m_nBoundarySR2 = content.getNumberNodes("ShieldR2/ZBoundary");
      double m_zSR2[m_nBoundarySR2];
      double m_rminSR2[m_nBoundarySR2];
      double m_rmaxSR2[m_nBoundarySR2];
      for (int izBoundary  = 0; izBoundary < m_nBoundarySR2; izBoundary++) {
        m_zSR2[izBoundary]    = content.getLength((boost::format("ShieldR2/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rminSR2[izBoundary] = content.getLength((boost::format("ShieldR2/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmaxSR2[izBoundary] = content.getLength((boost::format("ShieldR2/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }
      int m_nBoundarySL1 = content.getNumberNodes("ShieldL1/ZBoundary");
      double m_zSL1[m_nBoundarySL1];
      double m_rminSL1[m_nBoundarySL1];
      double m_rmaxSL1[m_nBoundarySL1];
      for (int izBoundary  = 0; izBoundary < m_nBoundarySL1; izBoundary++) {
        m_zSL1[izBoundary]    = content.getLength((boost::format("ShieldL1/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rminSL1[izBoundary] = content.getLength((boost::format("ShieldL1/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmaxSL1[izBoundary] = content.getLength((boost::format("ShieldL1/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }
      int m_nBoundarySL2 = content.getNumberNodes("ShieldL2/ZBoundary");
      double m_zSL2[m_nBoundarySL2];
      double m_rminSL2[m_nBoundarySL2];
      double m_rmaxSL2[m_nBoundarySL2];
      for (int izBoundary  = 0; izBoundary < m_nBoundarySL2; izBoundary++) {
        m_zSL2[izBoundary]    = content.getLength((boost::format("ShieldL2/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rminSL2[izBoundary] = content.getLength((boost::format("ShieldL2/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmaxSL2[izBoundary] = content.getLength((boost::format("ShieldL2/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }

      G4Polycone* ShieldPconR1 =
        new G4Polycone("ShieldOR1", m_phi, m_dphi, m_nBoundarySR1, m_zSR1, m_rminSR1, m_rmaxSR1);
      G4Polycone* ShieldPconR2 =
        new G4Polycone("ShieldOR2", m_phi, m_dphi, m_nBoundarySR2, m_zSR2, m_rminSR2, m_rmaxSR2);
      G4Polycone* ShieldPconL1 =
        new G4Polycone("ShieldOL1", m_phi, m_dphi, m_nBoundarySL1, m_zSL1, m_rminSL1, m_rmaxSL1);
      G4Polycone* ShieldPconL2 =
        new G4Polycone("ShieldOR2", m_phi, m_dphi, m_nBoundarySL2, m_zSL2, m_rminSL2, m_rmaxSL2);

      G4LogicalVolume* ShieldLVR1 =
        new G4LogicalVolume(ShieldPconR1, Materials::get("G4_Pb"), "LVShieldR1", 0, 0, 0);
      //new G4LogicalVolume(ShieldPconR1, Materials::get("G4_POLYETHYLENE"), "LVShieldR1", 0, 0, 0);
      G4LogicalVolume* ShieldLVR2 =
        //new G4LogicalVolume(ShieldPconR2, Materials::get("G4_Pb"), "LVShieldR2", 0, 0, 0);
        new G4LogicalVolume(ShieldPconR2, Materials::get("G4_POLYETHYLENE"), "LVShieldR2", 0, 0, 0);
      G4LogicalVolume* ShieldLVL1 =
        new G4LogicalVolume(ShieldPconL1, Materials::get("G4_Pb"), "LVShieldL1", 0, 0, 0);
      //new G4LogicalVolume(ShieldPconL1, Materials::get("G4_POLYETHYLENE"), "LVShieldL1", 0, 0, 0);
      G4LogicalVolume* ShieldLVL2 =
        //new G4LogicalVolume(ShieldPconL2, Materials::get("G4_Pb"), "LVShieldL2", 0, 0, 0);
        new G4LogicalVolume(ShieldPconL2, Materials::get("G4_POLYETHYLENE"), "LVShieldL2", 0, 0, 0);

      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), ShieldLVR1, "PVShieldR1", &topVolume, false, 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), ShieldLVR2, "PVShieldR2", &topVolume, false, 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), ShieldLVL1, "PVShieldL1", &topVolume, false, 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), ShieldLVL2, "PVShieldL2", &topVolume, false, 0);

      */

    }
  }
}
