/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/HyperonPhysics.h>

#include "G4ProcessManager.hh"
#include "G4LambdaInelasticProcess.hh"
#include "G4SigmaPlusInelasticProcess.hh"
#include "G4SigmaMinusInelasticProcess.hh"
#include "G4XiZeroInelasticProcess.hh"
#include "G4XiMinusInelasticProcess.hh"
#include "G4OmegaMinusInelasticProcess.hh"

#include "G4HadronElasticProcess.hh"
#include "G4HadronicAbsorptionBertini.hh"

#include "G4CascadeInterface.hh"
#include "G4TheoFSGenerator.hh"
#include "G4FTFModel.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4LundStringFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4HadronElastic.hh"

#include "G4ChipsHyperonElasticXS.hh"
#include "G4ChipsHyperonInelasticXS.hh"
#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


HyperonPhysics::HyperonPhysics()
  : m_ftfp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr)
{}


HyperonPhysics::~HyperonPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;
}


void HyperonPhysics::ConstructParticle()
{}


void HyperonPhysics::ConstructProcess()
{
  G4ProcessManager* procMan = 0;

  // One elastic model for all hyperon energies
  G4HadronElastic* elModel = new G4HadronElastic();

  // Use Bertini cascade for low energies
  G4CascadeInterface* loInelModel = new G4CascadeInterface;
  loInelModel->SetMinEnergy(0.0);
  loInelModel->SetMaxEnergy(6.0 * GeV);

  // Use FTFP for high energies   ==>>   eventually replace this with new class FTFPInterface
  m_ftfp = new G4TheoFSGenerator("FTFP");
  m_stringModel = new G4FTFModel;
  m_stringDecay =
    new G4ExcitedStringDecay(m_fragModel = new G4LundStringFragmentation);
  m_stringModel->SetFragmentationModel(m_stringDecay);
  m_preCompoundModel = new G4GeneratorPrecompoundInterface();

  m_ftfp->SetHighEnergyGenerator(m_stringModel);
  m_ftfp->SetTransport(m_preCompoundModel);
  m_ftfp->SetMinEnergy(4 * GeV);
  m_ftfp->SetMaxEnergy(100 * TeV);

  // Cross section sets
  G4ChipsHyperonElasticXS* chipsElastic = new G4ChipsHyperonElasticXS;
  G4ChipsHyperonInelasticXS* chipsInelastic = new G4ChipsHyperonInelasticXS;

  //////////////////////////////////////////////////////////////////////////////
  //   Lambda                                                                 //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4Lambda::Lambda()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* lamProcEl = new G4HadronElasticProcess;
  lamProcEl->RegisterMe(elModel);
  lamProcEl->AddDataSet(chipsElastic);
  procMan->AddDiscreteProcess(lamProcEl);

  // inelastic
  G4LambdaInelasticProcess* lamProcInel = new G4LambdaInelasticProcess;
  lamProcInel->RegisterMe(loInelModel);
  lamProcInel->RegisterMe(m_ftfp);
  lamProcInel->AddDataSet(chipsInelastic);
  procMan->AddDiscreteProcess(lamProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Sigma+                                                                 //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4SigmaPlus::SigmaPlus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* spProcEl = new G4HadronElasticProcess;
  spProcEl->RegisterMe(elModel);
  spProcEl->AddDataSet(chipsElastic);
  procMan->AddDiscreteProcess(spProcEl);

  // inelastic
  G4SigmaPlusInelasticProcess* spProcInel = new G4SigmaPlusInelasticProcess;
  spProcInel->RegisterMe(loInelModel);
  spProcInel->RegisterMe(m_ftfp);
  spProcInel->AddDataSet(chipsInelastic);
  procMan->AddDiscreteProcess(spProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Sigma-                                                                 //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4SigmaMinus::SigmaMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* smProcEl = new G4HadronElasticProcess;
  smProcEl->RegisterMe(elModel);
  smProcEl->AddDataSet(chipsElastic);
  procMan->AddDiscreteProcess(smProcEl);

  // inelastic
  G4SigmaMinusInelasticProcess* smProcInel = new G4SigmaMinusInelasticProcess;
  smProcInel->RegisterMe(loInelModel);
  smProcInel->RegisterMe(m_ftfp);
  smProcInel->AddDataSet(chipsInelastic);
  procMan->AddDiscreteProcess(smProcInel);

  // stopping
  G4HadronicAbsorptionBertini* smAbsorb = new G4HadronicAbsorptionBertini;
  procMan->AddRestProcess(smAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Xi0                                                                    //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4XiZero::XiZero()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* xzProcEl = new G4HadronElasticProcess;
  xzProcEl->RegisterMe(elModel);
  xzProcEl->AddDataSet(chipsElastic);
  procMan->AddDiscreteProcess(xzProcEl);

  // inelastic
  G4XiZeroInelasticProcess* xzProcInel = new G4XiZeroInelasticProcess;
  xzProcInel->RegisterMe(loInelModel);
  xzProcInel->RegisterMe(m_ftfp);
  xzProcInel->AddDataSet(chipsInelastic);
  procMan->AddDiscreteProcess(xzProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Xi-                                                                    //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4XiMinus::XiMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* xmProcEl = new G4HadronElasticProcess;
  xmProcEl->RegisterMe(elModel);
  xmProcEl->AddDataSet(chipsElastic);
  procMan->AddDiscreteProcess(xmProcEl);

  // inelastic
  G4XiMinusInelasticProcess* xmProcInel = new G4XiMinusInelasticProcess;
  xmProcInel->RegisterMe(loInelModel);
  xmProcInel->RegisterMe(m_ftfp);
  xmProcInel->AddDataSet(chipsInelastic);
  procMan->AddDiscreteProcess(xmProcInel);

  // stopping
  G4HadronicAbsorptionBertini* xmAbsorb = new G4HadronicAbsorptionBertini;
  procMan->AddRestProcess(xmAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Omega-                                                                 //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4OmegaMinus::OmegaMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* omProcEl = new G4HadronElasticProcess;
  omProcEl->RegisterMe(elModel);
  omProcEl->AddDataSet(chipsElastic);
  procMan->AddDiscreteProcess(omProcEl);

  // inelastic
  G4OmegaMinusInelasticProcess* omProcInel = new G4OmegaMinusInelasticProcess;
  omProcInel->RegisterMe(loInelModel);
  omProcInel->RegisterMe(m_ftfp);
  omProcInel->AddDataSet(chipsInelastic);
  procMan->AddDiscreteProcess(omProcInel);

  // stopping
  G4HadronicAbsorptionBertini* omAbsorb = new G4HadronicAbsorptionBertini;
  procMan->AddRestProcess(omAbsorb);
}

