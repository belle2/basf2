/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/ProtonPhysics.h>

#include "G4ProcessManager.hh"
#include "G4ProtonInelasticProcess.hh"
#include "G4HadronElasticProcess.hh"

#include "G4CascadeInterface.hh"
#include "G4TheoFSGenerator.hh"
#include "G4FTFModel.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4LundStringFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4ChipsElasticModel.hh"

#include "G4BGGNucleonInelasticXS.hh"
#include "G4ChipsProtonElasticXS.hh"

#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


ProtonPhysics::ProtonPhysics()
  : m_ftfp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr)
{}


ProtonPhysics::~ProtonPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;
}


void ProtonPhysics::ConstructProcess()
{
  // Low energy elastic model
  G4ChipsElasticModel* elMod = new G4ChipsElasticModel();

  // Use Bertini cascade for low energies
  G4CascadeInterface* loInelModel = new G4CascadeInterface;
  loInelModel->SetMinEnergy(0.0);
  loInelModel->SetMaxEnergy(12.0 * GeV);

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

  // Inelastic cross section
  G4BGGNucleonInelasticXS* inelCS = new G4BGGNucleonInelasticXS(G4Proton::Proton());
  G4ChipsProtonElasticXS* elCS = new G4ChipsProtonElasticXS;

  G4ProcessManager* procMan = G4Proton::Proton()->GetProcessManager();

  // Elastic process
  G4HadronElasticProcess* pProcEl = new G4HadronElasticProcess;
  pProcEl->RegisterMe(elMod);
  pProcEl->AddDataSet(elCS);
  procMan->AddDiscreteProcess(pProcEl);

  // Inelastic process
  G4ProtonInelasticProcess* pProcInel = new G4ProtonInelasticProcess;
  pProcInel->RegisterMe(loInelModel);
  pProcInel->RegisterMe(m_ftfp);
  pProcInel->AddDataSet(inelCS);
  procMan->AddDiscreteProcess(pProcInel);
}


void ProtonPhysics::ConstructParticle()
{}

