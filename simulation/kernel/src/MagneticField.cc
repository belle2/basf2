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
#include <framework/logging/Logger.h>

#include <globals.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


MagneticField::MagneticField(): G4MagneticField()
{

}


MagneticField::~MagneticField()
{

}


void MagneticField::GetFieldValue(const G4double Point[3], G4double *Bfield) const
{
  //const 1.5 Tesla magnetic field in z direction. Hard coded. Sorry.
  Bfield[0] = 0;
  Bfield[1] = 0;
  Bfield[2] = 1.5 * tesla; //Use the Geant4 tesla unit here !
}
