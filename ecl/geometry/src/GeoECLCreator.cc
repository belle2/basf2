/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geometry/GeoECLCreator.h>
#include <ecl/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include "G4NistManager.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include <ecl/geometry/shapes.h>
#include <geometry/CreatorFactory.h>

#include "G4SDManager.hh"
#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  using namespace geometry;

  namespace ECL {

//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

    geometry::CreatorFactory<GeoECLCreator> GeoECLFactory("ECLCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


    GeoECLCreator::GeoECLCreator(): isBeamBkgStudy(0)
    {
      m_sensitive = new SensitiveDetector("ECLSensitiveDetector", (2 * 24)*CLHEP::eV, 10 * CLHEP::MeV);
      G4SDManager::GetSDMpointer()->AddNewDetector(m_sensitive);
      m_bkgsensitive.clear();
    }


    GeoECLCreator::~GeoECLCreator()
    {
      //      delete m_sensitive;
      for (BkgSensitiveDetector* sensitive : m_bkgsensitive) delete sensitive;
      m_bkgsensitive.clear();
    }


    void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      // Get nist material manager
      G4NistManager* nist = G4NistManager::Instance();

      G4Material* sus304 = new G4Material("SUS304", 8.00 * CLHEP::g / CLHEP::cm3, 3);
      sus304->AddElement(nist->FindOrBuildElement("Fe"), 0.74);
      sus304->AddElement(nist->FindOrBuildElement("Cr"), 0.18);
      sus304->AddElement(nist->FindOrBuildElement("Ni"), 0.08);
      cout << "SUS304 radiation length = " << sus304->GetRadlen() << " mm" << endl;

      G4Material* a5083 = new G4Material("A5083", 2.65 * CLHEP::g / CLHEP::cm3, 2);
      a5083->AddElement(nist->FindOrBuildElement("Al"), 0.955);
      a5083->AddElement(nist->FindOrBuildElement("Mg"), 0.045);
      cout << "A5083 radiation length = " << a5083->GetRadlen() << " mm" << endl;

      G4Material* a5052 = new G4Material("A5052", 2.68 * CLHEP::g / CLHEP::cm3, 2);
      a5052->AddElement(nist->FindOrBuildElement("Al"), 0.975);
      a5052->AddElement(nist->FindOrBuildElement("Mg"), 0.025);
      cout << "A5052 radiation length = " << a5052->GetRadlen() << " mm" << endl;

      G4Material* a6063 = new G4Material("A6063", 2.68 * CLHEP::g / CLHEP::cm3, 3);
      a6063->AddElement(nist->FindOrBuildElement("Al"), 0.98925);
      a6063->AddElement(nist->FindOrBuildElement("Mg"), 0.00675);
      a6063->AddElement(nist->FindOrBuildElement("Si"), 0.004);
      cout << "A6063 radiation length = " << a6063->GetRadlen() << " mm" << endl;

      G4Material* c1220 = new G4Material("C1220", 2.68 * CLHEP::g / CLHEP::cm3, 1); // c1200 contains >99.9% of copper
      c1220->AddElement(nist->FindOrBuildElement("Cu"), 1);
      cout << "Pure copper radiation length = " << c1220->GetRadlen() << " mm" << endl;

      double wrapm = 2.699 * 0.0025 + 1.4 * 0.0025 + 2.2 * 0.02;
      G4Material* medWrap = new G4Material("WRAP", wrapm / 0.025 * CLHEP::g / CLHEP::cm3, 3);
      medWrap->AddMaterial(nist->FindOrBuildMaterial("G4_Al"), 2.699 * 0.0025 / wrapm);
      medWrap->AddMaterial(nist->FindOrBuildMaterial("G4_MYLAR"), 1.4 * 0.0025 / wrapm);
      medWrap->AddMaterial(nist->FindOrBuildMaterial("G4_TEFLON"), 2.2 * 0.02 / wrapm);

      forward(content, topVolume);
      barrel(content, topVolume);
      backward(content, topVolume);
    }

    G4LogicalVolume* GeoECLCreator::wrapped_crystal(const shape_t* s, const string& endcap, double wrapthickness)
    {
      // Get nist material manager
      G4NistManager* nist = G4NistManager::Instance();

      string prefix("sv_"); prefix += endcap; prefix += "_wrap";
      G4Translate3D tw;
      G4VSolid* wrapped_crystal = s->get_solid(prefix, wrapthickness, tw);
      //  cout<<wrapped_crystal<<endl;
      string name("lv_"); name += endcap + "_wrap_" + to_string(s->nshape);
      G4LogicalVolume* wrapped_logical = new G4LogicalVolume(wrapped_crystal, nist->FindOrBuildMaterial("WRAP"), name.c_str(), 0, 0, 0);
      //  wrapLogical->SetVisAttributes(cvol);

      prefix = "sv_"; prefix += endcap; prefix += "_crystal";
      G4Translate3D tc;
      G4VSolid* crystal_solid = s->get_solid(prefix, 0, tc);
      name = "lv_" + endcap + "_crystal_" + to_string(s->nshape);
      G4LogicalVolume* crystal_logical = new G4LogicalVolume(crystal_solid, nist->FindOrBuildMaterial("G4_CESIUM_IODIDE"), name.c_str(),
                                                             0, 0, 0);
      //  crystalLogical->SetVisAttributes(cvol);
      crystal_logical->SetSensitiveDetector(m_sensitive);

      new G4PVPlacement(NULL, G4ThreeVector(), crystal_logical, name.c_str(), wrapped_logical, false, 0, 0);
      return wrapped_logical;
    }

  }//ecl
}//belle2
