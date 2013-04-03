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

      // --- Collect global parameters
      //double GlobalRotAngle = content.getAngle("Rotation") / Unit::rad;
      double GlobalOffsetZ  = content.getLength("OffsetZ") / Unit::mm;

      double m_phi = content.getAngle("Poletip/Phi") / Unit::rad;
      double m_dphi = content.getAngle("Poletip/Dphi") / Unit::rad;

      int m_nBoundary = content.getNumberNodes("Poletip/ZBoundary");

      double m_z[m_nBoundary];
      double m_rmin[m_nBoundary];
      double m_rmax[m_nBoundary];
      for (int izBoundary  = 0; izBoundary < m_nBoundary; izBoundary++) {
        m_z[izBoundary]    = content.getLength((boost::format("Poletip/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmin[izBoundary] = content.getLength((boost::format("Poletip/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmax[izBoundary] = content.getLength((boost::format("Poletip/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }

      int m_nBoundary2 = content.getNumberNodes("Poletip2/ZBoundary");
      double m_z2[m_nBoundary2];
      double m_rmin2[m_nBoundary2];
      double m_rmax2[m_nBoundary2];
      for (int izBoundary  = 0; izBoundary < m_nBoundary2; izBoundary++) {
        m_z2[izBoundary]    = content.getLength((boost::format("Poletip2/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmin2[izBoundary] = content.getLength((boost::format("Poletip2/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmax2[izBoundary] = content.getLength((boost::format("Poletip2/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }

      int m_nBoundary3 = content.getNumberNodes("Poletip3/ZBoundary");
      double m_z3[m_nBoundary3];
      double m_rmin3[m_nBoundary3];
      double m_rmax3[m_nBoundary3];
      for (int izBoundary  = 0; izBoundary < m_nBoundary3; izBoundary++) {
        m_z3[izBoundary]    = content.getLength((boost::format("Poletip3/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmin3[izBoundary] = content.getLength((boost::format("Poletip3/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
        m_rmax3[izBoundary] = content.getLength((boost::format("Poletip3/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
      }

      //G4Polyhedra with more than 5 boundaries does not work with dawncut ???
      G4Polycone* PoleTipPcon =
        new G4Polycone("PoleTipO", m_phi, m_dphi, m_nBoundary, m_z, m_rmin, m_rmax);
      G4Polycone* PoleTipPcon2 =
        new G4Polycone("PoleTipO2", m_phi, m_dphi, m_nBoundary2, m_z2, m_rmin2, m_rmax2);
      G4Polycone* PoleTipPcon3 =
        new G4Polycone("PoleTipO3", m_phi, m_dphi, m_nBoundary3, m_z3, m_rmin3, m_rmax3);

      G4LogicalVolume* PoleTipLV =
        new G4LogicalVolume(PoleTipPcon, Materials::get("G4_Fe"), "LVPoleTip", 0, 0, 0);
      G4LogicalVolume* PoleTipLV2 =
        new G4LogicalVolume(PoleTipPcon2, Materials::get("G4_Fe"), "LVPoleTip2", 0, 0, 0);
      G4LogicalVolume* PoleTipLV3 =
        new G4LogicalVolume(PoleTipPcon3, Materials::get("G4_Fe"), "LVPoleTip3", 0, 0, 0);

      G4RotationMatrix rotation(0.0, 0.0, 0.0);
      G4ThreeVector translation(0.0, 0.0, GlobalOffsetZ);
      G4Transform3D transform3(rotation, translation);

      new G4PVPlacement(transform3, PoleTipLV, "PVPoleTip", &topVolume, false, 1);
      new G4PVPlacement(transform3, PoleTipLV2, "PVPoleTip2", &topVolume, false, 1);
      new G4PVPlacement(transform3, PoleTipLV3, "PVPoleTip3", &topVolume, false, 1);

      G4RotationMatrix rotation2(0.0, M_PI, 0.0);
      G4Transform3D transform6(rotation2, translation);

      new G4PVPlacement(transform6, PoleTipLV, "PVPoleTip", &topVolume, false, 2);
      new G4PVPlacement(transform6, PoleTipLV2, "PVPoleTip2", &topVolume, false, 2);
      new G4PVPlacement(transform6, PoleTipLV3, "PVPoleTip3", &topVolume, false, 2);
    }
  }
}
