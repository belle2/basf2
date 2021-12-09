/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own include. */
#include <generators/modules/BoostMCParticlesModule.h>

/* Belle2 headers. */
#include <framework/dataobjects/MCInitialParticles.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TLorentzVector.h>

using namespace Belle2;

REG_MODULE(BoostMCParticles)

BoostMCParticlesModule::BoostMCParticlesModule() : Module(), m_firstEvent{true}, m_initial(0)
{
  setDescription(
    R"DOC(Module for boosting the MCParticles from CM to LAB frame. The module must be appended to the path only when HepMC, Hepevt or LHE input files are used.)DOC");
  addParam("MCParticlesStoreArrayName", m_mcParticlesName, "Name of the MC particles StoreArray.", std::string(""));
}

void BoostMCParticlesModule::initialize()
{
  m_mcParticles.isRequired(m_mcParticlesName);
  m_initial.initialize();
  const MCInitialParticles& initial = m_initial.generate();
  m_boost = initial.getCMSToLab();
}

void BoostMCParticlesModule::event()
{
  if (m_beamParameters.hasChanged()) {
    if (not m_firstEvent) {
      // Be lenient with the first event: BeamParameters may be changed because of some basf2 black magic,
      // so print the warning only if it changes during subsequent events.
      B2WARNING("BeamParameters changed within the same process, check if this is fine according to the primary generator you are using.");
    }
    const MCInitialParticles& initial = m_initial.generate();
    m_boost = initial.getCMSToLab();
  }
  m_firstEvent = false;
  for (MCParticle& mcParticle : m_mcParticles) {
    TLorentzVector momentum = mcParticle.get4Vector();
    mcParticle.set4Vector(m_boost * momentum);
    if (mcParticle.getProductionTime() != 0.) { // Boost only if the production vertex is not the default one.
      TLorentzVector productionVertex{mcParticle.getProductionVertex(), Const::speedOfLight * mcParticle.getProductionTime()};
      productionVertex = m_boost * productionVertex;
      mcParticle.setProductionVertex(productionVertex.X(), productionVertex.Y(), productionVertex.Z());
      mcParticle.setProductionTime(productionVertex.T() / Const::speedOfLight);
    }
    if (mcParticle.getDecayTime() != 0.) { // Boost only if the decay vertex is not the default one.
      TLorentzVector decayVertex{mcParticle.getDecayVertex(), Const::speedOfLight * mcParticle.getDecayTime()};
      decayVertex = m_boost * decayVertex;
      mcParticle.setDecayVertex(decayVertex.X(), decayVertex.Y(), decayVertex.Z());
      mcParticle.setDecayTime(decayVertex.T() / Const::speedOfLight);
    }
  }
}
