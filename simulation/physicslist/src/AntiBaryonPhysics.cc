/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/AntiBaryonPhysics.h>

#include "G4ProcessManager.hh"
#include "G4AntiProtonInelasticProcess.hh"
#include "G4AntiNeutronInelasticProcess.hh"
#include "G4AntiLambdaInelasticProcess.hh"
#include "G4AntiSigmaPlusInelasticProcess.hh"
#include "G4AntiSigmaMinusInelasticProcess.hh"
#include "G4AntiXiZeroInelasticProcess.hh"
#include "G4AntiXiMinusInelasticProcess.hh"
#include "G4AntiOmegaMinusInelasticProcess.hh"

#include "G4AntiDeuteronInelasticProcess.hh"
#include "G4AntiTritonInelasticProcess.hh"
#include "G4AntiHe3InelasticProcess.hh"
#include "G4AntiAlphaInelasticProcess.hh"
#include "G4HadronElasticProcess.hh"

#include "G4TheoFSGenerator.hh"
#include "G4FTFModel.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4LundStringFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4HadronElastic.hh"
#include "G4AntiNuclElastic.hh"
#include "G4HadronicAbsorptionFritiof.hh"

#include "G4ChipsAntiBaryonElasticXS.hh"
#include "G4ChipsAntiBaryonInelasticXS.hh"
#include "G4ComponentAntiNuclNuclearXS.hh"
#include "G4CrossSectionInelastic.hh"
#include "G4CrossSectionElastic.hh"

#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


AntiBaryonPhysics::AntiBaryonPhysics()
  : m_ftfp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr),
    m_theAntiNucleonXS(nullptr)
{}


AntiBaryonPhysics::~AntiBaryonPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;

  delete m_theAntiNucleonXS;
}


void AntiBaryonPhysics::ConstructParticle()
{}


void AntiBaryonPhysics::ConstructProcess()
{
  G4ProcessManager* procMan = 0;

  // One elastic model for all anti-hyperon and anti-neutron energies
  G4HadronElastic* elModel = new G4HadronElastic();

  // Elastic models for anti-(p, d, t, He3, alpha)
  G4HadronElastic* loelModel = new G4HadronElastic();
  loelModel->SetMaxEnergy(100.1 * MeV);

  G4AntiNuclElastic* anucEl = new G4AntiNuclElastic();
  anucEl->SetMinEnergy(100.0 * MeV);

  // Use FTFP for all energies   ==>>   eventually replace this with new class FTFPInterface
  m_ftfp = new G4TheoFSGenerator("FTFP");
  m_stringModel = new G4FTFModel;
  m_stringDecay =
    new G4ExcitedStringDecay(m_fragModel = new G4LundStringFragmentation);
  m_stringModel->SetFragmentationModel(m_stringDecay);
  m_preCompoundModel = new G4GeneratorPrecompoundInterface();

  m_ftfp->SetHighEnergyGenerator(m_stringModel);
  m_ftfp->SetTransport(m_preCompoundModel);
  m_ftfp->SetMinEnergy(0.0);
  m_ftfp->SetMaxEnergy(100 * TeV);

  // Elastic data set
  G4CrossSectionElastic* anucElxs =
    new G4CrossSectionElastic(anucEl->GetComponentCrossSection());

  // Inelastic cross section sets
  m_theAntiNucleonXS = new G4ComponentAntiNuclNuclearXS;
  G4VCrossSectionDataSet* antiNucleonData =
    new G4CrossSectionInelastic(m_theAntiNucleonXS);

  G4ChipsAntiBaryonElasticXS* hchipsElastic = new G4ChipsAntiBaryonElasticXS;
  G4ChipsAntiBaryonInelasticXS* hchipsInelastic = new G4ChipsAntiBaryonInelasticXS;

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-proton                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiProton::AntiProton()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* apProcEl = new G4HadronElasticProcess;
  apProcEl->RegisterMe(loelModel);
  apProcEl->RegisterMe(anucEl);
  apProcEl->AddDataSet(anucElxs);
  procMan->AddDiscreteProcess(apProcEl);

  // inelastic
  G4AntiProtonInelasticProcess* apProcInel = new G4AntiProtonInelasticProcess;
  apProcInel->RegisterMe(m_ftfp);
  apProcInel->AddDataSet(antiNucleonData);
  procMan->AddDiscreteProcess(apProcInel);

  // stopping
  G4HadronicAbsorptionFritiof* apAbsorb = new G4HadronicAbsorptionFritiof();
  procMan->AddRestProcess(apAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-neutron                                                           //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiNeutron::AntiNeutron()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* anProcEl = new G4HadronElasticProcess;
  anProcEl->RegisterMe(elModel);
  anProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(anProcEl);

  // inelastic
  G4AntiNeutronInelasticProcess* anProcInel = new G4AntiNeutronInelasticProcess;
  anProcInel->RegisterMe(m_ftfp);
  anProcInel->AddDataSet(antiNucleonData);
  procMan->AddDiscreteProcess(anProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-deuteron                                                          //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiDeuteron::AntiDeuteron()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* adProcEl = new G4HadronElasticProcess;
  adProcEl->RegisterMe(loelModel);
  adProcEl->RegisterMe(anucEl);
  adProcEl->AddDataSet(anucElxs);
  procMan->AddDiscreteProcess(adProcEl);

  // inelastic
  G4AntiDeuteronInelasticProcess* adProcInel = new G4AntiDeuteronInelasticProcess;
  adProcInel->RegisterMe(m_ftfp);
  adProcInel->AddDataSet(antiNucleonData);
  procMan->AddDiscreteProcess(adProcInel);

  // stopping
  G4HadronicAbsorptionFritiof* adAbsorb = new G4HadronicAbsorptionFritiof();
  procMan->AddRestProcess(adAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-triton                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiTriton::AntiTriton()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* atProcEl = new G4HadronElasticProcess;
  atProcEl->RegisterMe(loelModel);
  atProcEl->RegisterMe(anucEl);
  atProcEl->AddDataSet(anucElxs);
  procMan->AddDiscreteProcess(atProcEl);

  // inelastic
  G4AntiTritonInelasticProcess* atProcInel = new G4AntiTritonInelasticProcess;
  atProcInel->RegisterMe(m_ftfp);
  atProcInel->AddDataSet(antiNucleonData);
  procMan->AddDiscreteProcess(atProcInel);

  // stopping
  G4HadronicAbsorptionFritiof* atAbsorb = new G4HadronicAbsorptionFritiof();
  procMan->AddRestProcess(atAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-He3                                                               //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiHe3::AntiHe3()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* ahe3ProcEl = new G4HadronElasticProcess;
  ahe3ProcEl->RegisterMe(loelModel);
  ahe3ProcEl->RegisterMe(anucEl);
  ahe3ProcEl->AddDataSet(anucElxs);
  procMan->AddDiscreteProcess(ahe3ProcEl);

  // inelastic
  G4AntiHe3InelasticProcess* ahe3ProcInel = new G4AntiHe3InelasticProcess;
  ahe3ProcInel->RegisterMe(m_ftfp);
  ahe3ProcInel->AddDataSet(antiNucleonData);
  procMan->AddDiscreteProcess(ahe3ProcInel);

  // stopping
  G4HadronicAbsorptionFritiof* ahe3Absorb = new G4HadronicAbsorptionFritiof();
  procMan->AddRestProcess(ahe3Absorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-alpha                                                             //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiAlpha::AntiAlpha()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* aaProcEl = new G4HadronElasticProcess;
  aaProcEl->RegisterMe(loelModel);
  aaProcEl->RegisterMe(anucEl);
  aaProcEl->AddDataSet(anucElxs);
  procMan->AddDiscreteProcess(aaProcEl);

  // inelastic
  G4AntiAlphaInelasticProcess* aaProcInel = new G4AntiAlphaInelasticProcess;
  aaProcInel->RegisterMe(m_ftfp);
  aaProcInel->AddDataSet(antiNucleonData);
  procMan->AddDiscreteProcess(aaProcInel);

  // stopping
  G4HadronicAbsorptionFritiof* aaAbsorb = new G4HadronicAbsorptionFritiof();
  procMan->AddRestProcess(aaAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-lambda                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiLambda::AntiLambda()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* alamProcEl = new G4HadronElasticProcess;
  alamProcEl->RegisterMe(elModel);
  alamProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(alamProcEl);

  // inelastic
  G4AntiLambdaInelasticProcess* alamProcInel = new G4AntiLambdaInelasticProcess;
  alamProcInel->RegisterMe(m_ftfp);
  alamProcInel->AddDataSet(hchipsInelastic);
  procMan->AddDiscreteProcess(alamProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-sigma+                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiSigmaPlus::AntiSigmaPlus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* aspProcEl = new G4HadronElasticProcess;
  aspProcEl->RegisterMe(elModel);
  aspProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(aspProcEl);

  // inelastic
  G4AntiSigmaPlusInelasticProcess* aspProcInel = new G4AntiSigmaPlusInelasticProcess;
  aspProcInel->RegisterMe(m_ftfp);
  aspProcInel->AddDataSet(hchipsInelastic);
  procMan->AddDiscreteProcess(aspProcInel);

  // stopping
  G4HadronicAbsorptionFritiof* aspAbsorb = new G4HadronicAbsorptionFritiof();
  procMan->AddRestProcess(aspAbsorb);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-sigma-                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiSigmaMinus::AntiSigmaMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* asmProcEl = new G4HadronElasticProcess;
  asmProcEl->RegisterMe(elModel);
  asmProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(asmProcEl);

  // inelastic
  G4AntiSigmaMinusInelasticProcess* asmProcInel = new G4AntiSigmaMinusInelasticProcess;
  asmProcInel->RegisterMe(m_ftfp);
  asmProcInel->AddDataSet(hchipsInelastic);
  procMan->AddDiscreteProcess(asmProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-xi0                                                               //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiXiZero::AntiXiZero()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* axzProcEl = new G4HadronElasticProcess;
  axzProcEl->RegisterMe(elModel);
  axzProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(axzProcEl);

  // inelastic
  G4AntiXiZeroInelasticProcess* axzProcInel = new G4AntiXiZeroInelasticProcess;
  axzProcInel->RegisterMe(m_ftfp);
  axzProcInel->AddDataSet(hchipsInelastic);
  procMan->AddDiscreteProcess(axzProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-xi-                                                               //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiXiMinus::AntiXiMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* axmProcEl = new G4HadronElasticProcess;
  axmProcEl->RegisterMe(elModel);
  axmProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(axmProcEl);

  // inelastic
  G4AntiXiMinusInelasticProcess* axmProcInel = new G4AntiXiMinusInelasticProcess;
  axmProcInel->RegisterMe(m_ftfp);
  axmProcInel->AddDataSet(hchipsInelastic);
  procMan->AddDiscreteProcess(axmProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   Anti-omega-                                                            //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4AntiOmegaMinus::AntiOmegaMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* aomProcEl = new G4HadronElasticProcess;
  aomProcEl->RegisterMe(elModel);
  aomProcEl->AddDataSet(hchipsElastic);
  procMan->AddDiscreteProcess(aomProcEl);

  // inelastic
  G4AntiOmegaMinusInelasticProcess* aomProcInel = new G4AntiOmegaMinusInelasticProcess;
  aomProcInel->RegisterMe(m_ftfp);
  aomProcInel->AddDataSet(hchipsInelastic);
  procMan->AddDiscreteProcess(aomProcInel);

}

