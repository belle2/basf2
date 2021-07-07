/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/IonPhysics.h>

#include "G4ProcessManager.hh"
#include "G4HadronElasticProcess.hh"
#include "G4HadronInelasticProcess.hh"

#include "G4TheoFSGenerator.hh"
#include "G4FTFModel.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4LundStringFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4QMDReaction.hh"
#include "G4HadronicInteractionRegistry.hh"
#include "G4PreCompoundModel.hh"
#include "G4BinaryLightIonReaction.hh"
#include "G4NuclNuclDiffuseElastic.hh"

#include "G4CrossSectionElastic.hh"
#include "G4CrossSectionInelastic.hh"
#include "G4ComponentGGNuclNuclXsc.hh"
#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


IonPhysics::IonPhysics()
  : m_ftfp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr)
{}


IonPhysics::~IonPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;

  delete m_theGGNuclNuclXS;
  delete m_ionGGXS;
}


void IonPhysics::ConstructParticle()
{}


void IonPhysics::ConstructProcess()
{
  G4ProcessManager* procMan = 0;

  // Elastic model for generic ions (z > 2)
  G4NuclNuclDiffuseElastic* ionElastic = new G4NuclNuclDiffuseElastic;
  ionElastic->SetMinEnergy(0.0);

  // FTFP    ==>>   eventually replace this with new class FTFPInterface
  m_ftfp = new G4TheoFSGenerator("FTFP");
  m_stringModel = new G4FTFModel;
  m_stringDecay =
    new G4ExcitedStringDecay(m_fragModel = new G4LundStringFragmentation);
  m_stringModel->SetFragmentationModel(m_stringDecay);
  m_preCompoundModel = new G4GeneratorPrecompoundInterface();

  m_ftfp->SetHighEnergyGenerator(m_stringModel);
  m_ftfp->SetTransport(m_preCompoundModel);
  m_ftfp->SetMinEnergy(10.01 * GeV);
  m_ftfp->SetMaxEnergy(1.0 * TeV);

  // QMD model
  G4QMDReaction* qmd = new G4QMDReaction;
  qmd->SetMinEnergy(100.0 * MeV);
  qmd->SetMaxEnergy(10.0 * GeV);

  // BIC ion model
  G4HadronicInteraction* p =
    G4HadronicInteractionRegistry::Instance()->FindModel("PRECO");
  G4PreCompoundModel* thePreCompound = static_cast<G4PreCompoundModel*>(p);
  if (!thePreCompound) { thePreCompound = new G4PreCompoundModel; }

  G4BinaryLightIonReaction* ionBC = new G4BinaryLightIonReaction(thePreCompound);
  ionBC->SetMinEnergy(0.0 * MeV);
  ionBC->SetMaxEnergy(110.0 * MeV);

  // Elastic cross section set
  m_ionGGXS = new G4ComponentGGNuclNuclXsc;
  G4VCrossSectionDataSet* ionElasticXS = new G4CrossSectionElastic(m_ionGGXS);
  ionElasticXS->SetMinKinEnergy(0.0);

  // Inelastic cross section set
  m_theGGNuclNuclXS = new G4ComponentGGNuclNuclXsc();
  G4VCrossSectionDataSet* nuclNuclXS =
    new G4CrossSectionInelastic(m_theGGNuclNuclXS);

  //////////////////////////////////////////////////////////////////////////////
  //   Deuteron                                                               //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4Deuteron::Deuteron()->GetProcessManager();

  // elastic
  // no model available

  // inelastic
  G4HadronInelasticProcess* deutProcInel =
    new G4HadronInelasticProcess("DeuteronInelProcess", G4Deuteron::Deuteron());
  deutProcInel->RegisterMe(ionBC);
  deutProcInel->RegisterMe(qmd);
  deutProcInel->RegisterMe(m_ftfp);
  deutProcInel->AddDataSet(nuclNuclXS);
  procMan->AddDiscreteProcess(deutProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Triton                                                                 //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4Triton::Triton()->GetProcessManager();

  // elastic
  // no model available

  // inelastic
  G4HadronInelasticProcess* tritProcInel =
    new G4HadronInelasticProcess("TritonInelProcess", G4Triton::Triton());
  tritProcInel->RegisterMe(ionBC);
  tritProcInel->RegisterMe(qmd);
  tritProcInel->RegisterMe(m_ftfp);
  tritProcInel->AddDataSet(nuclNuclXS);
  procMan->AddDiscreteProcess(tritProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   He3                                                                    //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4He3::He3()->GetProcessManager();

  // elastic
  // no model available

  // inelastic
  G4HadronInelasticProcess* he3ProcInel =
    new G4HadronInelasticProcess("He3InelProcess", G4He3::He3());
  he3ProcInel->RegisterMe(ionBC);
  he3ProcInel->RegisterMe(qmd);
  he3ProcInel->RegisterMe(m_ftfp);
  he3ProcInel->AddDataSet(nuclNuclXS);
  procMan->AddDiscreteProcess(he3ProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Alpha                                                                  //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4Alpha::Alpha()->GetProcessManager();

  // elastic
  // no model available

  // inelastic
  G4HadronInelasticProcess* alphProcInel =
    new G4HadronInelasticProcess("AlphaInelProcess", G4Alpha::Alpha());
  alphProcInel->RegisterMe(ionBC);
  alphProcInel->RegisterMe(qmd);
  alphProcInel->RegisterMe(m_ftfp);
  alphProcInel->AddDataSet(nuclNuclXS);
  procMan->AddDiscreteProcess(alphProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Generic ion                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4GenericIon::GenericIon()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* ionProcEl = new G4HadronElasticProcess;
  ionProcEl->RegisterMe(ionElastic);
  ionProcEl->AddDataSet(ionElasticXS);
  procMan->AddDiscreteProcess(ionProcEl);

  // inelastic
  G4HadronInelasticProcess* genIonProcInel =
    new G4HadronInelasticProcess("IonInelProcess", G4GenericIon::GenericIon());
  genIonProcInel->RegisterMe(ionBC);
  genIonProcInel->RegisterMe(qmd);
  genIonProcInel->RegisterMe(m_ftfp);
  genIonProcInel->AddDataSet(nuclNuclXS);
  procMan->AddDiscreteProcess(genIonProcInel);

}

