/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/NeutronPhysics.h>

#include "G4ProcessManager.hh"
#include "G4NeutronInelasticProcess.hh"
#include "G4HadronElasticProcess.hh"
#include "G4HadronCaptureProcess.hh"
#include "G4NeutronKiller.hh"

#include "G4CascadeInterface.hh"
#include "G4TheoFSGenerator.hh"
#include "G4FTFModel.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4LundStringFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4ChipsElasticModel.hh"
#include "G4NeutronRadCapture.hh"

#include "G4BGGNucleonInelasticXS.hh"
#include "G4NeutronElasticXS.hh"
#include "G4NeutronCaptureXS.hh"

#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


NeutronPhysics::NeutronPhysics()
  : m_ftfp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr)
{}


NeutronPhysics::~NeutronPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;
}


void NeutronPhysics::ConstructParticle()
{}


void NeutronPhysics::ConstructProcess()
{
  // Low energy elastic model
  G4ChipsElasticModel* elMod = new G4ChipsElasticModel();

  // Use Bertini cascade for low energies
  G4CascadeInterface* loInelModel = new G4CascadeInterface;
  loInelModel->SetMinEnergy(0.0);
  loInelModel->SetMaxEnergy(12.0 * GeV);

  // Capture model
  G4NeutronRadCapture* capModel = new G4NeutronRadCapture();

  // Use FTFP for high energies   ==>>   eventually replace this with new class FTFPInterface
  m_ftfp = new G4TheoFSGenerator("FTFP");
  m_stringModel = new G4FTFModel;
  m_stringDecay =
    new G4ExcitedStringDecay(m_fragModel = new G4LundStringFragmentation);
  m_stringModel->SetFragmentationModel(m_stringDecay);
  m_preCompoundModel = new G4GeneratorPrecompoundInterface();

  m_ftfp->SetHighEnergyGenerator(m_stringModel);
  m_ftfp->SetTransport(m_preCompoundModel);
  m_ftfp->SetMinEnergy(5 * GeV);
  m_ftfp->SetMaxEnergy(100 * TeV);

  // Cross section sets
  G4BGGNucleonInelasticXS* inelCS = new G4BGGNucleonInelasticXS(G4Neutron::Neutron());
  G4NeutronElasticXS* elCS = new G4NeutronElasticXS;
  G4NeutronCaptureXS* capCS = new G4NeutronCaptureXS;

  G4ProcessManager* procMan = G4Neutron::Neutron()->GetProcessManager();

  // Elastic process
  G4HadronElasticProcess* nProcEl = new G4HadronElasticProcess;
  nProcEl->RegisterMe(elMod);
  nProcEl->AddDataSet(elCS);
  procMan->AddDiscreteProcess(nProcEl);

  // Inelastic process
  G4NeutronInelasticProcess* nProcInel = new G4NeutronInelasticProcess;
  nProcInel->RegisterMe(loInelModel);
  nProcInel->RegisterMe(m_ftfp);
  nProcInel->AddDataSet(inelCS);
  procMan->AddDiscreteProcess(nProcInel);

  // Capture process
  G4HadronCaptureProcess* nProcCap = new G4HadronCaptureProcess("nCapture");
  nProcCap->RegisterMe(capModel);
  nProcCap->AddDataSet(capCS);
  procMan->AddDiscreteProcess(nProcCap);

  // Neutron cut (kill neutrons that live too long or have too little energy)
  G4NeutronKiller* nKiller = new G4NeutronKiller();
  nKiller->SetKinEnergyLimit(0.0 * MeV);
  nKiller->SetTimeLimit(10.*microsecond);
  procMan->AddDiscreteProcess(nKiller);

}


