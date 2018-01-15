/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopolePhysics.h>
#include <simulation/monopoles/G4MonopolePhysicsMessenger.h>
#include <simulation/monopoles/G4mplIonisation.h>
#include <simulation/monopoles/G4mplIonisationWithDeltaModel.h>
#include <simulation/monopoles/G4Monopole.h>
#include <simulation/monopoles/G4MonopoleTransportation.h>

#include <G4ParticleDefinition.hh>
#include <G4ProcessManager.hh>
#include <G4ProcessVector.hh>
#include <G4StepLimiter.hh>
#include <G4Transportation.hh>
#include <G4hMultipleScattering.hh>
#include <G4hhIonisation.hh>
#include <G4hIonisation.hh>
#include <G4PhysicsListHelper.hh>
#include <G4BuilderType.hh>
#include <G4SystemOfUnits.hh>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

G4MonopolePhysics::G4MonopolePhysics(const G4String& nam)
  : G4VPhysicsConstructor(nam),
    fMessenger(0), fMpl(0), fApl(0)
{
  fMagCharge = 1.0;
  fElCharge  = 0.0;
  fMonopoleMass = 4.5 * GeV;
  fMessenger = new G4MonopolePhysicsMessenger(this);
  SetPhysicsType(bUnknown);
}

G4MonopolePhysics::~G4MonopolePhysics()
{
  delete fMessenger;
}

void G4MonopolePhysics::ConstructParticle()
{
  fMpl = new G4Monopole("monopole",      fMonopoleMass,  fMagCharge,  fElCharge,  99666);
//NOTE careful not to use same name or encoding, this will lead to G4exception
  fApl = new G4Monopole("anti-monopole", fMonopoleMass, -fMagCharge, -fElCharge, -99666);
}


void G4MonopolePhysics::ConstructProcess()
{
  if (verboseLevel > 0) {
    G4cout << "G4MonopolePhysics::ConstructProcess" << G4endl;
  }

  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
  G4ProcessManager* pmanager[2] = {fMpl->GetProcessManager(), fApl->GetProcessManager()};

  // defined monopole parameters and binning

  G4double magn = fMpl->MagneticCharge();
  G4double elec = fMpl->GetPDGCharge();
  G4double emin = fMonopoleMass / 20000.;
  if (emin < keV) { emin = keV; }
  G4double emax = std::max(10.*TeV, fMonopoleMass * 100);
  G4int nbin = G4lrint(10 * std::log10(emax / emin));

  // dedicated trasporation
  if (magn != 0.0) {
    pmanager[0]->RemoveProcess(0);
    pmanager[0]->AddProcess(new G4MonopoleTransportation(fMpl), -1, 0, 0);
    pmanager[1]->RemoveProcess(0);
    pmanager[1]->AddProcess(new G4MonopoleTransportation(fApl), -1, 0, 0);
  }

  if (magn != 0.0) {
    G4double chg = sqrt(magn * magn + elec * elec);//TODO properly combine electric and magnetic ionisation
    G4mplIonisation* mplioni = new G4mplIonisation(chg);
    mplioni->SetDEDXBinning(nbin);
    mplioni->SetMinKinEnergy(emin);
    mplioni->SetMaxKinEnergy(emax);
    ph->RegisterProcess(mplioni, fMpl);
    ph->RegisterProcess(mplioni, fApl);
  }
  if (elec != 0.0 && magn == 0.0) {
    G4hIonisation* hhioni = new G4hIonisation();
    hhioni->SetDEDXBinning(nbin);
    hhioni->SetMinKinEnergy(emin);
    hhioni->SetMaxKinEnergy(emax);
    ph->RegisterProcess(hhioni, fMpl);
    ph->RegisterProcess(hhioni, fApl);
  }
  ph->RegisterProcess(new G4StepLimiter(), fMpl);
  ph->RegisterProcess(new G4StepLimiter(), fApl);
}

void G4MonopolePhysics::SetMagneticCharge(G4double val)
{
  fMagCharge = val;
}

void G4MonopolePhysics::SetElectricCharge(G4double val)
{
  fElCharge = val;
}

void G4MonopolePhysics::SetMonopoleMass(G4double mass)
{
  fMonopoleMass = mass;
}

