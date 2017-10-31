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

#include <simulation/monopoles/G4mplIonisation.h>
#include <simulation/monopoles/G4mplIonisationModel.h>
#include <simulation/monopoles/G4mplIonisationWithDeltaModel.h>

#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>
#include <G4Electron.hh>
#include <G4EmParameters.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

G4mplIonisation::G4mplIonisation(G4double mCharge, const G4String& name)
  : G4VEnergyLossProcess(name),
    magneticCharge(mCharge),
    isInitialised(false)
{
  // By default 1e magnetic charge is used
  if (magneticCharge == 0.0) { magneticCharge = 1.0 * eplus; }

  SetVerboseLevel(0);
  SetProcessSubType(fIonisation);
  SetStepFunction(0.2, 1 * mm);
  SetSecondaryParticle(G4Electron::Electron());
}

G4mplIonisation::~G4mplIonisation()
{}

G4bool G4mplIonisation::IsApplicable(const G4ParticleDefinition&)
{
  return true;
}

void G4mplIonisation::InitialiseEnergyLossProcess(const G4ParticleDefinition* p,
                                                  const G4ParticleDefinition*)
{
  if (isInitialised) { return; }

  SetBaseParticle(0);

  // monopole model is responsible both for energy loss and fluctuations
  G4mplIonisationWithDeltaModel* ion =
    new G4mplIonisationWithDeltaModel(magneticCharge, "PAI");//NOTE or G4mplIonisationModel
  ion->SetParticle(p);

  // define size of dedx and range tables
  G4EmParameters* param = G4EmParameters::Instance();
  G4double emin  = std::min(param->MinKinEnergy(), ion->LowEnergyLimit());
  G4double emax  = std::max(param->MaxKinEnergy(), ion->HighEnergyLimit());
  G4int bin = G4lrint(param->NumberOfBinsPerDecade() * std::log10(emax / emin));
  ion->SetLowEnergyLimit(emin);
  ion->SetHighEnergyLimit(emax);
  SetMinKinEnergy(emin);
  SetMaxKinEnergy(emax);
  SetDEDXBinning(bin);

  AddEmModel(1, ion, ion);

  isInitialised = true;
}

void G4mplIonisation::PrintInfo()
{}
