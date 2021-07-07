/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/cave/geometry/CaveCreator.h>
#include <beast/cave/simulation/SensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4UserLimits.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CAVE detector */
  namespace cave {

    // Register the creator
    /** Creator creates the cave geometry */
    geometry::CreatorFactory<CaveCreator> CaveFactory("CAVECreator");

    CaveCreator::CaveCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    CaveCreator::~CaveCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void CaveCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      G4double density;
      G4double A;
      G4int Z;

      G4String name, symbol;
      G4double fractionmass;

      A = 1.01 * CLHEP::g / CLHEP::mole;
      G4Element* elH  = new G4Element(name = "Hydrogen", symbol = "H" , Z = 1, A);

      A = 12.01 * CLHEP::g / CLHEP::mole;
      G4Element* elC  = new G4Element(name = "Carbon"  , symbol = "C" , Z = 6, A);

      A = 16.00 * CLHEP::g / CLHEP::mole;
      G4Element* elO  = new G4Element(name = "Oxygen"  , symbol = "O" , Z = 8, A);

      A = 22.99 * CLHEP::g / CLHEP::mole;
      G4Element* elNa  = new G4Element(name = "Natrium"  , symbol = "Na" , Z = 11 , A);

      A = 200.59 * CLHEP::g / CLHEP::mole;
      G4Element* elHg  = new G4Element(name = "Hg"  , symbol = "Hg" , Z = 80, A);

      A = 26.98 * CLHEP::g / CLHEP::mole;
      G4Element* elAl  = new G4Element(name = "Aluminium"  , symbol = "Al" , Z = 13, A);

      A = 28.09 * CLHEP::g / CLHEP::mole;
      G4Element* elSi  = new G4Element(name = "Silicon", symbol = "Si", Z = 14, A);

      A = 39.1 * CLHEP::g / CLHEP::mole;
      G4Element* elK  = new G4Element(name = "K"  , symbol = "K" , Z = 19 , A);

      A = 69.72 * CLHEP::g / CLHEP::mole;
      G4Element* elCa  = new G4Element(name = "Calzium"  , symbol = "Ca" , Z = 31 , A);

      A = 55.85 * CLHEP::g / CLHEP::mole;
      G4Element* elFe = new G4Element(name = "Iron"    , symbol = "Fe", Z = 26, A);

      density = 2.03 * CLHEP::g / CLHEP::cm3;
      G4Material* Concrete = new G4Material("Concrete", density, 10);
      Concrete->AddElement(elH , fractionmass = 0.01);
      Concrete->AddElement(elO , fractionmass = 0.529);
      Concrete->AddElement(elNa , fractionmass = 0.016);
      Concrete->AddElement(elHg , fractionmass = 0.002);
      Concrete->AddElement(elAl , fractionmass = 0.034);
      Concrete->AddElement(elSi , fractionmass = 0.337);
      Concrete->AddElement(elK , fractionmass = 0.013);
      Concrete->AddElement(elCa , fractionmass = 0.044);
      Concrete->AddElement(elFe , fractionmass = 0.014);
      Concrete->AddElement(elC , fractionmass = 0.001);


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

        //create cave volume
        G4Box* s_CAVE = new G4Box("s_CAVE",
                                  activeParams.getLength("px")*CLHEP::cm ,
                                  activeParams.getLength("py")*CLHEP::cm ,
                                  activeParams.getLength("pz")*CLHEP::cm);

        //G4LogicalVolume* l_CAVE = new G4LogicalVolume(s_CAVE, geometry::Materials::get("CAVE"), "l_CAVE", 0, m_sensitive);
        G4LogicalVolume* l_CAVE = new G4LogicalVolume(s_CAVE, Concrete, "l_CAVE", 0, 0);

        //Lets limit the Geant4 stepsize inside the volume
        l_CAVE->SetUserLimits(new G4UserLimits(stepSize));

        //position cave volume
        G4ThreeVector CAVEpos = G4ThreeVector(
                                  activeParams.getLength("x_cave") * CLHEP::cm,
                                  activeParams.getLength("y_cave") * CLHEP::cm,
                                  activeParams.getLength("z_cave") * CLHEP::cm
                                );

        G4RotationMatrix* rot_cave = new G4RotationMatrix();
        rot_cave->rotateX(activeParams.getLength("AngleX"));
        rot_cave->rotateY(activeParams.getLength("AngleY"));
        rot_cave->rotateZ(activeParams.getLength("AngleZ"));
        //geometry::setColor(*l_CAVE, "#006699");
        //double angle = activeParams.getDouble("angle");
        //double rx = activeParams.getDouble("rx");
        //double ry = activeParams.getDouble("ry");
        //double rz = activeParams.getDouble("rz");
        //rot_cave->rotate(-angle, G4ThreeVector(rx, ry, rz));
        new G4PVPlacement(rot_cave, CAVEpos, l_CAVE, "p_CAVE", &topVolume, false, detID);

        detID++;
      }
    }
  } // cave namespace
} // Belle2 namespace
