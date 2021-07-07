/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/bgo/geometry/BgoCreator.h>
#include <beast/bgo/simulation/SensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trap.hh>
#include <G4UserLimits.hh>
#include "G4NistManager.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the BGO detector */
  namespace bgo {

    // Register the creator
    /** Creator creates the BGO geometry */
    geometry::CreatorFactory<BgoCreator> BgoFactory("BGOCreator");

    BgoCreator::BgoCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    BgoCreator::~BgoCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void BgoCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      // **Materials from the NIST database**
      G4NistManager* man = G4NistManager::Instance();

      G4bool isotopes = false;

      G4Element*  O = man->FindOrBuildElement("O" , isotopes);
      G4Element* Bi = man->FindOrBuildElement("Bi", isotopes);
      G4Element* Ge = man->FindOrBuildElement("Ge", isotopes);

      G4Material* BGO_BGO = new G4Material("BGO_BGO",//name
                                           7.13 * CLHEP::g / CLHEP::cm3, //density
                                           3);//number of elements
      BGO_BGO->AddElement(O, 12);
      BGO_BGO->AddElement(Bi, 4);
      BGO_BGO->AddElement(Ge, 3);

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create bgo volume
        G4Trap* s_BGO = new G4Trap("s_BGO",
                                   activeParams.getLength("cDz") / 2.*CLHEP::cm ,
                                   activeParams.getLength("cDtheta") ,
                                   activeParams.getLength("cDphi") ,
                                   activeParams.getLength("cDy1") / 2.*CLHEP::cm ,
                                   activeParams.getLength("cDx2") / 2.*CLHEP::cm ,
                                   activeParams.getLength("cDx1") / 2.*CLHEP::cm , 0,
                                   activeParams.getLength("cDy2") / 2.*CLHEP::cm ,
                                   activeParams.getLength("cDx4") / 2.*CLHEP::cm ,
                                   activeParams.getLength("cDx3") / 2.*CLHEP::cm , 0);

        //G4LogicalVolume* l_BGO = new G4LogicalVolume(s_BGO, geometry::Materials::get("BGO"), "l_BGO", 0, m_sensitive);
        G4LogicalVolume* l_BGO = new G4LogicalVolume(s_BGO, BGO_BGO, "l_BGO", 0, m_sensitive);

        //cout << "BGO volume " << s_BGO->GetCubicVolume() / CLHEP::cm / CLHEP::cm / CLHEP::cm
        //<< " density " << geometry::Materials::get("BGO")->GetDensity() / CLHEP::g * CLHEP::cm * CLHEP::cm * CLHEP::cm << endl;

        //Lets limit the Geant4 stepsize inside the volume
        l_BGO->SetUserLimits(new G4UserLimits(stepSize));

        //position bgo volume
        /*
        G4Transform3D theta_init = G4RotateX3D(- activeParams.getLength("cDtheta"));
        G4Transform3D phi_init = G4RotateZ3D(activeParams.getLength("k_phi_init"));
        G4Transform3D tilt_z = G4RotateY3D(activeParams.getLength("k_z_TILTED"));
        G4Transform3D tilt_phi = G4RotateZ3D(activeParams.getLength("k_phi_TILTED"));
        G4Transform3D position = G4Translate3D(activeParams.getLength("k_zC") * tan(activeParams.getLength("k_z_TILTED")) * CLHEP::cm, 0,
                                               activeParams.getLength("k_zC") * CLHEP::cm);
        G4Transform3D pos_phi = G4RotateZ3D(activeParams.getLength("k_phiC"));
        G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * phi_init * theta_init;
        //cout << "rotation  " << Tr.getRotation() << " translation " << Tr.getTranslation() << endl;
        */
        double px = activeParams.getDouble("px");
        double py = activeParams.getDouble("py");
        double pz = activeParams.getDouble("pz");
        double angle = activeParams.getDouble("angle");
        double rx = activeParams.getDouble("rx");
        double ry = activeParams.getDouble("ry");
        double rz = activeParams.getDouble("rz");

        G4RotationMatrix* pRot = new G4RotationMatrix();
        pRot->rotate(-angle, G4ThreeVector(rx, ry, rz));
        //G4Transform3D transform = G4Translate3D(px, py, pz) * G4Rotate3D(-angle, G4ThreeVector(rx, ry, rz));
        new G4PVPlacement(pRot, G4ThreeVector(px, py, pz), l_BGO, "p_BGO", &topVolume, false, detID);
        //new G4PVPlacement(transform, l_BGO, "p_BGO", &topVolume, false, detID);
        B2INFO("BGO-" << detID << " placed at: (" << px << "," << py << "," << pz << ")" << " mm ");
        //B2INFO("BGO-" << detID << " placed at: " << transform.getTranslation() << " mm ");
        B2INFO("      rotation of " << -angle << " degree a long (" << rx << "," << ry << "," << rz << ") axis");
        detID++;
      }
    }
  } // bgo namespace
} // Belle2 namespace
