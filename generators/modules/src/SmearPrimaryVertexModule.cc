/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  // smear the primary vertex if not already done
  TVector3 shift = m_Initial.updateVertex();
  if (shift == TVector3{0, 0, 0}) return;
  for (MCParticle& mcParticle : mcParticles) {
    /* Shift the production vertex. */
    mcParticle.setProductionVertex(mcParticle.getProductionVertex() + shift);
    /* Shift also the decay vertex only if the MCParticle has a daughter. */
    if (mcParticle.getNDaughters() > 0)
      mcParticle.setDecayVertex(mcParticle.getDecayVertex() + shift);
  }
}
