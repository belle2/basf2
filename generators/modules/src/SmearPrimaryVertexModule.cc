/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/SmearPrimaryVertexModule.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/MCInitialParticles.h>

#include <TVector3.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SmearPrimaryVertex)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SmearPrimaryVertexModule::SmearPrimaryVertexModule() : Module(),
  m_Initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("Smears primary vertex and all subsequent vertices of all the MCParticles using the informations stored in BeamParameters.");

  //Parameter definition
  addParam("MCParticlesStoreArrayName", m_MCParticlesName, "Name of the MCParticles StoreArray.", std::string(""));
}

SmearPrimaryVertexModule::~SmearPrimaryVertexModule()
{
}

void SmearPrimaryVertexModule::initialize()
{
  StoreArray<MCParticle> mcParticles(m_MCParticlesName);
  mcParticles.isRequired(m_MCParticlesName);
  m_Initial.initialize();
}

void SmearPrimaryVertexModule::beginRun()
{
  if (not m_BeamParameters.isValid())
    B2FATAL("Beam Parameters data are not available.");
}

void SmearPrimaryVertexModule::event()
{
  StoreArray<MCParticle> mcParticles(m_MCParticlesName);
  MCInitialParticles& initial = m_Initial.generate();
  m_NewPrimaryVertex = initial.getVertex();
  bool primaryVertexFound = false;
  for (MCParticle& mcParticle : mcParticles) {
    if (mcParticle.hasStatus(MCParticle::c_Initial) or mcParticle.hasStatus(MCParticle::c_IsVirtual))
      continue;
    if (not primaryVertexFound) {
      m_OldPrimaryVertex = mcParticle.getProductionVertex();
      primaryVertexFound = true;
    }
    /* Shift the production vertex. */
    mcParticle.setProductionVertex(getShiftedVertex(mcParticle.getProductionVertex()));
    /* Shift also the decay vertex only if the MCParticle has a daughter. */
    if (mcParticle.getNDaughters() > 0)
      mcParticle.setDecayVertex(getShiftedVertex(mcParticle.getDecayVertex()));
  }
}
