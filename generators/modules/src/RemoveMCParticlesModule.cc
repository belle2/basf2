/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Marko Petric                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/RemoveMCParticlesModule.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RemoveMCParticles)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

namespace {
  /** Small helper function to cut a value if outside a given range */
  inline bool alwaysCut(double min, double max, double value)
  {
    return (value < min || value >= max);
  }
  /** Small helper function to cut a value if outside a given range, but only if min<max */
  inline bool optionalCut(double min, double max, double value)
  {
    if (min < max) return (value < min || value >= max);
    return false;
  }
}

RemoveMCParticlesModule::RemoveMCParticlesModule() : Module()
{
  //Set module properties
  setDescription(R"DOC(
Remove particles from the MCParticle Collection.

Warning:
  At the moment, Relations to that MCParticle collection will become invalid
  and are not fixed automatically
)DOC");

  //Parameter definition
  addParam("collectionName", m_particleList, "Collection to perform the cuts on", std::string(""));
  addParam("minZ", m_minZ,
           "Minimum Z value of Particles to be kept. If bigger or equal to maxZ, no cut on z is performed", 0.0);
  addParam("maxZ", m_maxZ,
           "Maximum Z value of Particles to be kept. If smaller or equal to minZ, no cut on z is performed", 0.0);
  addParam("minR", m_minR,
           "Minimum Rphi value of Particles to be kept. If bigger or equal to maxR, no cut on Rphi is performed", 0.0);
  addParam("maxR", m_maxR,
           "Maximum Rphi value of Particles to be kept. If smaller or equal to minR, no cut on Rphi is performed", 0.0);
  addParam("minTheta", m_minTheta,
           "Minimum theta value of Particles to be kept", 0.0);
  addParam("maxTheta", m_maxTheta,
           "Maximum theta value of Particles to be kept", 180.0);
  addParam("minPt", m_minPt,
           "Minimum Pt value of Particles to be kept. If bigger or equal to maxPt, no cut on Pt is performed", 0.0);
  addParam("maxPt", m_maxPt,
           "Maximum Pt value of Particles to be kept. If smaller or equal to minPt, no cut on Pt is performed", 0.0);
  addParam("alsoChildren", m_alsoChildren,
           "If true, all children of a particle are removed together with the particle, otherwise children are kept", true);
  addParam("pdgCodes", m_pdgCodes,
           "If not empty, cuts will only be performed on particles matching the given PDG codes. "
           "To remove all particles with a given code just set maxTheta to 0 and fill this list "
           "with all codes to be removed", m_pdgCodes);
}

void RemoveMCParticlesModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  std::sort(m_pdgCodes.begin(), m_pdgCodes.end());
  m_minTheta *= Unit::deg;
  m_maxTheta *= Unit::deg;
}

void RemoveMCParticlesModule::event()
{
  StoreArray<MCParticle> mcParticles(m_particleList);
  if (!mcParticles) {
    B2WARNING("No MCParticle collection with name \"" << m_particleList << "\", cannot remove anything");
    return;
  }
  unsigned nPart = mcParticles.getEntries();

  m_mpg.clear();
  m_mpg.loadList(m_particleList);
  //We know that all MCParticles in the list are also 1:1 in the graph with the
  //same index. This helps also removing children if neccessary. No we apply
  //cuts to all particles having no mother and the applyCuts method will then
  //do all the children
  for (unsigned i = 0; i < nPart; ++i) {
    MCParticle& mcp = *mcParticles[i];
    if (!mcp.getMother()) applyCuts(mcp, false);
  }

  m_mpg.generateList(m_particleList, MCParticleGraph::c_clearParticles);

  //FIXME: Update all Relations pointing to this MCParticle List?
  //We would need to create an old index -> new index list and than call
  //consolidate on all affected relations. It would be a good idea to implement
  //in a generic way (maybe as part of MCParticleGraph) to also use the same
  //code after Simulation
}

void RemoveMCParticlesModule::applyCuts(const MCParticle& particle, bool cut)
{
  //If we don't remove all children, the cut does not propagate
  if (!m_alsoChildren) cut = false;

  //Only apply cuts if list of pdg codes is empty or pdg code of current particle is in list
  if (m_pdgCodes.empty() || std::binary_search(m_pdgCodes.begin(), m_pdgCodes.end(), particle.getPDG())) {
    cut |= optionalCut(m_minZ, m_maxZ, particle.getProductionVertex().Z());
    cut |= optionalCut(m_minR, m_maxR, particle.getProductionVertex().Perp());
    cut |= optionalCut(m_minPt, m_maxPt, particle.getMomentum().Pt());
    cut |= alwaysCut(m_minTheta, m_maxTheta, particle.getMomentum().Theta());
  }

  //If cut is true, then we remove this particle from the list by ignoring it in the graph
  if (cut) m_mpg[particle.getArrayIndex()].setIgnore(true);

  //Then we look at all daughters
  for (MCParticle* daughter : particle.getDaughters()) {
    applyCuts(*daughter, cut);
  }
}

void RemoveMCParticlesModule::terminate()
{
}
