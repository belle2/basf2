/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/DrawMCParticlesModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <geometry/geodetector/CreatorUtils.h>

#include <boost/foreach.hpp>
#include <TGeoManager.h>

using namespace std;
using namespace Belle2;
using namespace Generators;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DrawMCParticles)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DrawMCParticlesModule::DrawMCParticlesModule() : Module()
{
  //Set module properties
  setDescription("Draws the content of the MCParticle Collection to the gGeoManager.");

  //Parameter definition
  addParam("collectionName", m_particleList, "Collection to draw", string(DEFAULT_MCPARTICLES));
  addParam("onlyPrimaries", m_onlyPrimaries, "Draw only primary particles", true);
}


void DrawMCParticlesModule::event()
{
  if (gGeoManager == NULL) {
    B2ERROR("Could not find a TGeoManager ! No MCParticle will be drawn.")
    return;
  }

  StoreArray<MCParticle> MCParticles(m_particleList);
  m_seen.clear();
  m_seen.resize(MCParticles.GetEntries() + 1, false);

  //Loop over the primary particles (no mother particle exists)
  for (int i = 0; i < MCParticles.GetEntries(); i++) {
    MCParticle &mc = *MCParticles[i];
    if (mc.getMother() != NULL) continue;
    drawTree(mc);
  }
}


void DrawMCParticlesModule::drawTree(const MCParticle &mc)
{
  if (m_onlyPrimaries && !mc.hasStatus(MCParticle::PrimaryParticle)) return;
  if (m_seen[mc.getIndex()]) return;

  CreatorUtils::addArrow(mc.getVertex(), mc.getMomentum());

  const vector<MCParticle*> daughters = mc.getDaughters();
  BOOST_FOREACH(MCParticle* daughter, daughters) {
    drawTree(*daughter);
  }
  m_seen[mc.getIndex()] = true;
}
