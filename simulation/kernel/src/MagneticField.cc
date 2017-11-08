/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/MagneticField.h>
#include <framework/geometry/BFieldManager.h>

#include <framework/gearbox/Unit.h>

#include <CLHEP/Units/PhysicalConstants.h>
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
      const B2Vector3D point = B2Vector3D{Point} * pos_conversion;
      // get the field in Geant4 units
      B2Vector3D magField = BFieldManager::getField(point) * mag_conversion;
      // and set it
      magField.GetXYZ(Bfield);
    }

  }
}
