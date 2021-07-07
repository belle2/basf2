/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopolePhysics.h>
#include <simulation/monopoles/G4mplIonisation.h>
#include <simulation/monopoles/G4Monopole.h>
#include <simulation/monopoles/G4MonopoleTransportation.h>
#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>

#include <G4ProcessManager.hh>
#include <G4StepLimiter.hh>
#include <G4hIonisation.hh>
#include <G4PhysicsListHelper.hh>
#include <G4BuilderType.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;
using namespace CLHEP;

G4MonopolePhysics::G4MonopolePhysics(double magneticCharge)
  : G4VPhysicsConstructor("Monopole physics"),
    fMagCharge(magneticCharge), //in units of the positron charge
    fMpl(0), fApl(0)
{
  //No way to store magnetic charge in TDatabasePDG,
  //so part of the information (e, m, code, etc.) should be stored before generation
  //and other part (g) passed to the simulation setup
  const auto monopoleInPDG = TDatabasePDG::Instance()->GetParticle(c_monopolePDGCode);
  const auto antiMonopoleInPDG = TDatabasePDG::Instance()->GetParticle(-c_monopolePDGCode);
  if (!monopoleInPDG || !antiMonopoleInPDG) {
    B2FATAL("Monopole physics was requested, but the monopole parameters"
            "were not registered in local PDG database under PID code " << c_monopolePDGCode);
  }
  fElCharge  = monopoleInPDG->Charge() / 3.0; //TParticlePDG returns in units of |e|/3
  fMonopoleMass = antiMonopoleInPDG->Mass() * GeV;
  SetPhysicsType(bUnknown);
}

G4MonopolePhysics::~G4MonopolePhysics()
{
}

void G4MonopolePhysics::ConstructParticle()
{
  fMpl = new G4Monopole("monopole",      fMonopoleMass,  fMagCharge,  fElCharge,  c_monopolePDGCode);
//NOTE careful not to use same name or encoding, this will lead to G4exception
  fApl = new G4Monopole("anti-monopole", fMonopoleMass, -fMagCharge, -fElCharge, -c_monopolePDGCode);
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
//
//  commented out the following 3 lines,
//  to supress a cppcheck [duplicateCondition] warning for the if condition
//  }
//
//  if (magn != 0.0) {
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
