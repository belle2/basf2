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

#include <structure/geometry/GeoCOILCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4Transform3D.hh>
#include <G4UserLimits.hh>
#include <G4VisAttributes.hh>

#include <iostream>

/* Geant4 default unit
                millimeter              (mm)
                nanosecond              (ns)
                Mega electron Volt      (MeV)
                positron charge         (eplus)
                degree Kelvin           (kelvin)
                the amount of substance (mole)
                luminous intensity      (candela)
                radian                  (radian)
                steradian               (steradian)
*/


using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace coil {
    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoCOILCreator> GeoCOILFactory("COILCreator");
    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoCOILCreator::GeoCOILCreator()
    {
      m_VisAttributes.clear();
      m_VisAttributes.push_back(new G4VisAttributes(false)); // for "invisible"
    }


    GeoCOILCreator::~GeoCOILCreator()
    {
      for (G4VisAttributes* visAttr : m_VisAttributes) delete visAttr;
      m_VisAttributes.clear();
    }

    // Write COILGeometryPar object from GearBox content
    void GeoCOILCreator::read(const GearDir& content, COILGeometryPar& parameters, std::string item)
    {

      // check if item is
      // Cavity1
      // Cavity2
      // Coil
      // Cryostat
      // call the right setters to write the parameters
    }

    // Create geometry from COILGeometryPar object
    void GeoCOILCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {


      // Global parameters
      //////////////////////
      double GlobalRotAngle = content.getAngle("Rotation") / Unit::rad;
      double GlobalOffsetZ  = content.getLength("OffsetZ") / Unit::mm;


      // Cryostat
      /////////////

      string strMatCryo = content.getString("Cryostat/Material", "Air");
      G4Material* matCryostat = Materials::get(strMatCryo);

      double CryoRmin   = content.getLength("Cryostat/Rmin") / Unit::mm;
      double CryoRmax   = content.getLength("Cryostat/Rmax") / Unit::mm;
      double CryoLength = content.getLength("Cryostat/HalfLength") / Unit::mm;

      G4Tubs* CryoTube =
        new G4Tubs("Cryostat", CryoRmin, CryoRmax, CryoLength, 0.0, M_PI * 2.0);
      G4LogicalVolume* CryoLV =
        new G4LogicalVolume(CryoTube, matCryostat, "LVCryo", 0, 0, 0);
      new G4PVPlacement(G4TranslateZ3D(GlobalOffsetZ) * G4RotateZ3D(GlobalRotAngle),
                        CryoLV, "PVCryo", &topVolume, false, 0);


      // Cavity #1
      //////////////

      string strMatCryo = content.getString("Cryostat/Material", "Air");
      G4Material* matCryostat = Materials::get(strMatCryo);

      double Cav1Rmin   = content.getLength("Cavity1/Rmin") / Unit::mm;
      double Cav1Rmax   = content.getLength("Cavity1/Rmax") / Unit::mm;
      double Cav1Length = content.getLength("Cavity1/HalfLength") / Unit::mm;

      G4Tubs* Cav1Tube =
        new G4Tubs("Cavity1", Cav1Rmin, Cav1Rmax, Cav1Length, 0.0, M_PI * 2.0);
      G4LogicalVolume* Cav1LV =
        new G4LogicalVolume(Cav1Tube, Materials::get("G4_AIR"), "LVCav1", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(0., 1., 0.)));
      Cav1LV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), Cav1LV, "PVCav1", CryoLV, false, 0);


      // Rad Shield
      ///////////////

      double ShieldRmin   = content.getLength("RadShield/Rmin") / Unit::mm;
      double ShieldRmax   = content.getLength("RadShield/Rmax") / Unit::mm;
      double ShieldLength = content.getLength("RadShield/HalfLength") / Unit::mm;

      G4Tubs* ShieldTube =
        new G4Tubs("RadShield", ShieldRmin, ShieldRmax,  ShieldLength, 0.0, M_PI * 2.0);
      G4LogicalVolume* ShieldLV =
        new G4LogicalVolume(ShieldTube, Materials::get("G4_Al"), "LVShield", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(1., 1., 0.)));
      ShieldLV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), ShieldLV, "PVShield", Cav1LV, false, 0);


      // Cavity #2
      /////////////

      double Cav2Rmin   = content.getLength("Cavity2/Rmin") / Unit::mm;
      double Cav2Rmax   = content.getLength("Cavity2/Rmax") / Unit::mm;
      double Cav2Length = content.getLength("Cavity2/HalfLength") / Unit::mm;

      G4Tubs* Cav2Tube =
        new G4Tubs("Cavity2", Cav2Rmin, Cav2Rmax, Cav2Length, 0.0, M_PI * 2.0);
      G4LogicalVolume* Cav2LV =
        new G4LogicalVolume(Cav2Tube, Materials::get("G4_AIR"), "LVCav2", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(1., 1., 1.)));
      Cav2LV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), Cav2LV, "PVCav2", ShieldLV, false, 0);


      // Coil
      /////////

      double CoilRmin   = content.getLength("Coil/Rmin") / Unit::mm;
      double CoilRmax   = content.getLength("Coil/Rmax") / Unit::mm;
      double CoilLength = content.getLength("Coil/HalfLength") / Unit::mm;

      G4Tubs* CoilTube =
        new G4Tubs("Coil", CoilRmin, CoilRmax, CoilLength, 0.0, M_PI * 2.0);
      G4LogicalVolume* CoilLV =
        new G4LogicalVolume(CoilTube, Materials::get("G4_Al"), "LVCoil", 0, 0, 0);
      m_VisAttributes.push_back(new G4VisAttributes(G4Colour(0., 1., 1.)));
      CoilLV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), CoilLV, "PVCoil", Cav2LV, false, 0);

    }
  }
}

