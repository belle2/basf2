/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4Monopole.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;
using namespace CLHEP;

G4Monopole::G4Monopole(const G4String& name,
                       G4double  mass,
                       G4double  mCharge,
                       G4double  eCharge,
                       G4int     encoding)
  : G4ParticleDefinition(name, mass, 0.0 * MeV, eplus * eCharge,
                         1, 0, 0,
                         0, 0, 0,
                         "fermion", 0, 0, encoding,
                         true, -1.0, 0),
    // Arguments for constructor are as follows
    //                   name       mass          width         charge
    //                   2*spin     parity        C-conjugation
    //                   2*Isospin  2*Isospin3    G-parity
    //                   type       lepton_number baryon_number PDG_encoding
    //                   stable     lifetime      decay_table
    magCharge(eplus * mCharge)
{
  B2INFO("Monopole is created: m(GeV)= " << mass / GeV
         << " Qel(e+)= " << eCharge / eplus
         << " Qmag(e+)= " << magCharge / eplus);
}

G4Monopole::~G4Monopole()
{}

G4double G4Monopole::MagneticCharge() const
{
  return magCharge;
}

