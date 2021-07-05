/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4mplIonisation.h>
#include <simulation/monopoles/G4mplIonisationWithDeltaModel.h>


#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Electron.hh>
#include <G4EmParameters.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;
using namespace CLHEP;

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

G4double G4mplIonisation::MinPrimaryEnergy(const G4ParticleDefinition* mpl,
                                           const G4Material*,
                                           G4double cut)
{
  G4double x = 0.5 * cut / electron_mass_c2;
  G4double mass  = mpl->GetPDGMass();
  G4double ratio = electron_mass_c2 / mass;
  G4double gam   = x * ratio + std::sqrt((1. + x) * (1. + x * ratio * ratio));
  return mass * (gam - 1.0);
}

void G4mplIonisation::InitialiseEnergyLossProcess(const G4ParticleDefinition* p,
                                                  const G4ParticleDefinition*)
{
  if (isInitialised) { return; }

  SetBaseParticle(0);

  // monopole model is responsible both for energy loss and fluctuations

  G4mplIonisationWithDeltaModel* ion =
    new G4mplIonisationWithDeltaModel(magneticCharge, "PAI");

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

  SetEmModel(ion);
  AddEmModel(1, ion, ion);

  isInitialised = true;
}

void G4mplIonisation::PrintInfo()
{}
