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
#include "G4UserLimits.hh"
#include <G4VisAttributes.hh>
#include <G4Box.hh>

#include <ecl/geometry/GeoECLCreator.h>
#include <ecl/geometry/shapes.h>
#include <ecl/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <framework/gearbox/GearDir.h>
#include "ecl/dbobjects/ECLCrystalsShapeAndPosition.h"
#include "ecl/dbobjects/ECLBeamBackgroundStudy.h"
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace Belle2::geometry;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the GeoCreator
//-----------------------------------------------------------------

CreatorFactory<GeoECLCreator> GeoECLFactory("ECLCreator");

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


GeoECLCreator::GeoECLCreator(): m_sap(0), m_overlap(0)
{
  m_sensediode = NULL;
  m_sensitive  = new SensitiveDetector("ECLSensitiveDetector", (2 * 24)*CLHEP::eV, 10 * CLHEP::MeV);
  G4SDManager::GetSDMpointer()->AddNewDetector(m_sensitive);
  defineVisAttributes();
}


GeoECLCreator::~GeoECLCreator()
{
  for (auto a : m_atts) delete a.second;
}

void GeoECLCreator::createFromDB(const std::string&, G4LogicalVolume& topVolume, geometry::GeometryTypes)
{
  DBObjPtr<ECLBeamBackgroundStudy> study;
  if (study.isValid() && (*study).getBeamBackgroundStudy()) {
    m_sensediode = new BkgSensitiveDiode("ECLBkgSensitiveDiode");
  } else {
    m_sensediode = new SensitiveDiode("ECLSensitiveDiode");
    G4SDManager::GetSDMpointer()->AddNewDetector(m_sensediode);
  }

  DBObjPtr<ECLCrystalsShapeAndPosition> crystals;
  if (!crystals.isValid()) B2FATAL("No crystal's data in the database.");
  m_sap = &(*crystals);

  forward(topVolume);
  barrel(topVolume);
  backward(topVolume);
}

void GeoECLCreator::createPayloads(const GearDir& content, const IntervalOfValidity& iov)
{
  ECLCrystalsShapeAndPosition crystals = loadCrystalsShapeAndPosition();
  Database::Instance().storeData<ECLCrystalsShapeAndPosition>(&crystals, iov);

  ECLBeamBackgroundStudy study;
  study.setBeamBackgroundStudy(content.getInt("BeamBackgroundStudy"));
  Database::Instance().storeData<ECLBeamBackgroundStudy>(&study, iov);
}

void GeoECLCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
{
  bool isBeamBkgStudy = content.getInt("BeamBackgroundStudy");
  if (isBeamBkgStudy) {
    m_sensediode = new BkgSensitiveDiode("ECLBkgSensitiveDiode");
  } else {
    m_sensediode = new SensitiveDiode("ECLSensitiveDiode");
    G4SDManager::GetSDMpointer()->AddNewDetector(m_sensediode);
  }

  ECLCrystalsShapeAndPosition crystals = loadCrystalsShapeAndPosition();
  m_sap = &crystals;

  forward(topVolume);
  barrel(topVolume);
  backward(topVolume);
}

G4LogicalVolume* GeoECLCreator::wrapped_crystal(const shape_t* s, const std::string& endcap, double wrapthickness)
{
  std::string prefix("sv_"); prefix += endcap; prefix += "_wrap";
  G4Translate3D tw;
  G4VSolid* wrapped_crystal = s->get_solid(prefix, wrapthickness, tw);
  std::string name("lv_"); name += endcap + "_wrap_" + std::to_string(s->nshape);
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
  name = "lv_" + endcap + "_crystal_" + std::to_string(s->nshape);
  G4LogicalVolume* crystal_logical = new G4LogicalVolume(crystal_solid, Materials::get("G4_CESIUM_IODIDE"), name.c_str(),
                                                         0, 0, 0);
  crystal_logical->SetVisAttributes(att("cryst"));
  crystal_logical->SetSensitiveDetector(m_sensitive);

  new G4PVPlacement(NULL, G4ThreeVector(), crystal_logical, name.c_str(), wrapped_logical, false, 0, 0);
  return wrapped_logical;
}

void GeoECLCreator::defineVisAttributes()
{
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
}

const G4VisAttributes* GeoECLCreator::att(const std::string& n) const
{
  auto p = m_atts.find(n);
  assert(p != m_atts.end());
  return p->second;
}

G4LogicalVolume* GeoECLCreator::get_preamp() const
{
  static G4LogicalVolume* lv_preamplifier = NULL;
  if (lv_preamplifier == NULL) {
    G4VSolid* sv_preamplifier = new G4Box("sv_preamplifier", 58. / 2, 51. / 2, get_pa_box_height() / 2);
    lv_preamplifier = new G4LogicalVolume(sv_preamplifier, Materials::get("A5052"), "lv_preamplifier", 0, 0, 0);
    G4VSolid* sv_diode = new G4Box("sv_diode", 20. / 2, 20. / 2, 0.3 / 2);
    G4LogicalVolume* lv_diode = new G4LogicalVolume(sv_diode, Materials::get("G4_Si"), "lv_diode", 0, 0, 0);
    lv_diode->SetUserLimits(new G4UserLimits(0.01));
    lv_diode->SetSensitiveDetector(m_sensediode);
    new G4PVPlacement(G4TranslateZ3D(-get_pa_box_height() / 2 + 0.3 / 2), lv_diode, "pv_diode", lv_preamplifier, false, 0, m_overlap);

    // G4VSolid* sv_diode = new G4Box("sv_diode", 10. / 2, 20. / 2, 0.3 / 2);
    // G4LogicalVolume* lv_diode = new G4LogicalVolume(sv_diode, Materials::get("G4_Si"), "lv_diode", 0, 0, 0);
    // lv_diode->SetUserLimits(new G4UserLimits(0.01));
    // new G4PVPlacement(G4Translate3D(-5, 0, -pa_box_height / 2 + 0.3 / 2), lv_diode, "pv_diode1", lv_preamplifier, false, 1, overlap);
    // new G4PVPlacement(G4Translate3D(5, 0, -pa_box_height / 2 + 0.3 / 2), lv_diode, "pv_diode2", lv_preamplifier, false, 2, overlap);

    lv_preamplifier->SetVisAttributes(att("preamp"));
  }
  return lv_preamplifier;
}

