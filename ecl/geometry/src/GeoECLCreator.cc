/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"

#include <ecl/geometry/GeoECLCreator.h>
#include <ecl/geometry/shapes.h>
#include <ecl/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>

namespace Belle2 {

  using namespace geometry;

  namespace ECL {

//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

    CreatorFactory<GeoECLCreator> GeoECLFactory("ECLCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


    GeoECLCreator::GeoECLCreator(): isBeamBkgStudy(0)
    {
      m_sensitive = new SensitiveDetector("ECLSensitiveDetector", (2 * 24)*CLHEP::eV, 10 * CLHEP::MeV);
      G4SDManager::GetSDMpointer()->AddNewDetector(m_sensitive);
    }


    GeoECLCreator::~GeoECLCreator()
    {
      //      delete m_sensitive;
      for (BkgSensitiveDetector* sensitive : m_bkgsensitive) delete sensitive;
      for (auto a : m_atts) delete a.second;
    }


    void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      cout << "SUS304 radiation length = " << Materials::get("SUS304")->GetRadlen() << " mm" << endl;
      cout << "A5083 radiation length = " << Materials::get("A5083")->GetRadlen() << " mm" << endl;
      cout << "A5052 radiation length = " << Materials::get("A5052")->GetRadlen() << " mm" << endl;
      cout << "A6063 radiation length = " << Materials::get("A6063")->GetRadlen() << " mm" << endl;
      cout << "Pure copper radiation length = " << Materials::get("C1220")->GetRadlen() << " mm" << endl;
      cout << "WRAP250 radiation length = " << Materials::get("WRAP250")->GetRadlen() << " mm" << endl;
      cout << "WRAP200 radiation length = " << Materials::get("WRAP200")->GetRadlen() << " mm" << endl;
      cout << "WRAP170 radiation length = " << Materials::get("WRAP170")->GetRadlen() << " mm" << endl;

      m_atts["wrap"]  = new G4VisAttributes(G4Colour(0.5, 0.5, 1.0));
      m_atts["cryst"] = new G4VisAttributes(G4Colour(0.7, 0.7, 1.0));

      m_atts["iron"]  = new G4VisAttributes(G4Colour(1., 0.1, 0.1));
      m_atts["iron2"] = new G4VisAttributes(G4Colour(1., 0.5, 0.5));
      m_atts["alum"]  = new G4VisAttributes(G4Colour(0.25, 0.25, 1.0, 0.5));
      m_atts["alum2"] = new G4VisAttributes(G4Colour(0.5, 0.5, 1.0));
      m_atts["silv"]  = new G4VisAttributes(G4Colour(0.9, 0., 0.9));
      m_atts["air"]   = new G4VisAttributes(G4Colour(1., 1., 1.)); m_atts["air"]->SetVisibility(false);
      m_atts["preamp"] = new G4VisAttributes(G4Colour(0.1, 0.1, 0.8));
      m_atts["plate"] = new G4VisAttributes(G4Colour(0.2, 0.8, 0.2));
      m_atts["connector"] = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1));
      m_atts["capacitor"] = new G4VisAttributes(G4Colour(0.1, 0.1, 0.8));
      m_atts["holder"] = new G4VisAttributes(G4Colour(0.4, 0.8, 0.8));

      forward(content, topVolume);
      barrel(content, topVolume);
      backward(content, topVolume);
    }

    G4LogicalVolume* GeoECLCreator::wrapped_crystal(const shape_t* s, const string& endcap, double wrapthickness)
    {
      string prefix("sv_"); prefix += endcap; prefix += "_wrap";
      G4Translate3D tw;
      G4VSolid* wrapped_crystal = s->get_solid(prefix, wrapthickness, tw);
      string name("lv_"); name += endcap + "_wrap_" + to_string(s->nshape);
      G4Material* wrap = NULL;
      if (wrapthickness < 0.170)
        wrap = Materials::get("WRAP170");
      else if (wrapthickness < 0.200)
        wrap = Materials::get("WRAP200");
      else
        wrap = Materials::get("WRAP250");
      G4LogicalVolume* wrapped_logical = new G4LogicalVolume(wrapped_crystal, wrap, name.c_str(), 0, 0, 0);
      wrapped_logical->SetVisAttributes(att("wrap"));

      prefix = "sv_"; prefix += endcap; prefix += "_crystal";
      G4Translate3D tc;
      G4VSolid* crystal_solid = s->get_solid(prefix, 0, tc);
      name = "lv_" + endcap + "_crystal_" + to_string(s->nshape);
      G4LogicalVolume* crystal_logical = new G4LogicalVolume(crystal_solid, Materials::get("G4_CESIUM_IODIDE"), name.c_str(),
                                                             0, 0, 0);
      crystal_logical->SetVisAttributes(att("cryst"));
      crystal_logical->SetSensitiveDetector(m_sensitive);

      new G4PVPlacement(NULL, G4ThreeVector(), crystal_logical, name.c_str(), wrapped_logical, false, 0, 0);
      return wrapped_logical;
    }

    const G4VisAttributes* GeoECLCreator::att(const string& n)
    {
      auto p = m_atts.find(n);
      if (p != m_atts.end()) return p->second;
      cout << "no such visattribute:" << n << endl;
      exit(0);
      return NULL;
    }

  }//ecl
}//belle2
