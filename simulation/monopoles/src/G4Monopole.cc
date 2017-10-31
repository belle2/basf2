//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// modified from exoticphysics/monopole/*

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/monopoles/G4Monopole.h>

#include <G4ParticleTable.hh>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//
//    Arguments for constructor are as follows
//               name             mass          width         charge
//             2*spin           parity  C-conjugation
//          2*Isospin       2*Isospin3       G-parity
//               type    lepton number  baryon number   PDG encoding
//             stable         lifetime    decay table
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4Monopole::G4Monopole(const G4String& aName,
                       G4double  mass,
                       G4double  mCharge,
                       G4double  eCharge,
                       G4int     encoding)
  : G4ParticleDefinition(aName, mass, 0.0 * MeV, eplus * eCharge, 0, 0,
                         0, 0, 0, 0, "fermion",
                         0, 0, encoding, true, -1.0, 0)
{
  magCharge = eplus * mCharge;
  G4cout << "Monopole is created: m(GeV)= " << mass / GeV
         << " Qel= " << eCharge / eplus
         << " Qmag= " << magCharge / eplus
         << G4endl;
}

G4Monopole::~G4Monopole()
{}

G4double G4Monopole::MagneticCharge() const
{
  return magCharge;
}

