/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
#include "G4HadronElastic.hh"
#include "G4NuclNuclDiffuseElastic.hh"

#include "G4CrossSectionElastic.hh"
#include "G4CrossSectionInelastic.hh"
#include "G4ComponentGGNuclNuclXsc.hh"
#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


IonPhysics::IonPhysics()
{}


IonPhysics::~IonPhysics()
{
  delete stringDecay;
  delete stringModel;
  delete fragModel;
  delete preCompoundModel;

  delete theGGNuclNuclXS;
  delete ionGGXS;
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
  ftfp = new G4TheoFSGenerator("FTFP");
  stringModel = new G4FTFModel;
  stringDecay =
    new G4ExcitedStringDecay(fragModel = new G4LundStringFragmentation);
  stringModel->SetFragmentationModel(stringDecay);
  preCompoundModel = new G4GeneratorPrecompoundInterface();

  ftfp->SetHighEnergyGenerator(stringModel);
  ftfp->SetTransport(preCompoundModel);
  ftfp->SetMinEnergy(10.01 * GeV);
  ftfp->SetMaxEnergy(1.0 * TeV);

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
  ionGGXS = new G4ComponentGGNuclNuclXsc;
  G4VCrossSectionDataSet* ionElasticXS = new G4CrossSectionElastic(ionGGXS);
  ionElasticXS->SetMinKinEnergy(0.0);

  // Inelastic cross section set
  theGGNuclNuclXS = new G4ComponentGGNuclNuclXsc();
  G4VCrossSectionDataSet* nuclNuclXS =
    new G4CrossSectionInelastic(theGGNuclNuclXS);

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
  deutProcInel->RegisterMe(ftfp);
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
  tritProcInel->RegisterMe(ftfp);
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
  he3ProcInel->RegisterMe(ftfp);
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
  alphProcInel->RegisterMe(ftfp);
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
  genIonProcInel->RegisterMe(ftfp);
  genIonProcInel->AddDataSet(nuclNuclXS);
  procMan->AddDiscreteProcess(genIonProcInel);

}

