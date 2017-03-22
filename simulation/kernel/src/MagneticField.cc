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

#include <framework/gearbox/Unit.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <globals.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


MagneticField::MagneticField(): G4MagneticField(), m_bField(BFieldMap::Instance())
{

}


MagneticField::~MagneticField()
{

}


void MagneticField::GetFieldValue(const G4double Point[3], G4double* Bfield) const
{
  //Get the magnetic field vector from the central magnetic field map (Geant4 uses [mm] as a length unit)
  B2Vector3D magField = m_bField.getBField(B2Vector3D(Point[0] * Unit::mm, Point[1] * Unit::mm, Point[2] * Unit::mm));

  //Set the magnetic field (Use the Geant4 tesla unit here !)
  Bfield[0] = magField[0] * CLHEP::tesla;
  Bfield[1] = magField[1] * CLHEP::tesla;
  Bfield[2] = magField[2] * CLHEP::tesla;
}
