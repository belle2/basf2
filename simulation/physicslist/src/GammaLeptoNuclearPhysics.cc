/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/GammaLeptoNuclearPhysics.h>

#include "G4ProcessManager.hh"
#include "G4PhotoNuclearProcess.hh"
#include "G4ElectronNuclearProcess.hh"
#include "G4PositronNuclearProcess.hh"
#include "G4MuonNuclearProcess.hh"

#include "G4CascadeInterface.hh"
#include "G4ElectroVDNuclearModel.hh"
#include "G4MuonVDNuclearModel.hh"

#include "G4TheoFSGenerator.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4QGSMFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"

#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


GammaLeptoNuclearPhysics::GammaLeptoNuclearPhysics()
  : m_qgsp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr)
{}


GammaLeptoNuclearPhysics::~GammaLeptoNuclearPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;
}


void GammaLeptoNuclearPhysics::ConstructProcess()
{
  // Use Bertini cascade for low energies
  G4CascadeInterface* theGammaReaction = new G4CascadeInterface;
  theGammaReaction->SetMinEnergy(0.0);
  theGammaReaction->SetMaxEnergy(3.5 * GeV);

  // Use QGSP for high energies
  m_qgsp = new G4TheoFSGenerator("QGSP");
  m_stringModel = new G4QGSModel<G4GammaParticipants>;
  m_stringDecay =
    new G4ExcitedStringDecay(m_fragModel = new G4QGSMFragmentation);
  m_stringModel->SetFragmentationModel(m_stringDecay);
  m_preCompoundModel = new G4GeneratorPrecompoundInterface();

  m_qgsp->SetHighEnergyGenerator(m_stringModel);
  m_qgsp->SetTransport(m_preCompoundModel);
  m_qgsp->SetMinEnergy(3 * GeV);
  m_qgsp->SetMaxEnergy(100 * TeV);

  // Lepto-nuclear models
  G4ElectroVDNuclearModel* evdn = new G4ElectroVDNuclearModel;
  G4MuonVDNuclearModel* mvdn = new G4MuonVDNuclearModel;


  G4ProcessManager* procMan = 0;

  // Gamma
  procMan = G4Gamma::Gamma()->GetProcessManager();
  G4PhotoNuclearProcess* pnProc = new G4PhotoNuclearProcess;
  pnProc->RegisterMe(theGammaReaction);
  pnProc->RegisterMe(m_qgsp);
  procMan->AddDiscreteProcess(pnProc);

  // Electron
  procMan = G4Electron::Electron()->GetProcessManager();
  G4ElectronNuclearProcess* emn = new G4ElectronNuclearProcess;
  emn->RegisterMe(evdn);
  procMan->AddDiscreteProcess(emn);

  // Positron
  procMan = G4Positron::Positron()->GetProcessManager();
  G4PositronNuclearProcess* epn = new G4PositronNuclearProcess;
  epn->RegisterMe(evdn);
  procMan->AddDiscreteProcess(epn);

  // Muon-
  procMan = G4MuonMinus::MuonMinus()->GetProcessManager();
  G4MuonNuclearProcess* mun = new G4MuonNuclearProcess;
  mun->RegisterMe(mvdn);
  procMan->AddDiscreteProcess(mun);

  // Muon+
  procMan = G4MuonPlus::MuonPlus()->GetProcessManager();
  procMan->AddDiscreteProcess(mun);

}


void GammaLeptoNuclearPhysics::ConstructParticle()
{}

