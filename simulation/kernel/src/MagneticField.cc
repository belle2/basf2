/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/MagneticField.h>
#include <framework/geometry/BFieldManager.h>

#include <framework/gearbox/Unit.h>

#include <CLHEP/Units/SystemOfUnits.h>

#include <globals.hh>

namespace Belle2 {
  namespace Simulation {

    MagneticField::MagneticField(): G4MagneticField()
    {
    }

    MagneticField::~MagneticField()
    {
    }

    void MagneticField::GetFieldValue(const G4double Point[3], G4double* Bfield) const
    {
      static const double pos_conversion{Unit::mm / CLHEP::mm};
      static const double mag_conversion{CLHEP::tesla / Unit::T};
      //Get the magnetic field vector from the central magnetic field map (Geant4 uses [mm] as a length unit)
      const ROOT::Math::XYZVector point = ROOT::Math::XYZVector(Point[0], Point[1], Point[2]) * pos_conversion;
      // get the field in Geant4 units
      ROOT::Math::XYZVector magField = BFieldManager::getField(point) * mag_conversion;
      // and set it
      magField.GetCoordinates(Bfield);
    }

  }
}
