/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Giacomo De Pietro                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <generators/modules/SmearPrimaryVertexModule.h>

/* Belle II headers. */
#include <framework/dataobjects/MCInitialParticles.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticle.h>

/* ROOT headers. */
#include <TVector3.h>

using namespace Belle2;

REG_MODULE(SmearPrimaryVertex)

SmearPrimaryVertexModule::SmearPrimaryVertexModule() : Module(),
  m_Initial(BeamParameters::c_smearVertex)
{
  /* Module description. */
  setDescription("Smears primary vertex and all subsequent vertices of all the MCParticles using the informations stored in BeamParameters.");
  /* Parameters definition. */
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

void SmearPrimaryVertexModule::event()
{
  StoreArray<MCParticle> mcParticles(m_MCParticlesName);
  /* Generate the primary beams. */
  MCInitialParticles& initial = m_Initial.generate();
  TVector3 shift;
  bool primaryVertexFound = false;
  for (MCParticle& mcParticle : mcParticles) {
    /* Skip an MCParticle if it is flagged as c_Initial or c_IsVirtual. */
    if (not(mcParticle.hasStatus(MCParticle::c_Initial) or mcParticle.hasStatus(MCParticle::c_IsVirtual)))
      continue;
    if (not primaryVertexFound) {
      /* Save the previous primary vertex. */
      shift = initial.getVertex() - mcParticle.getProductionVertex();
      primaryVertexFound = true;
    }
    /* Shift the production vertex. */
    mcParticle.setProductionVertex(mcParticle.getProductionVertex() + shift);
    /* Shift also the decay vertex only if the MCParticle has a daughter. */
    if (mcParticle.getNDaughters() > 0)
      mcParticle.setDecayVertex(mcParticle.getDecayVertex() + shift);
  }
}
