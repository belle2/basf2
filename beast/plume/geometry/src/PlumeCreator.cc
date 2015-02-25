/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/plume/geometry/PlumeCreator.h>
#include <beast/plume/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <plume/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PLUME detector */
  namespace plume {

    // Register the creator
    /** Creator creates the PLUME geometry */
    geometry::CreatorFactory<PlumeCreator> PlumeFactory("PLUMECreator");

    PlumeCreator::PlumeCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    PlumeCreator::~PlumeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void PlumeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
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

        //create plume volume
        G4Trap* s_PLUME = new G4Trap("s_PLUME",
                                     activeParams.getLength("cDz") / 2.*CLHEP::mm ,
                                     activeParams.getLength("cDtheta") ,
                                     activeParams.getLength("cDphi") ,
                                     activeParams.getLength("cDy1") / 2.*CLHEP::mm ,
                                     activeParams.getLength("cDx2") / 2.*CLHEP::mm ,
                                     activeParams.getLength("cDx1") / 2.*CLHEP::mm , 0,
                                     activeParams.getLength("cDy2") / 2.*CLHEP::mm ,
                                     activeParams.getLength("cDx4") / 2.*CLHEP::mm ,
                                     activeParams.getLength("cDx3") / 2.*CLHEP::mm , 0);

        G4LogicalVolume* l_PLUME = new G4LogicalVolume(s_PLUME, geometry::Materials::get("PLUME"), "l_PLUME", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_PLUME->SetUserLimits(new G4UserLimits(stepSize));

        //position plume volume
        G4Transform3D theta_init = G4RotateX3D(- activeParams.getLength("cDtheta"));
        G4Transform3D phi_init = G4RotateZ3D(activeParams.getLength("k_phi_init"));
        G4Transform3D tilt_z = G4RotateY3D(activeParams.getLength("k_z_TILTED"));
        G4Transform3D tilt_phi = G4RotateZ3D(activeParams.getLength("k_phi_TILTED"));
        G4Transform3D position = G4Translate3D(activeParams.getLength("k_zC") * tan(activeParams.getLength("k_z_TILTED")) * CLHEP::cm, 0,
                                               activeParams.getLength("k_zC") * CLHEP::cm);
        G4Transform3D pos_phi = G4RotateZ3D(activeParams.getLength("k_phiC"));
        G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * phi_init * theta_init;
        //cout << "rotation  " << Tr.getRotation() << " translation " << Tr.getTranslation() << endl;

        new G4PVPlacement(Tr, l_PLUME, "p_PLUME", &topVolume, false, detID);

        detID++;
      }

      /*
        //--------- Element definition ---------
      G4int nel,natoms;
      G4double a, z,density, temperature, pressure;
      //ELEMENTS (to be differentiated from Materials) [they begin with "el"]
      //G4Element (const G4String &name, const G4String &symbol, G4double Zeff, G4double Aeff)
      G4Element* elH  = new G4Element("Hydrogen", "H", z=1.,  a= 1.01*g/mole);
      // G4Element* elC  = new G4Element("Carbon"  , "O", z=6.,  a= 12.00*g/mole);
      G4Element* elN  = new G4Element("Nitrogen", "N", z=7.,  a= 14.01*g/mole);
      G4Element* elO  = new G4Element("Oxygen"  , "O", z=8.,  a= 16.00*g/mole);
      G4Element* elSi = new G4Element("Silicon",  "Si",z=14., a= 28.01*g/mole);

      //--------- Material definition ---------
      //G4Material (const G4String &name, G4double z, G4double a, G4double density, G4State state=kStateUndefined, G4double temp=STP_Temperature, G4double pressure=STP_Pressure)
      //Air Mixture
      G4Material* Air = new G4Material("Air", density= 1.29*mg/cm3, nel=2);
      Air->AddElement(elN, 70*perCent);
      Air->AddElement(elO, 30*perCent);

      //Water (chemical compound)
      G4Material* Water = new G4Material("Water", density= 1.0*g/cm3, nel=2);
      Water->AddElement(elH,natoms=2);
      Water->AddElement(elO,natoms=1);

      G4Material* matSi = new G4Material("Silicon", z=14., a=  28.09*g/mole, density=2.3290*g/cm3);

      //Quartz (SiO2)
      G4Material* Quartz = new G4Material("quartz",2.2*g/cm3, nel=2);
      Quartz->AddElement(elSi, natoms=1);
      Quartz->AddElement(elO , natoms=2);

      //Vacuum is an "ordinary gas" with very low density->
      G4Material* vacuum = new G4Material("Vacuum", z=1., a=1.01*g/mole, density=1.e-25*g/cm3,kStateGas,temperature=0.01*kelvin, pressure=1.e-19*pascal);

      //default materials to be used in the construction. Attribution of pointers.
      //air=Air;
      //vacuo=vacuum;
      //silicon=matSi;

      G4VSolid *solidSubstrate = new G4Box("Substrate",fSubstrateLengthX/2,fSubstrateLengthY/2,fSubstrateLengthZ/2);
      G4VLogival *logicSubstrate = new G4LogicalVolume(solidSubstrate,silicon,"Substrate");
      for(int i=0;i<12;i++)
      new G4PVPlacement(&rot,fPositionSubstrate,logicSubstrate,"Substrate", logicWorld,false,0); // Basf2
      */

    }
  } // plume namespace
} // Belle2 namespace
