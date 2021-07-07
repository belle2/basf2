/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/PionPhysics.h>

#include "G4ProcessManager.hh"
#include "G4PionPlusInelasticProcess.hh"
#include "G4PionMinusInelasticProcess.hh"
#include "G4HadronElasticProcess.hh"
#include "G4HadronicAbsorptionBertini.hh"

#include "G4CascadeInterface.hh"
#include "G4TheoFSGenerator.hh"
#include "G4FTFModel.hh"
#include "G4ExcitedStringDecay.hh"
#include "G4LundStringFragmentation.hh"
#include "G4GeneratorPrecompoundInterface.hh"
#include "G4HadronElastic.hh"
#include "G4ElasticHadrNucleusHE.hh"

#include "G4BGGPionElasticXS.hh"
#include "G4BGGPionInelasticXS.hh"
#include "G4SystemOfUnits.hh"

using namespace Belle2;
using namespace Simulation;


PionPhysics::PionPhysics()
  : m_ftfp(nullptr), m_stringModel(nullptr), m_stringDecay(nullptr),
    m_fragModel(nullptr), m_preCompoundModel(nullptr)
{}


PionPhysics::~PionPhysics()
{
  delete m_stringDecay;
  delete m_stringModel;
  delete m_fragModel;
  delete m_preCompoundModel;
}


void PionPhysics::ConstructParticle()
{}


void PionPhysics::ConstructProcess()
{
  G4ProcessManager* procMan;

  // Low energy elastic model
  G4HadronElastic* loElModel = new G4HadronElastic();
  loElModel->SetMaxEnergy(1.0001 * GeV);

  // High energy elastic model
  G4ElasticHadrNucleusHE* hiElModel = new G4ElasticHadrNucleusHE();
  hiElModel->SetMinEnergy(1.0 * GeV);

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
  m_ftfp->SetMinEnergy(10 * GeV);
  m_ftfp->SetMaxEnergy(100 * TeV);

  //////////////////////////////////////////////////////////////////////////////
  //   pi+                                                                    //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4PionPlus::PionPlus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* pipProcEl = new G4HadronElasticProcess;
  pipProcEl->RegisterMe(loElModel);
  pipProcEl->RegisterMe(hiElModel);
  pipProcEl->AddDataSet(new G4BGGPionElasticXS(G4PionPlus::PionPlus()));
  procMan->AddDiscreteProcess(pipProcEl);

  // inelastic
  G4PionPlusInelasticProcess* pipProcInel = new G4PionPlusInelasticProcess;
  pipProcInel->RegisterMe(loInelModel);
  pipProcInel->RegisterMe(m_ftfp);
  pipProcInel->AddDataSet(new G4BGGPionInelasticXS(G4PionPlus::PionPlus()));
  procMan->AddDiscreteProcess(pipProcInel);

  //////////////////////////////////////////////////////////////////////////////
  //   pi-                                                                    //
  //////////////////////////////////////////////////////////////////////////////

  procMan = G4PionMinus::PionMinus()->GetProcessManager();

  // elastic
  G4HadronElasticProcess* pimProcEl = new G4HadronElasticProcess;
  pimProcEl->RegisterMe(loElModel);
  pimProcEl->RegisterMe(hiElModel);
  pimProcEl->AddDataSet(new G4BGGPionElasticXS(G4PionMinus::PionMinus()));
  procMan->AddDiscreteProcess(pimProcEl);

  // inelastic
  G4PionMinusInelasticProcess* pimProcInel = new G4PionMinusInelasticProcess;
  pimProcInel->RegisterMe(loInelModel);
  pimProcInel->RegisterMe(m_ftfp);
  pimProcInel->AddDataSet(new G4BGGPionInelasticXS(G4PionMinus::PionMinus()));
  procMan->AddDiscreteProcess(pimProcInel);

  // stopping
  G4HadronicAbsorptionBertini* bertAbsorb = new G4HadronicAbsorptionBertini;
  procMan->AddRestProcess(bertAbsorb);

}

