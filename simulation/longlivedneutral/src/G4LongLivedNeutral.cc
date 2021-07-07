/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from BelleII monopole simulation

#include <simulation/longlivedneutral/G4LongLivedNeutral.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CLHEP;

G4LongLivedNeutral::G4LongLivedNeutral(const G4String& name)
//                                        G4double  mass,
//                                        G4int     encoding)
  : G4ParticleDefinition(name, 0, 0.0 * MeV, 0,
                         0, 0, 0,
                         0, 0, 0,
                         "scalar", 0.0, 0, 0,
                         false, 0, 0)
//Arguments for constructor are as follows
// name       mass          width         charge
// 2*spin     parity        C-conjugation
// 2*Isospin  2*Isospin3    G-parity
// type       lepton_number baryon_number PDG_encoding
// stable     lifetime      decay_table
{

  B2DEBUG(0, "neutral long-lived particle created: m(MeV)= " << this->GetPDGMass());

}

G4LongLivedNeutral::~G4LongLivedNeutral()
{}
